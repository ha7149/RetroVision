#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <SDL3/SDL.h>

class Renderer {
private:
    SDL_Renderer* sdlRenderer = nullptr;
    SDL_Texture* videoTexture = nullptr;
    int texWidth = 0;
    int texHeight = 0;

public:
    Renderer();
    ~Renderer();

    // Initialize the SDL Renderer attached to your window
    bool Initialize(SDL_Window* window);

    // Create or resize the texture based on the Decoder's output
    void PrepareTexture(int width, int height);

    // Push the raw FFmpeg frame buffer to the GPU (Phase 5)
    void PushFrame(const unsigned char* pixelData, int pitch);

    // Clear and present the screen
    void RenderPresent();
};

#endif