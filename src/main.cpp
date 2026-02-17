#include <iostream>
#include <filesystem>
#include <cstdlib>
#include <memory>
#include <thread>
#include <chrono>
#include <nlohmann/json.hpp>

#include "Conversation.h"
#include "GeminiClient.h"
#include "CLIHandler.h"
#include "EnvHandler.h"

// Conversation* g_convo = nullptr;
// std::string g_chatFile;

// void handleExitSignal(int)
// {
//     if (g_convo)
//     {
//         g_convo->saveToFile(g_chatFile);
//     }

//     std::_Exit(0);
// }


int main()
{
    loadEnvFile(".env");

    Conversation convo;
    std::unique_ptr<GeminiClient> client;
    bool shouldExit = false;
    const std::filesystem::path dataDir = "./data";
    const std::filesystem::path chatFile = dataDir / "chat_history.json";

    std::error_code ec;

    if (!std::filesystem::exists(dataDir))
    {
        if (!std::filesystem::create_directories(dataDir, ec))
        {
            std::cerr << "ERROR: Cannot create data directory: "
                      << ec.message() << "\n";
            return EXIT_FAILURE;
        }
    }

    if (std::filesystem::exists(chatFile))
    {
        if (!convo.loadFromFile(chatFile.string()))
        {
            std::cerr << "WARNING: Failed to load chat history.\n";
        }
    }

    // globalConvo = &convo;
    // globalChatFile = chatFile;

    // std::signal(SIGINT, handleExitSignal);
    // std::signal(SIGTERM, handleExitSignal);

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
        // handling normal user input, add to conversation, send to Gemini, get response, add to conversation, save to file
        if (input.empty())
        {
            std::cerr << "Please enter a message or command.\n";
            continue;
        }
        // string validation for input can be added here if needed (e.g., check for max length, prohibited content, etc.)
        input.erase(0, input.find_first_not_of(" \t\n\r"));
        input.erase(input.find_last_not_of(" \t\n\r") + 1);
        convo.addMessage(Role::user, input);

        // Ensure Gemini client available
        if (!client)
        {
            try
            {
                client = std::make_unique<GeminiClient>();
                if (!client->isConfigured())
                {
                    std::cerr << "Warning: GEMINI_API_KEY not set. Gemini requests will be disabled.\n";
                    client.reset();
                }
            }
            catch (const std::exception &e)
            {
                std::cerr << "Warning: failed to initialize Gemini client: " << e.what() << "\n";
                client.reset();
            }
        }

        if (!client)
        {
            std::cerr << "Gemini client unavailable; skipping request.\n";
            continue;
        }

        try
        {
            nlohmann::json geminiInput =  convo.toGeminiFormat();
            // std::cout<< "Gemini input JSON: " << geminiInput.dump(2) << "\n"; // Debugging output
            std::string response = client->sendMessage(geminiInput);
            // std::cout<< "Raw Gemini response: " << response << "\n"; // Debugging output
            std::string reply = client->extractGeminiReply(response);
            std::cout << "Gemini: " << reply << "\n";
            convo.addMessage(Role::model, reply);

            bool saveOk = convo.saveToFile(chatFile.string());

            if (!saveOk)
            {
                std::cerr << "\nERROR: Failed to save chat history.\n"
                          << "Your recent messages may not be permanently saved.\n"
                          << "Possible causes:\n"
                          << "  - Disk is full\n"
                          << "  - Permission denied\n"
                          << "  - File system error\n"
                          << "\nAttempting recovery...\n";

                // Retry once after short delay
                std::this_thread::sleep_for(std::chrono::milliseconds(200));

                saveOk = convo.saveToFile(chatFile.string());

                if (!saveOk)
                {
                    std::cerr << "\nCRITICAL: Unable to save chat history after retry.\n"
                              << "Please free disk space or fix permissions.\n"
                              << "You may lose recent conversation state if program exits.\n";

                    // Optional: emergency backup attempt
                    std::string emergencyFile = "./data/chat_history_backup.json";

                    if (convo.saveToFile(emergencyFile))
                    {
                        std::cerr << "Emergency backup saved to: "
                                  << emergencyFile << "\n";
                    }
                    else
                    {
                        std::cerr << "Emergency backup also failed.\n";
                    }
                }
            }

        }
        catch (const std::exception &e)
        {
            std::cerr << "Error: " << e.what() << "\n";
        }
    }
}