/*
Conversation.h - Header file for the Conversation class
This class manages a list of messages in a conversation, including their roles, content, and timestamps.
Functions include adding messages, retrieving messages, clearing the conversation, and checking if it's empty.
*/
#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

// Using Enum class for better type safety and readability
enum class Role
{
    user,
    model
};

struct Message
{
    std::string role;
    std::string content;
    std::string timestamp;
};

// Conversation class to manage the list of messages and related operations
class Conversation
{
private:
    // Vector to store the messages in the conversation in the order they were added in memory.
    std::vector<Message> messages;
    std::string currentTimestamp() const;
    static std::string roleToString(Role role);

public:
    void addMessage(Role role, const std::string &content);
    const std::vector<Message> &getMessages() const;
    void clearMessages();
    bool empty() const;
    size_t size() const;

    // Phase 2: Serialization and Deserialization functions
    nlohmann::json toJson() const;
    void fromJson(const nlohmann::json &jsondata);
    bool saveToFile(const std::string &FILENAME) const;
    bool loadFromFile(const std::string &FILENAME);
    void printHistory() const;
    void exportToMarkdown(const std::string &FILENAME) const;
};