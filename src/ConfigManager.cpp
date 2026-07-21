#include "ConfigManager.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;
using json = nlohmann::json;

void ConfigManager::ScanDirectoryForVideos(Channel& channel) {
    channel.videoFiles.clear();
    
    // Error Handling: Verify path existence before attempting directory iteration
    if (!fs::exists(channel.directoryPath) || !fs::is_directory(channel.directoryPath)) {
        std::cerr << "[ConfigManager Warning] Directory missing or invalid: " 
                  << channel.directoryPath << std::endl;
        channel.isOperational = false;
        return;
    }

    try {
        for (const auto& entry : fs::directory_iterator(channel.directoryPath)) {
            if (entry.is_regular_file()) {
                std::string ext = entry.path().extension().string();
                // Filter supported media formats
                if (ext == ".mp4" || ext == ".mkv" || ext == ".avi" || ext == ".mov") {
                    channel.videoFiles.push_back(entry.path().string());
                }
            }
        }
        channel.isOperational = !channel.videoFiles.empty();
    } catch (const std::exception& e) {
        std::cerr << "[ConfigManager Error] Disk scan exception: " << e.what() << std::endl;
        channel.isOperational = false;
    }
}

bool ConfigManager::LoadConfiguration(const std::string& configFilePath) {
    std::ifstream file(configFilePath);
    if (!file.is_open()) {
        std::cerr << "[ConfigManager Critical] Could not open config file: " << configFilePath << std::endl;
        return false;
    }

    try {
        json configJson = json::parse(file);

        // Load Channel 0 (Fallback)
        if (configJson.contains("fallback_channel")) {
            fallbackChannel.id = configJson["fallback_channel"]["id"];
            fallbackChannel.name = configJson["fallback_channel"]["name"];
            fallbackChannel.directoryPath = configJson["fallback_channel"]["directory"];
            ScanDirectoryForVideos(fallbackChannel);
            
            std::cout << "[ConfigManager] Fallback Channel Loaded: " << fallbackChannel.videoFiles.size() << " files found." << std::endl;
        }

        // Load User Channels
        userChannels.clear();
        if (configJson.contains("channels")) {
            for (const auto& item : configJson["channels"]) {
                Channel ch;
                ch.id = item["id"];
                ch.name = item["name"];
                ch.directoryPath = item["directory"];
                ScanDirectoryForVideos(ch);

                std::cout << "[ConfigManager] Loaded " << ch.name << " (" << ch.videoFiles.size() << " videos)" << std::endl;
                userChannels.push_back(ch);
            }
        }
        return true;
    } catch (const std::exception& e) {
        std::cerr << "[ConfigManager Exception] JSON parse error: " << e.what() << std::endl;
        return false;
    }
}

const Channel& ConfigManager::GetChannel(int channelId) const {
    for (const auto& ch : userChannels) {
        if (ch.id == channelId && ch.isOperational) {
            return ch;
        }
    }
    // Edge Case Handling: Default to Channel 0 if channel fails or is missing
    std::cout << "[ConfigManager] Channel " << channelId << " unavailable. Falling back to Channel 0." << std::endl;
    return fallbackChannel;
}