#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <string>
#include <vector>

#include "ConfigManager.hpp"
#include "Scheduler.hpp"
#include "Decoder.hpp"
#include "Renderer.hpp"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
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
    Renderer renderer;

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

    if (!renderer.Initialize(window)) {
        std::cerr << "[Renderer Error] Failed to initialize Renderer." << std::endl;
        return 1;
    }

    // Initial Display & Tuning
    double initialSeekTime = scheduler.CalculateSeekTimestamp(simulatedVideoDuration);
    DisplayActiveChannel(window, activeChannel, initialSeekTime);
    TuneChannel(decoder, scheduler, activeChannel, simulatedVideoDuration);

    // 3. Application Polling Loop
    bool isRunning = true;
    SDL_Event event;

    // Allocate AVFrames and SwsContext for color conversion (YUV -> RGBA)
    AVFrame* avFrame = av_frame_alloc();
    AVFrame* rgbFrame = av_frame_alloc();
    struct SwsContext* swsContext = nullptr;
    int cachedWidth = 0;
    int cachedHeight = 0;
    int cachedFormat = -1;

    // Timing and frame pacing variables (targeting ~30 FPS playback)
    const Uint64 targetFrameDuration = 33; 
    Uint64 lastFrameTime = SDL_GetTicks();

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

        // Fetch decoded frames, convert format/color via sws_scale, and push to renderer
        if (decoder.IsOpen()) {
            decoder.FetchFrame(avFrame);
            if (avFrame->width > 0 && avFrame->height > 0 && avFrame->data[0]) {
                if (!swsContext || cachedWidth != avFrame->width || cachedHeight != avFrame->height || cachedFormat != avFrame->format) {
                    if (swsContext) sws_free_context(&swsContext);
                    cachedWidth = avFrame->width;
                    cachedHeight = avFrame->height;
                    cachedFormat = avFrame->format;

                    swsContext = sws_getContext(
                        cachedWidth, cachedHeight, (AVPixelFormat)cachedFormat,
                        cachedWidth, cachedHeight, AV_PIX_FMT_RGBA,
                        SWS_BILINEAR, nullptr, nullptr, nullptr
                    );

                    av_frame_unref(rgbFrame);
                    rgbFrame->format = AV_PIX_FMT_RGBA;
                    rgbFrame->width = cachedWidth;
                    rgbFrame->height = cachedHeight;
                    av_frame_get_buffer(rgbFrame, 32);
                }

                if (swsContext) {
                    sws_scale(
                        swsContext, 
                        avFrame->data, avFrame->linesize, 0, cachedHeight,
                        rgbFrame->data, rgbFrame->linesize
                    );

                    renderer.PrepareTexture(rgbFrame->width, rgbFrame->height);
                    renderer.PushFrame(rgbFrame->data[0], rgbFrame->linesize[0]);
                }
            }
        }

        renderer.RenderPresent();

        // Frame rate / timing control to prevent unthrottled frame blasting
        Uint64 currentTime = SDL_GetTicks();
        Uint64 elapsed = currentTime - lastFrameTime;
        if (elapsed < targetFrameDuration) {
            SDL_Delay(targetFrameDuration - elapsed);
        }
        lastFrameTime = SDL_GetTicks();
    }

    // Cleanup memory
    if (swsContext) sws_free_context(&swsContext);
    av_frame_free(&avFrame);
    av_frame_free(&rgbFrame);

    SDL_GL_DestroyContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}