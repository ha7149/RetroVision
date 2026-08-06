#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <string>

#include "ConfigManager.hpp"
#include "Scheduler.hpp"
#include "Decoder.hpp"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

void DisplayActiveChannel(SDL_Window* window, const Channel& activeChannel, double seekTimestamp) {
    std::string windowTitle = "RetroVision | Active: " + activeChannel.name;
    SDL_SetWindowTitle(window, windowTitle.c_str());

    std::cout << "\n========================================" << std::endl;
    std::cout << "[TUNING ENGINE] Context Switched!" << std::endl;
    std::cout << "  * Channel ID   : " << activeChannel.id << std::endl;
    std::cout << "  * Channel Name : " << activeChannel.name << std::endl;
    std::cout << "  * Operational  : " << (activeChannel.isOperational ? "YES" : "NO (FALLBACK ACTIVE)") << std::endl;
    
    if (!activeChannel.videoFiles.empty()) {
        std::cout << "  * Primary Media: " << activeChannel.videoFiles[0] << std::endl;
    }
    
    std::cout << "  * Sync Seek T-Stamp: " << seekTimestamp << "s (Joining Broadcast in progress...)" << std::endl;
    std::cout << "========================================\n" << std::endl;
}

void TuneChannel(Decoder& decoder, Scheduler& scheduler, const Channel& channel, double duration) {
    if (channel.videoFiles.empty()) return;

    std::string mediaPath = channel.videoFiles[0];
    double seekTime = scheduler.CalculateSeekTimestamp(duration);

    if (decoder.OpenMedia(mediaPath)) {
        if (decoder.SeekToTime(seekTime)) {
            std::cout << "[Decoder] Seek successful to " << seekTime << "s on keyframe." << std::endl;
        } else {
            std::cerr << "[Decoder Warning] Failed to seek to timestamp: " << seekTime << "s" << std::endl;
        }
    }
}

int main(int argc, char* argv[]) {
    std::cout << "RetroVision Engine Boot Sequence Initiated..." << std::endl;

    // 1. Instantiate Managers & Media Engine
    ConfigManager configManager;
    if (!configManager.LoadConfiguration("config/channels.json")) {
        std::cerr << "[Warning] Default config failed. Operating in fallback mode." << std::endl;
    }

    Scheduler scheduler;
    Decoder decoder;

    int currentChannelIndex = 1;
    Channel activeChannel = configManager.GetChannel(currentChannelIndex);
    
    // Simulate a 2-minute video file for math testing
    double simulatedVideoDuration = 120.0; 

    // 2. Initialize SDL3
    if (SDL_Init(SDL_INIT_VIDEO) < 0) return 1;
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_Window* window = SDL_CreateWindow(
        "RetroVision Window Client", 1280, 720, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
    );
    if (!window) return 1;

    SDL_GLContext glContext = SDL_GL_CreateContext(window);
    SDL_GL_SetSwapInterval(1);

    // Initial Display & Tuning
    double initialSeekTime = scheduler.CalculateSeekTimestamp(simulatedVideoDuration);
    DisplayActiveChannel(window, activeChannel, initialSeekTime);
    TuneChannel(decoder, scheduler, activeChannel, simulatedVideoDuration);

    // 3. Application Polling Loop
    bool isRunning = true;
    SDL_Event event;

    // Added: Allocate AVFrame container for decoding loop
    AVFrame* avFrame = av_frame_alloc();

    while (isRunning) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                isRunning = false;
            }
            if (event.type == SDL_EVENT_KEY_DOWN) {
                SDL_Keycode key = event.key.key;
                if (key == SDLK_ESCAPE) isRunning = false;

                bool channelChanged = false;

                if (key == SDLK_UP) {
                    currentChannelIndex++;
                    channelChanged = true;
                } else if (key == SDLK_DOWN) {
                    currentChannelIndex--;
                    if (currentChannelIndex < 0) currentChannelIndex = 0;
                    channelChanged = true;
                } else if (key >= SDLK_0 && key <= SDLK_9) {
                    currentChannelIndex = key - SDLK_0;
                    channelChanged = true;
                }

                if (channelChanged) {
                    activeChannel = configManager.GetChannel(currentChannelIndex);
                    double currentSeekTime = scheduler.CalculateSeekTimestamp(simulatedVideoDuration);
                    DisplayActiveChannel(window, activeChannel, currentSeekTime);
                    TuneChannel(decoder, scheduler, activeChannel, simulatedVideoDuration);
                }
            }
        }

        // Added: Fetch decoded frames continuously if the decoder is active
        if (decoder.IsOpen()) {
            decoder.FetchFrame(avFrame);
        }

        glClearColor(0.15f, 0.15f, 0.16f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        SDL_GL_SwapWindow(window);
    }

    // Added: Free frame container memory
    av_frame_free(&avFrame);

    SDL_GL_DestroyContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}