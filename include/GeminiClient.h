#pragma once

#include <string>
#include <nlohmann/json.hpp>
#include "Conversation.h"

class GeminiClient {
public:
    GeminiClient();
    std::string sendMessage(const nlohmann::json& conversation) const;
    std::string extractGeminiReply(const std::string& responseStr) const;
    bool isConfigured() const;
private:
    std::string apiKey;
};