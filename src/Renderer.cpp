#include "Renderer.hpp"
#include <iostream>

Renderer::Renderer() = default;

Renderer::~Renderer() {
    if (videoTexture) SDL_DestroyTexture(videoTexture);
    if (sdlRenderer) SDL_DestroyRenderer(sdlRenderer);
}

bool Renderer::Initialize(SDL_Window* window) {
    sdlRenderer = SDL_CreateRenderer(window, nullptr);
    if (!sdlRenderer) {
        std::cerr << "[Renderer Error] Failed to create SDL Renderer: " << SDL_GetError() << std::endl;
        return false;
    }
    return true;
}

void Renderer::PrepareTexture(int width, int height) {
    if (texWidth == width && texHeight == height && videoTexture) return;

    if (videoTexture) SDL_DestroyTexture(videoTexture);

    // Assuming standard 8-bit RGBA pixel format from the Decoder
    videoTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, width, height);
    texWidth = width;
    texHeight = height;
}

void Renderer::PushFrame(const unsigned char* pixelData, int pitch) {
    if (!videoTexture || !pixelData) return;
    
    // Upload the raw pixel buffer to the GPU texture
    SDL_UpdateTexture(videoTexture, nullptr, pixelData, pitch);
}

void Renderer::RenderPresent() {
    SDL_RenderClear(sdlRenderer);
    
    if (videoTexture && texWidth > 0 && texHeight > 0) {
        int winWidth = 0, winHeight = 0;
        if (SDL_GetRenderOutputSize(sdlRenderer, &winWidth, &winHeight) && winWidth > 0 && winHeight > 0) {
            float windowAspect = static_cast<float>(winWidth) / static_cast<float>(winHeight);
            float textureAspect = static_cast<float>(texWidth) / static_cast<float>(texHeight);

            SDL_FRect dstRect;
            if (windowAspect > textureAspect) {
                // Window is wider than video: pillarbox (fit height, center width)
                dstRect.h = static_cast<float>(winHeight);
                dstRect.w = dstRect.h * textureAspect;
                dstRect.x = (winWidth - dstRect.w) * 0.5f;
                dstRect.y = 0.0f;
            } else {
                // Window is taller than video: letterbox (fit width, center height)
                dstRect.w = static_cast<float>(winWidth);
                dstRect.h = dstRect.w / textureAspect;
                dstRect.x = 0.0f;
                dstRect.y = (winHeight - dstRect.h) * 0.5f;
            }

            SDL_RenderTexture(sdlRenderer, videoTexture, nullptr, &dstRect);
        } else {
            SDL_RenderTexture(sdlRenderer, videoTexture, nullptr, nullptr);
        }
    }
    
    SDL_RenderPresent(sdlRenderer);
}