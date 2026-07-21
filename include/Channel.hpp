#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <vector>

struct Channel {
    int id = 0;
    std::string name = "UNKNOWN";
    std::string directoryPath = "";
    std::vector<std::string> videoFiles;
    bool isOperational = false;
};

#endif // CHANNEL_HPP