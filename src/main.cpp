#include <iostream>
#include "Conversation.h"

int main() {
    Conversation convo;

    convo.addMessage(Role::User , "Hello");
    convo.addMessage(Role::Assistant, "Hi! How can I help?");

    std::cout<<"Conversation Size : "<<convo.size()<<"\n\n";

    for (const auto& msg : convo.getMessages()) {
        std::cout << "[" << msg.timestamp << "] "
                  << msg.role << ": "
                  << msg.content << "\n";
    }

    return 0;
}
