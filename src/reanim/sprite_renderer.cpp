#include "sprite_renderer.hpp"

#include <SDL_image.h>

ISpriteRenderer::TexHandle SdlSpriteRenderer::loadFile(const std::string &path)
{
    SDL_Texture *tex = IMG_LoadTexture(mR, path.c_str());
    if (tex)
        SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
    return tex;
}

ISpriteRenderer::TexHandle SdlSpriteRenderer::loadSurface(SDL_Surface *surf, bool whiteMask)
{
    if (surf == nullptr)
        return nullptr;
    SDL_Surface *use = surf;
    if (whiteMask)
    {
        use = SDL_ConvertSurfaceFormat(surf, SDL_PIXELFORMAT_RGBA32, 0);
        if (use == nullptr)
            return nullptr;
        Uint32 *px = (Uint32 *)use->pixels;
        size_t count = (size_t)use->w * use->h;
        for (size_t i = 0; i < count; i++)
            px[i] = (px[i] & 0xFF000000u) | 0x00FFFFFFu; // white RGB, keep alpha
    }
    SDL_Texture *tex = SDL_CreateTextureFromSurface(mR, use);
    if (use != surf)
        SDL_FreeSurface(use);
    if (tex)
        SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
    return tex;
}

void SdlSpriteRenderer::textureSize(TexHandle handle, int &w, int &h) const
{
    w = h = 0;
    if (handle)
        SDL_QueryTexture((SDL_Texture *)handle, nullptr, nullptr, &w, &h);
}

void SdlSpriteRenderer::drawQuad(TexHandle handle, const SpriteVertex verts[4])
{
    if (handle == nullptr)
        return;
    SDL_Vertex vtx[4];
    for (int i = 0; i < 4; i++)
    {
        vtx[i].position = SDL_FPoint{verts[i].x, verts[i].y};
        vtx[i].color = SDL_Color{verts[i].r, verts[i].g, verts[i].b, verts[i].a};
        vtx[i].tex_coord = SDL_FPoint{verts[i].u, verts[i].v};
    }
    static const int idx[6] = {0, 1, 2, 0, 2, 3};
    SDL_RenderGeometry(mR, (SDL_Texture *)handle, vtx, 4, idx, 6);
}
