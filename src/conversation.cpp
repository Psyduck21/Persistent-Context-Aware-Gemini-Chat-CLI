#include<bits/stdc++.h>
#include "Conversation.h"

// Formating the timestamp as "YYYY-MM-DD HH:MM:SS"
std::string Conversation::currentTimestamp() const
{
    auto now = std::chrono::system_clock::now();
    std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm* now_tm = std::localtime(&now_time_t);
    char buffer[20];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", now_tm);
    return std::string(buffer);
}

// Convert Role enum to string for storage in Message struct
std::string Conversation::roleToString(Role role) const {
    return (role == Role::User) ? "User" : "Assistant";
}

// Add a new message to the Conversation with the current timestamp
void Conversation::addMessage(Role role, const std::string& content) {
    Message msg;
    msg.role = roleToString(role);
    msg.content = content;
    msg.timestamp = currentTimestamp();
    // Time complexity: O(1) - Adding a message to the end of the vector is constant time.            
    messages.push_back(msg);
}

// Return a const reference to the messages vector for read-only access
const std::vector<Message>& Conversation::getMessages() const {
    return messages;
}

// Clear all messages from the Conversation
void Conversation::clearMessages() {
    messages.clear();
}

// Check if the Conversation is empty
bool Conversation::empty() const {
    return messages.empty();
}

// Return the number of messages in the Conversation
size_t Conversation::size() const {
    return messages.size();
}

