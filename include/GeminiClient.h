#pragma once

#include <string>
#include <nlohmann/json.hpp>
#include "Conversation.h"

class GeminiClient {
public:
    GeminiClient();
    std::string sendMessage(const nlohmann::json& conversation);
    nlohmann::json toGeminiFormat(const Conversation& convo) const;
    std::string extractGeminiReply(const std::string& responseStr);
private:
    std::string apiKey;
};