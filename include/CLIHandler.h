#pragma once
#include "Conversation.h"
#include <string>

bool handleCommand(const std::string& input, Conversation& convo, bool& shouldExit);