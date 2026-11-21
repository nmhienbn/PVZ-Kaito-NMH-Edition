#pragma once

// ISpriteRenderer implementation that feeds quads into the game's
// GlSpriteBatch (OpenGL backend). Demos do not use this; they stay on
// SdlSpriteRenderer so they still run headless.

#include "gl_sprite_batch.hpp"
#include "reanim/sprite_renderer.hpp"

#include <SDL_image.h>

#include <unordered_map>

class GlSpriteRenderer : public ISpriteRenderer
{
public:
    explicit GlSpriteRenderer(GlSpriteBatch &batch) : mBatch(batch) {}
    ~GlSpriteRenderer() override { freeAll(); }

    TexHandle loadFile(const std::string &path) override;
    TexHandle loadSurface(SDL_Surface *surf, bool whiteMask) override;
    void textureSize(TexHandle handle, int &w, int &h) const override;
    void drawQuad(TexHandle handle, const SpriteVertex verts[4]) override;
    void setClipRect(const SDL_Rect *rect) override { mBatch.setClipRect(rect); }

    // Frees every texture uploaded through this renderer (GL context must
    // still be alive).
    void freeAll();

private:
    TexHandle store(const GlTexture &tex);

    GlSpriteBatch &mBatch;
    // handle == GlTexture::id as void*; map remembers per-texture sizes.
    std::unordered_map<pvzgl::GLuint_t, GlTexture> mTextures;
};
