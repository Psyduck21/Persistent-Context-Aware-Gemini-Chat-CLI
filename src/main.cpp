#include <iostream>
#include "Conversation.h"

int main() {
    Conversation convo;

    convo.addMessage(Role::User, "Hello");
    convo.addMessage(Role::Assistant, "Hi! How can I help?");
    convo.addMessage(Role::User, "Explain Dijkstra");

    if (convo.saveToFile("../data/chat_history.json")) {
        std::cout << "Conversation saved.\n";
    }

    Conversation loaded;
    if (loaded.loadFromFile("../data/chat_history.json")) {
        std::cout << "Conversation loaded.\n";
    }

    for (const auto& msg : loaded.getMessages()) {
        std::cout << "[" << msg.timestamp << "] "
                  << msg.role << ": "
                  << msg.content << "\n";
    }

    return 0;
}
