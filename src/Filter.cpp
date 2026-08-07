#include "Filter.hpp"

void Filter::Apply(
    SDL_Renderer* renderer,
    SDL_Texture* texture,
    const SDL_FRect& dstRect,
    VideoFilter filter
)
{
    if (filter == VideoFilter::None)
    {
        SDL_RenderTexture(
            renderer,
            texture,
            nullptr,
            &dstRect
        );

        return;
    }

    if (filter == VideoFilter::CRT)
    {
        SDL_SetRenderDrawBlendMode(
            renderer,
            SDL_BLENDMODE_BLEND
        );

        //
        // Color boost glow
        //

        SDL_SetRenderDrawBlendMode(
            renderer,
            SDL_BLENDMODE_ADD
        );

        SDL_SetTextureColorMod(
            texture,
            255,
            255,
            255
        );

        SDL_SetTextureAlphaMod(
            texture,
            20
        );

        SDL_RenderTexture(
            renderer,
            texture,
            nullptr,
            &dstRect
        );

        SDL_SetTextureAlphaMod(
            texture,
            255
        );

        SDL_SetRenderDrawBlendMode(
            renderer,
            SDL_BLENDMODE_BLEND
        );

        // Color boost pass
        SDL_SetTextureColorMod(
            texture,
            255,
            255,
            255
        );

        SDL_SetTextureAlphaMod(
            texture,
            255
        );

        SDL_RenderTexture(
            renderer,
            texture,
            nullptr,
            &dstRect
        );

        //
        // RGB chromatic ghosting
        //

        SDL_FRect ghost = dstRect;


        // Red channel offset
        SDL_SetTextureColorMod(
            texture,
            255,
            20,
            20
        );

        SDL_SetTextureAlphaMod(
            texture,
            120
        );

        ghost.x += 2.0f;


        SDL_RenderTexture(
            renderer,
            texture,
            nullptr,
            &ghost
        );


        // Blue channel offset
        SDL_SetTextureColorMod(
            texture,
            20,
            20,
            255
        );

        SDL_SetTextureAlphaMod(
            texture,
            120
        );        

        ghost.x = dstRect.x - 2.0f;


        SDL_RenderTexture(
            renderer,
            texture,
            nullptr,
            &ghost
        );


        //
        // Main image
        //

        SDL_SetTextureColorMod(
            texture,
            255,
            235,
            215
        );

        SDL_SetTextureAlphaMod(
            texture,
            255
        );


        SDL_RenderTexture(
            renderer,
            texture,
            nullptr,
            &dstRect
        );


        //
        // CRT glass tint
        //

        SDL_SetRenderDrawColor(
            renderer,
            5,
            10,
            15,
            8
        );


        SDL_RenderFillRect(
            renderer,
            &dstRect
        );


        //
        // Scanlines
        //

        SDL_SetRenderDrawColor(
            renderer,
            0,
            0,
            0,
            50
        );


        for(
            int y = (int)dstRect.y;
            y < dstRect.y + dstRect.h;
            y += 4
        )
        {
            SDL_RenderLine(
                renderer,
                dstRect.x,
                y,
                dstRect.x + dstRect.w,
                y
            );
        }
/*
        //
        // CRT glass vignette
        //

        SDL_SetRenderDrawBlendMode(
            renderer,
            SDL_BLENDMODE_BLEND
        );

        // Top edge
        SDL_SetRenderDrawColor(
            renderer,
            0,
            0,
            0,
            30
        );

        SDL_FRect topEdge =
        {
            dstRect.x,
            dstRect.y,
            dstRect.w,
            40
        };

        SDL_RenderFillRect(
            renderer,
            &topEdge
        );


        // Bottom edge
        SDL_FRect bottomEdge =
        {
            dstRect.x,
            dstRect.y + dstRect.h - 40,
            dstRect.w,
            40
        };

        SDL_RenderFillRect(
            renderer,
            &bottomEdge
        );


        // Left edge
        SDL_FRect leftEdge =
        {
            dstRect.x,
            dstRect.y,
            40,
            dstRect.h
        };

        SDL_RenderFillRect(
            renderer,
            &leftEdge
        );


        // Right edge
        SDL_FRect rightEdge =
        {
            dstRect.x + dstRect.w - 40,
            dstRect.y,
            40,
            dstRect.h
        };

        SDL_RenderFillRect(
            renderer,
            &rightEdge
        );

        SDL_SetRenderDrawBlendMode(
            renderer,
            SDL_BLENDMODE_NONE
        );
*/
        return;
    }
}