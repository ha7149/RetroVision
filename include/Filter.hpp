#pragma once

#include <SDL3/SDL.h>

enum class VideoFilter
{
    None,
    CRT
};

class Filter
{
public:

    static void Apply(
        SDL_Renderer* renderer,
        SDL_Texture* texture,
        const SDL_FRect& dstRect,
        VideoFilter filter
    );
};