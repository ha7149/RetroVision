#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <string>

// Include our new configuration manager header
#include "ConfigManager.hpp"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

// Helper function to update window title and print tuning diagnostic logs
void DisplayActiveChannel(SDL_Window* window, const Channel& activeChannel) {
    std::string windowTitle = "RetroVision | Active: " + activeChannel.name + 
                               " [" + std::to_string(activeChannel.videoFiles.size()) + " Videos Loaded]";
    SDL_SetWindowTitle(window, windowTitle.c_str());

    std::cout << "\n========================================" << std::endl;
    std::cout << "[TUNING ENGINE] Context Switched!" << std::endl;
    std::cout << "  * Channel ID   : " << activeChannel.id << std::endl;
    std::cout << "  * Channel Name : " << activeChannel.name << std::endl;
    std::cout << "  * File Vector  : " << activeChannel.videoFiles.size() << " media files mapped." << std::endl;
    std::cout << "  * Operational  : " << (activeChannel.isOperational ? "YES" : "NO (FALLBACK ACTIVE)") << std::endl;
    
    if (!activeChannel.videoFiles.empty()) {
        std::cout << "  * Primary Media: " << activeChannel.videoFiles[0] << std::endl;
    }
    std::cout << "========================================\n" << std::endl;
}

int main(int argc, char* argv[]) {
    // 1. Diagnostics & System Checks
    std::cout << "========================================" << std::endl;
    std::cout << "RetroVision Engine Diagnostics Initialization" << std::endl;
    std::cout << "FFmpeg AVCodec Version: " << avcodec_version() << std::endl;
    std::cout << "FFmpeg AVFormat Version: " << avformat_version() << std::endl;
    
    nlohmann::json testJson = {{"engine_status", "operational"}};
    std::cout << "JSON Engine State: " << testJson["engine_status"] << std::endl;
    std::cout << "========================================" << std::endl;

    // 2. Configuration & Ingestion Ingestion (FR-004, FR-005)
    // Functional Statement: Instantiate ConfigManager and load runtime configuration schema
    ConfigManager configManager;
    if (!configManager.LoadConfiguration("config/channels.json")) {
        std::cerr << "[Warning] Default config failed to load. Operating in standalone fallback mode." << std::endl;
    }

    // Initialize default active channel state
    int currentChannelIndex = 1;
    Channel activeChannel = configManager.GetChannel(currentChannelIndex);

    // 3. Initialize SDL3 Video Subsystem
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "Engine Failure: SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    // 4. Configure Hardware Core OpenGL Context (3.3 Core)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    // 5. Spawn the Window Client
    SDL_Window* window = SDL_CreateWindow(
        "RetroVision Window Client - Standup Verification", 
        1280, 720, 
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
    );

    if (!window) {
        std::cerr << "Engine Failure: Window creation failed! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    // 6. Instantiate Context and Bind to Window Client
    SDL_GLContext glContext = SDL_GL_CreateContext(window);
    if (!glContext) {
        std::cerr << "Engine Failure: OpenGL context failed! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_GL_SetSwapInterval(1); // Force VSync alignment

    // Display initial startup channel on window title bar and terminal
    DisplayActiveChannel(window, activeChannel);

    // 7. Application Polling Loop
    bool isRunning = true;
    SDL_Event event;

    while (isRunning) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                isRunning = false;
            }
            if (event.type == SDL_EVENT_KEY_DOWN) {
                SDL_Keycode key = event.key.key;

                // Exit condition
                if (key == SDLK_ESCAPE) {
                    isRunning = false;
                }

                // --- Sequential Surfing Logic (FR-006) ---
                if (key == SDLK_UP) {
                    currentChannelIndex++;
                    activeChannel = configManager.GetChannel(currentChannelIndex);
                    DisplayActiveChannel(window, activeChannel);
                } 
                else if (key == SDLK_DOWN) {
                    currentChannelIndex--;
                    if (currentChannelIndex < 0) currentChannelIndex = 0;
                    activeChannel = configManager.GetChannel(currentChannelIndex);
                    DisplayActiveChannel(window, activeChannel);
                }

                // --- Direct Numeric Tuning Logic (FR-007) ---
                else if (key >= SDLK_0 && key <= SDLK_9) {
                    currentChannelIndex = key - SDLK_0; // Convert keycode to integer channel ID
                    activeChannel = configManager.GetChannel(currentChannelIndex);
                    DisplayActiveChannel(window, activeChannel);
                }
            }
        }

        // Color Pass: Broadcast static simulator baseline (Dark Grey)
        glClearColor(0.15f, 0.15f, 0.16f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        SDL_GL_SwapWindow(window);
    }

    // Clean structural cleanup
    SDL_GL_DestroyContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}