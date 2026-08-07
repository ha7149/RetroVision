#include "Renderer.hpp"
#include "Filter.hpp"
#include <iostream>


Renderer::Renderer() = default;


Renderer::~Renderer()
{
    if (videoTexture)
        SDL_DestroyTexture(videoTexture);

    if (sdlRenderer)
        SDL_DestroyRenderer(sdlRenderer);
}


void Renderer::SetFilter(VideoFilter filter)
{
    currentFilter = filter;
}


VideoFilter Renderer::GetFilter() const
{
    return currentFilter;
}


bool Renderer::Initialize(SDL_Window* window)
{
    sdlRenderer = SDL_CreateRenderer(window, nullptr);

    if (!sdlRenderer)
    {
        std::cerr
            << "[Renderer Error] Failed to create SDL Renderer: "
            << SDL_GetError()
            << std::endl;

        return false;
    }

    return true;
}


void Renderer::PrepareTexture(int width, int height)
{
    if (texWidth == width &&
        texHeight == height &&
        videoTexture)
    {
        return;
    }


    if (videoTexture)
        SDL_DestroyTexture(videoTexture);


    videoTexture = SDL_CreateTexture(
        sdlRenderer,
        SDL_PIXELFORMAT_RGBA32,
        SDL_TEXTUREACCESS_STREAMING,
        width,
        height
    );


    texWidth = width;
    texHeight = height;
}


void Renderer::PushFrame(
    const unsigned char* pixelData,
    int pitch
)
{
    if (!videoTexture || !pixelData)
        return;


    SDL_UpdateTexture(
        videoTexture,
        nullptr,
        pixelData,
        pitch
    );
}

void Renderer::RenderPresent()
{
    SDL_RenderClear(sdlRenderer);

    static VideoFilter lastFilter = VideoFilter::None;

    if (currentFilter != lastFilter)
    {
        std::cout
            << "[Renderer] Filter: "
            << (currentFilter == VideoFilter::CRT ? "CRT" : "NONE")
            << std::endl;

        lastFilter = currentFilter;
    }

    if (videoTexture &&
        texWidth > 0 &&
        texHeight > 0)
    {
        int winWidth = 0;
        int winHeight = 0;

        SDL_GetRenderOutputSize(
            sdlRenderer,
            &winWidth,
            &winHeight
        );

        float windowAspect =
            (float)winWidth / (float)winHeight;

        float textureAspect =
            (float)texWidth / (float)texHeight;

        SDL_FRect dstRect;

        if (windowAspect > textureAspect)
        {
            dstRect.h = (float)winHeight;
            dstRect.w = dstRect.h * textureAspect;
            dstRect.x = (winWidth - dstRect.w) * 0.5f;
            dstRect.y = 0;
        }
        else
        {
            dstRect.w = (float)winWidth;
            dstRect.h = dstRect.w / textureAspect;
            dstRect.x = 0;
            dstRect.y = (winHeight - dstRect.h) * 0.5f;
        }

        Filter::Apply(
            sdlRenderer,
            videoTexture,
            dstRect,
            currentFilter
        );
    }

    SDL_RenderPresent(sdlRenderer);
}

/*
void Renderer::RenderPresent()
{
    SDL_RenderClear(sdlRenderer);


    if (videoTexture &&
        texWidth > 0 &&
        texHeight > 0)
    {

        int winWidth = 0;
        int winHeight = 0;


        SDL_GetRenderOutputSize(
            sdlRenderer,
            &winWidth,
            &winHeight
        );


        float windowAspect =
            (float)winWidth /
            (float)winHeight;


        float textureAspect =
            (float)texWidth /
            (float)texHeight;


        SDL_FRect dstRect;


        if (windowAspect > textureAspect)
        {
            // pillarbox
            dstRect.h = (float)winHeight;
            dstRect.w = dstRect.h * textureAspect;
            dstRect.x =
                (winWidth - dstRect.w) * 0.5f;
            dstRect.y = 0;
        }
        else
        {
            // letterbox
            dstRect.w = (float)winWidth;
            dstRect.h = dstRect.w / textureAspect;
            dstRect.x = 0;
            dstRect.y =
                (winHeight - dstRect.h) * 0.5f;
        }


        Filter::Apply(
            sdlRenderer,
            videoTexture,
            dstRect,
            currentFilter
        );
    }


    SDL_RenderPresent(sdlRenderer);
}
*/    