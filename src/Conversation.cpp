#include "Conversation.h"
#include <chrono>
#include <ctime>
#include <string>
#include <vector>
#include <fstream>
#include <cstdio>
#include <cerrno>
#include <cstring>
#include <iostream>

// Formating the timestamp as "YYYY-MM-DD HH:MM:SS"
std::string Conversation::currentTimestamp() const
{
    auto now = std::chrono::system_clock::now();
    std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm now_tm;
#if defined(_POSIX_VERSION) || defined(__linux__)
    localtime_r(&now_time_t, &now_tm);
#else
    const std::tm *tmp = std::localtime(&now_time_t);
    if (tmp)
        now_tm = *tmp;
#endif
    char buffer[20];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &now_tm);
    return std::string(buffer);
}

// Convert Role enum to string for storage in Message struct
std::string Conversation::roleToString(Role role)
{
    return (role == Role::user) ? "user" : "model";
}

// Add a new message to the Conversation with the current timestamp
void Conversation::addMessage(Role role, const std::string &content)
{
    Message msg;
    msg.role = roleToString(role);
    msg.content = content;
    msg.timestamp = currentTimestamp();
    // Time complexity: O(1) - Adding a message to the end of the vector is constant time.
    messages.push_back(msg);
}

// Return a const reference to the messages vector for read-only access
const std::vector<Message> &Conversation::getMessages() const
{
    return messages;
}

// Clear all messages from the Conversation
void Conversation::clearMessages()
{
    messages.clear();
}

// Check if the Conversation is empty
bool Conversation::empty() const
{
    return messages.empty();
}

// Return the number of messages in the Conversation
size_t Conversation::size() const
{
    return messages.size();
}

// PHASE 2 - Persistence and JSON

// Serialize the Conversation to JSON format
nlohmann::json Conversation::toJson() const
{
    nlohmann::json jsondata;
    jsondata["messages"] = nlohmann::json::array();
    for (const auto &msg : messages)
    {
        nlohmann::json msgJson;
        msgJson["role"] = msg.role;
        msgJson["content"] = msg.content;
        msgJson["timestamp"] = msg.timestamp;
        jsondata["messages"].push_back(msgJson);
    }
    return jsondata;
}

// Deserialize the Conversation from JSON format
void Conversation::fromJson(const nlohmann::json &jsondata)
{
    // adding rules for JSON format validation and Gemini protocol compliance
    if (!jsondata.contains("messages") || !jsondata["messages"].is_array())
    {
        throw std::invalid_argument("Invalid JSON format: 'messages' array is missing should be {messages : [] }");
    }

    // laoding Messages from JSON array to memeory
    std::vector<Message> loadedMessages;
    for (const auto &MJSON : jsondata["messages"])
    {
        // Validate each message object
        if (!MJSON.contains("role") || !MJSON.contains("content") || !MJSON.contains("timestamp"))
            throw std::runtime_error("Invalid Messages format in JSon should be {role,content,timestamp}");

        Message msg;
        msg.role = MJSON["role"].get<std::string>();
        msg.content = MJSON["content"].get<std::string>();
        msg.timestamp = MJSON["timestamp"].get<std::string>();

        loadedMessages.push_back(msg);
    }
    // laoding into memory after validation
    messages = std::move(loadedMessages);
}

// saving to JSON file data/chat_history.json
bool Conversation::saveToFile(const std::string &FILENAME) const
{
    const std::string tempfile = FILENAME + ".tmp";

    try
    {
        std::ofstream out(tempfile);
        if (!out)
            return false;

        // writing the JSON data to file with pretty printing (indentation of 2 spaces)
        out << toJson().dump(2);
        out.close();

        // attempt to remove old file (ignore if it doesn't exist)
        if (std::remove(FILENAME.c_str()) != 0 && errno != ENOENT)
        {
            std::cerr << "Warning: failed to remove old file " << FILENAME << ": " << std::strerror(errno) << "\n";
        }

        // rename tempfile to final filename
        if (std::rename(tempfile.c_str(), FILENAME.c_str()) != 0)
        {
            std::cerr << "Error: failed to rename " << tempfile << " to " << FILENAME << ": " << std::strerror(errno) << "\n";
            // try to remove tempfile
            std::remove(tempfile.c_str());
            return false;
        }

        return true;
    }
    catch (...)
    {
        return false;
    }
}

// laoding th file from disk
bool Conversation::loadFromFile(const std::string &FILENAME)
{
    try
    {
        std::ifstream in(FILENAME);
        if (!in)
        {

            return false;
        }
        // parsing the JSON data from file
        nlohmann::json jsondata;
        in >> jsondata;
        fromJson(jsondata);
        std::cout << "Loading conversation from file: " << FILENAME << "\n";
        return true;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error loading conversation from file: " << FILENAME << ": " << e.what() << "\n";
        return false;
    }
}

// phase 4 - Command handling and conversation history printing
void Conversation::printHistory() const
{
    if (messages.empty())
    {
        std::cout << "No conversation history available.\n";
        return;
    }
    std::cout << "Conversation History:\n";
    for (const auto &msg : messages)
    {
        std::cout << "[" << msg.timestamp << "] " << msg.role << ": " << msg.content << "\n";
    }
}

void Conversation::exportToMarkdown(const std::string &FILENAME) const
{
    if (messages.empty())
    {
        std::cout << "No conversation history to export.\n";
        return;
    }
    try
    {
        std::ofstream out(FILENAME);
        if (!out)
        {
            std::cerr << "Error opening file for writing: " << FILENAME << "\n";
            return;
        }

        out << "# Conversation History\n\n";
        for (const auto &msg : messages)
        {
            std::string roleHeader =
                (msg.role == "user" || msg.role == "User")
                    ? "User"
                    : "Assistant";

            out << "## " << roleHeader << "\n";
            out << msg.content << "\n\n";
        }
        out.close();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error exporting conversation: " << e.what() << "\n";
    }
}
