#ifndef CONFIGMANAGER_HPP
#define CONFIGMANAGER_HPP

#include "Channel.hpp"
#include <vector>
#include <string>

class ConfigManager {
public:
    Channel fallbackChannel;
    std::vector<Channel> userChannels;

    // Functional Statement: Reads JSON config, scans disk, and populates channel structures
    bool LoadConfiguration(const std::string& configFilePath);

    // Functional Statement: Safe channel retrieval with automatic fallback redirection (FR-011)
    const Channel& GetChannel(int channelId) const;

private:
    void ScanDirectoryForVideos(Channel& channel);
};

#endif // CONFIGMANAGER_HPP