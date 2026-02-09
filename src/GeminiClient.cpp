#include <string>
#include <curl/curl.h>
#include "GeminiClient.h"
#include <nlohmann/json.hpp>
#include <iostream>
#include "Conversation.h"

static size_t writeCallback(
    void *contents,
    size_t size,
    size_t nmemb,
    void *userp)
{
    size_t total = size * nmemb;
    std::string *str = static_cast<std::string *>(userp);
    str->append(static_cast<char *>(contents), total);
    return total;
}
// api key
GeminiClient::GeminiClient()
{
    const char *env_var_name = "GEMINI_API_KEY";
    const char *env_api_key = std::getenv(env_var_name);
    if (!env_api_key)
    {
        throw std::runtime_error(
            "GEMINI_API_KEY not found. "
            "Load it using `source .env` or export it in the shell.");
    }
    apiKey = env_api_key;
    // std::cout<<apiKey<<std::endl;
}

std::string GeminiClient::sendMessage(const nlohmann::json &conversation)
{
    // initialize curl
    CURL *curl = curl_easy_init();
    if (!curl)
    {
        throw std::runtime_error("Failed to initialize CURL");
    }
    // preare response string
    std::string response;

    // set url
    std::string url = "https://generativelanguage.googleapis.com/v1beta/models/gemini-2.5-flash:generateContent?key=" + apiKey;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    // set headers
    struct curl_slist *header = nullptr;
    header = curl_slist_append(header, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);

    // set post data
    std::string payload = conversation.dump();
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());

    // callback function
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    // set timeout
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);

    // perform request
    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK)
    {
        curl_easy_cleanup(curl);
        throw std::runtime_error("CURL request failed: " + std::string(curl_easy_strerror(res)));
    }
    // cleanup
    curl_slist_free_all(header);
    curl_easy_cleanup(curl);
    return response;
}

// Convert the Conversation to Gemini API format
nlohmann::json GeminiClient::toGeminiFormat(const Conversation& convo) const {
    nlohmann::json j;
    j["contents"] = nlohmann::json::array();

    for (const auto& msg : convo.getMessages()) {
        nlohmann::json content;
        content["role"] = msg.role;

        // Parts MUST be an array
        content["parts"] = nlohmann::json::array({
            { { "text", msg.content } }
        });

        j["contents"].push_back(content);
    }

    return j;
}

// Extract the assistant's reply from the Gemini API response
std::string GeminiClient::extractGeminiReply(const std::string& responseStr) {
    auto json = nlohmann::json::parse(responseStr);

    if (!json.contains("candidates") || json["candidates"].empty()) {
        throw std::runtime_error("No candidates in Gemini response");
    }

    const auto& candidate = json["candidates"][0];

    if (!candidate.contains("content") ||
        !candidate["content"].contains("parts") ||
        candidate["content"]["parts"].empty()) {
        throw std::runtime_error("Invalid Gemini response format");
    }

    return candidate["content"]["parts"][0]["text"].get<std::string>();
}
