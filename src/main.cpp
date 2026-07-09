#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>
#include <nlohmann/json.hpp>
#include <iostream>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

int main(int argc, char* argv[]) {
    // 1. Verify FFmpeg integration via version printing
    std::cout << "========================================" << std::endl;
    std::cout << "RetroVision Engine Diagnostics Initialization" << std::endl;
    std::cout << "FFmpeg AVCodec Version: " << avcodec_version() << std::endl;
    std::cout << "FFmpeg AVFormat Version: " << avformat_version() << std::endl;
    
    // Test JSON inclusion cleanly
    nlohmann::json testJson = {{"engine_status", "operational"}};
    std::cout << "JSON Engine State: " << testJson["engine_status"] << std::endl;
    std::cout << "========================================" << std::endl;

    // 2. Initialize SDL3 Video Subsystem
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "Engine Failure: SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    // 3. Configure Hardware Core OpenGL Context (3.3 Core)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    // 4. Spawn the Window Client
    SDL_Window* window = SDL_CreateWindow(
        "RetroVision Window Client - Production Validation", 
        1280, 720, 
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
    );

    if (!window) {
        std::cerr << "Engine Failure: Window creation failed! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    // 5. Instatiate Context and Bind to Window Client
    SDL_GLContext glContext = SDL_GL_CreateContext(window);
    if (!glContext) {
        std::cerr << "Engine Failure: OpenGL context failed! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_GL_SetSwapInterval(1); // Force VSync alignment

    // 6. Application Polling Loop
    bool isRunning = true;
    SDL_Event event;

    while (isRunning) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                isRunning = false;
            }
            if (event.type == SDL_EVENT_KEY_DOWN) {
                if (event.key.key == SDLK_ESCAPE) {
                    isRunning = false;
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
