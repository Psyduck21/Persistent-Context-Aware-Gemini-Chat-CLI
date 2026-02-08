#pragma once

#include <string>
#include <vector>

// Using Enum class for better type safety and readability
enum class Role {
    User,
    Assistant
};

struct Message
{
    std::string role; 
    std::string content;
    std::string timestamp;
};

class Conversation
{
private:
    std::vector<Message> messages;
    std::string currentTimestamp() const;
    std::string roleToString(Role role) const ;
public:
    void addMessage(Role role, const std::string& content);
    const std::vector<Message>& getMessages() const;
    void clearMessages();
    bool empty() const;
    size_t size() const;
};