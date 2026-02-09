#include <iostream>
#include <sstream>
#include <map>
#include <filesystem>


#include "CLIHandler.h"

static const std::map<std::string, std::string> COMMAND_HELP = {
    {"/help", "Show available commands"},
    {"/new", "Start a new conversation"},
    {"/clear", "Clear current conversation"},
    {"/load", "Load conversation from file: /load <file>"},
    {"/export", "Export conversation to Markdown: /export <file>"},
    {"/history", "Show conversation history"},
    {"/exit", "Exit the application"}};

bool handleCommand(const std::string &input, Conversation &convo, bool &shouldExit)
{
    if (input.empty() || input[0] != '/')
    {
        return false; // Not a command
    }

    std::istringstream iss(input);
    std::string command, arg;
    iss >> command;
    // Read the remainder of the line as the argument to allow paths with spaces
    iss >> std::ws;
    std::getline(iss, arg);

    // Handle commands
    if (command == "/help")
    {
        std::cout << "\nAvailable commands:\n";
        for (const auto &[cmd, desc] : COMMAND_HELP)
        {
            std::cout << "  " << cmd << " - " << desc << "\n";
        }
        return true;
    }

    // Start a new conversation
    if (command == "/new")
    {
        if (!convo.empty())
        {
            std::cout << "Current conversation will be lost. Continue? (y/n): ";
            std::string choice;
            std::getline(std::cin, choice);

            if (choice != "y" && choice != "Y")
            {
                std::cout << "Cancelled. Conversation preserved.\n";
                return true;
            }
        }

        convo.clearMessages();
        std::cout << "Started a new conversation.\n";
        return true;
    }

    // Exiting the application
    if (command == "/exit")
    {
        shouldExit = true;
        std::cout << "Exiting the application. Goodbye!\n";
        return true;
    }

    // Loading a conversation from file

    if (command == "/load")
    {
        if (arg.empty())
        {
            std::cerr << "Error: filename required. Usage: /load <file>\n";
            return true;
        }

        std::filesystem::path filePath(arg);

        if (!std::filesystem::exists(filePath))
        {
            std::cerr << "Error: file does not exist.\n";
            return true;
        }

        if (!std::filesystem::is_regular_file(filePath))
        {
            std::cerr << "Error: not a regular file.\n";
            return true;
        }

        if (!convo.loadFromFile(filePath.string())) {
            std::cerr << "Error loading file: " << filePath << "\n";
        }

        return true;
    }

    // Exporting conversation to Markdown
    if (command == "/export")
    {
        if (arg.empty())
        {
            std::cout << "Error: filename required. Usage: /export <file>\n";
            return true;
        }

        try
        {
            convo.exportToMarkdown(arg);
            std::cout << "Conversation exported to " << arg << "\n";
        }
        catch (const std::exception &e)
        {
            std::cout << "Error exporting conversation: " << e.what() << "\n";
        }
        return true;
    }

    // Print conversation history
    if (command == "/history")
    {
        convo.printHistory();
        return true;
    }

    std::cout << "Unknown command: " << command << "Use /help for commands." << "\n";
    return true; // Command was handled
}