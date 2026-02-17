#include <fstream>
#include <sstream>
#include <cstdlib>

#include "EnvHandler.h"

void loadEnvFile(const std::string& filename)
{
    std::ifstream file(filename);

    if (!file.is_open())
        return;

    std::string line;

    while (std::getline(file, line))
    {
        if (line.empty() || line[0] == '#')
            continue;

        size_t pos = line.find('=');

        if (pos == std::string::npos)
            continue;

        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);

        setenv(key.c_str(), value.c_str(), 1);
    }
}
