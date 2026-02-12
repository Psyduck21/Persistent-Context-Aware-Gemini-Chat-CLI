#include <iostream>
#include <filesystem>
#include <cstdlib>
#include <memory>
#include <nlohmann/json.hpp>

#include "Conversation.h"
#include "GeminiClient.h"
#include "CLIHandler.h"

int main()
{

    Conversation convo;
    std::unique_ptr<GeminiClient> client;
    bool shouldExit = false;
    
    // Load existing conversation from file if it exists
    if (std::filesystem::exists("./data/chat_history.json"))
    {
        if (convo.loadFromFile("./data/chat_history.json"))
            std::cout << "Conversation loaded.\n";
        else
            std::cerr << "Failed to load conversation from ./data/chat_history.json\n";
    }
    
    std::string input;
    std::cout << "Commands: /new, /load <file>, /export <file>, /exit\n";
    
    while (!shouldExit)
    {
        std::cout << "\nYou: ";
        std::getline(std::cin, input);

        if (handleCommand(input, convo, shouldExit))
        {
            continue;
        }
        
        convo.addMessage(Role::user, input);
        
        // Ensure Gemini client available
        if (!client) {
            try {
                client = std::make_unique<GeminiClient>();
                if (!client->isConfigured()) {
                    std::cerr << "Warning: GEMINI_API_KEY not set. Gemini requests will be disabled.\n";
                    client.reset();
                }
            } catch (const std::exception &e) {
                std::cerr << "Warning: failed to initialize Gemini client: " << e.what() << "\n";
                client.reset();
            }
        }
        
        if (!client) {
            std::cerr << "Gemini client unavailable; skipping request.\n";
            continue;
        }

        try
        {
            nlohmann::json geminiInput = client->toGeminiFormat(convo);
            // std::cout<< "Gemini input JSON: " << geminiInput.dump(2) << "\n"; // Debugging output
            std::string response = client->sendMessage(geminiInput);
            // std::cout<< "Raw Gemini response: " << response << "\n"; // Debugging output
            std::string reply = client->extractGeminiReply(response);
            std::cout << "Gemini: " << reply << "\n";
            convo.addMessage(Role::model, reply);
            convo.saveToFile("./data/chat_history.json");
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error: " << e.what() << "\n";
        }
    }
}