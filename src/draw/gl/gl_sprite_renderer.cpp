#include "gl_sprite_renderer.hpp"

ISpriteRenderer::TexHandle GlSpriteRenderer::store(const GlTexture &tex)
{
    if (tex.id == 0)
        return nullptr;
    mTextures[tex.id] = tex;
    return reinterpret_cast<TexHandle>((uintptr_t)tex.id);
}

ISpriteRenderer::TexHandle GlSpriteRenderer::loadFile(const std::string &path)
{
    SDL_Surface *surf = IMG_Load(path.c_str());
    if (surf == nullptr)
        return nullptr;
    TexHandle h = store(mBatch.upload(surf));
    SDL_FreeSurface(surf);
    return h;
}

ISpriteRenderer::TexHandle GlSpriteRenderer::loadSurface(SDL_Surface *surf, bool whiteMask)
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
    TexHandle h = store(mBatch.upload(use));
    if (use != surf)
        SDL_FreeSurface(use);
    return h;
}

void GlSpriteRenderer::textureSize(TexHandle handle, int &w, int &h) const
{
    w = h = 0;
    auto it = mTextures.find((pvzgl::GLuint_t)(uintptr_t)handle);
    if (it != mTextures.end())
    {
        w = it->second.w;
        h = it->second.h;
    }
}

void GlSpriteRenderer::drawQuad(TexHandle handle, const SpriteVertex verts[4])
{
    if (handle == nullptr)
        return;
    GlSpriteBatch::Vert gv[4];
    for (int i = 0; i < 4; i++)
    {
        gv[i].x = verts[i].x;
        gv[i].y = verts[i].y;
        gv[i].u = verts[i].u;
        gv[i].v = verts[i].v;
        gv[i].r = verts[i].r;
        gv[i].g = verts[i].g;
        gv[i].b = verts[i].b;
        gv[i].a = verts[i].a;
    }
    mBatch.drawGeometry((pvzgl::GLuint_t)(uintptr_t)handle, gv);
}

void GlSpriteRenderer::freeAll()
{
    for (auto &kv : mTextures)
        mBatch.free(kv.second);
    mTextures.clear();
}
