#include <iostream>
#include "Conversation.h"
#include "GeminiClient.h"
#include <filesystem>
#include <cstdlib> // for std::getenv
#include <nlohmann/json.hpp>

int main()
{

    Conversation convo;
    GeminiClient client;

    // Load existing conversation from file if it exists
    if (std::filesystem::exists("../data/chat_history.json"))
    {
        convo.loadFromFile("../data/chat_history.json");
        std::cout << "Conversation loaded.\n";
    }

    std::string input;
    std::cout << "Type 'exit' to quit\n";

    while (true)
    {
        std::cout << "\nYou: ";
        std::getline(std::cin, input);

        if (input == "exit")
            break;
        if (input.empty())
            continue;

        convo.addMessage(Role::User, input);

        try
        {
            nlohmann::json geminiInput = client.toGeminiFormat(convo);
            std::string response = client.sendMessage(geminiInput);
            std::string reply = client.extractGeminiReply(response);
            std::cout << "Gemini: " << reply << "\n";
            convo.addMessage(Role::Assistant, reply);
            convo.saveToFile("../data/chat_history.json");
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error: " << e.what() << "\n";
        }
    }
}