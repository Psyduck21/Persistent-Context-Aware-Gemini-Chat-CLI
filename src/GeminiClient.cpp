#include <string>
#include <curl/curl.h>
#include "GeminiClient.h"
#include <nlohmann/json.hpp>
#include <iostream>
#include "Conversation.h"
#include <stdexcept>
#include <cstring>
#include <algorithm>
#include <cctype>

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
    // Do not throw in constructor; mark client as not configured if key absent.
    if (!env_api_key)
    {
        apiKey.clear();
    }
    else
    {
        apiKey = env_api_key;
    }
    // std::cout<<apiKey<<"\n;
}

std::string GeminiClient::sendMessage(const nlohmann::json &conversation) const
{
    // initialize curl
    CURL *curl = curl_easy_init();
    if (!curl)
    {
        throw std::runtime_error("Failed to initialize CURL");
    }
    // RAII wrappers for cleanup
    struct CurlHandle {
        CURL* c;
        explicit CurlHandle(CURL* c):c(c){}
        ~CurlHandle(){ if(c) curl_easy_cleanup(c); }
    } guardCurl(curl);

    std::string response;

    // ensure API key present
    if (apiKey.empty()) {
        throw std::runtime_error("GEMINI_API_KEY is not configured; cannot send requests");
    }

    // set url
    std::string url = "https://generativelanguage.googleapis.com/v1beta/models/gemini-2.5-flash:generateContent?key=" + apiKey;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    // set headers
    struct curl_slist *header = nullptr;
    header = curl_slist_append(header, "Content-Type: application/json");
    // RAII for header
    struct SlistGuard {
        struct curl_slist* l;
        explicit SlistGuard(struct curl_slist* l):l(l){}
        ~SlistGuard(){ if(l) curl_slist_free_all(l); }
    } guardHeader(header);
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
        throw std::runtime_error(std::string("CURL request failed: ") + curl_easy_strerror(res));
    }

    // success: guards will clean up
    return response;
}

// Convert the Conversation to Gemini API format
nlohmann::json GeminiClient::toGeminiFormat() const {
    nlohmann::json j;
    j["contents"] = nlohmann::json::array();

    for (const auto& msg : convo.getMessages()) {
        nlohmann::json content;
        // Normalize role strings to Gemini expected values
        std::string roleLower = msg.role;
        std::transform(roleLower.begin(), roleLower.end(), roleLower.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        if (roleLower == "user") content["role"] = "user";
        else content["role"] = "assistant";

        // Parts MUST be an array
        content["parts"] = nlohmann::json::array({
            { { "text", msg.content } }
        });

        j["contents"].push_back(content);
    }

    return j;
}

// Extract the assistant's reply from the Gemini API response
std::string GeminiClient::extractGeminiReply(const std::string& responseStr) const {
    try {
        auto json = nlohmann::json::parse(responseStr);

        // Check for API error response first
        if (json.contains("error")) {
            const auto& error = json["error"];
            int code = error.value("code", 0);
            std::string message = error.value("message", "Unknown error");
            std::string status = error.value("status", "UNKNOWN");

            // Handle rate limiting (429)
            if (code == 429 || status == "RESOURCE_EXHAUSTED") {
                std::string detail = "API Rate Limit Exceeded (Free Tier: 20 requests/day per model)\n";
                detail += "Message: " + message + "\n";
                
                // Extract retry information if available
                if (error.contains("details")) {
                    for (const auto& detail_item : error["details"]) {
                        if (detail_item.contains("retryDelay")) {
                            detail += "Retry after: " + detail_item["retryDelay"].get<std::string>();
                        }
                    }
                }
                throw std::runtime_error(detail);
            }

            // Generic API error
            std::string errorMsg = "Gemini API Error (Code " + std::to_string(code) + "): " + message;
            throw std::runtime_error(errorMsg);
        }

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
    } catch (const nlohmann::json::exception &e) {
        throw std::runtime_error(std::string("Failed to parse Gemini response JSON: ") + e.what());
    }
}

bool GeminiClient::isConfigured() const {
    return !apiKey.empty();
}
