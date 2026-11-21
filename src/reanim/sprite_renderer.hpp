#pragma once

// Backend-neutral textured-quad renderer used by the reanim/PAM animators.
// This mirrors exactly what the SDL_RenderGeometry paths need: load an image
// file or an (optionally white-silhouette-edited) surface into a texture
// handle, then draw transformed quads as 4 vertices + the implicit triangle
// list {0,1,2,  0,2,3}. The game runs on the GL implementation
// (src/draw/gl/gl_sprite_renderer.*) while the headless demos keep using the
// SDL_Renderer one defined here.

#include <SDL.h>

#include <string>

// One screen-space quad corner. UVs are normalized to the texture.
struct SpriteVertex
{
    float x = 0, y = 0;                     // position
    float u = 0, v = 0;                     // texcoord, 0..1
    Uint8 r = 255, g = 255, b = 255, a = 255; // multiplicative tint
};

class ISpriteRenderer
{
public:
    // Opaque per-backend texture handle. nullptr == load failed (cached nulls
    // are allowed and meaningful: "don't retry this stem").
    typedef void *TexHandle;

    virtual ~ISpriteRenderer() = default;

    virtual TexHandle loadFile(const std::string &path) = 0;
    // Caller keeps ownership of `surf`. whiteMask rewrites pixels to a white
    // silhouette (white RGB, alpha kept) before upload.
    virtual TexHandle loadSurface(SDL_Surface *surf, bool whiteMask) = 0;
    virtual void textureSize(TexHandle handle, int &w, int &h) const = 0;
    // verts must be ordered TL TR BR BL; drawn as {0,1,2},{0,2,3}. Null
    // handles are ignored.
    virtual void drawQuad(TexHandle handle, const SpriteVertex verts[4]) = 0;
    // Screen-space clip in pixel coords (top-left origin), applying until
    // cleared with nullptr. The grave-rise emergence uses this to cut the
    // zombie body off at the ground line (repvz Graphics::SetClipRect in
    // Zombie::DrawReanim, Zombie.cpp:5689).
    virtual void setClipRect(const SDL_Rect *rect) = 0;
};

// SDL_Renderer backend (used by the demos; needs no GL context).
class SdlSpriteRenderer : public ISpriteRenderer
{
public:
    explicit SdlSpriteRenderer(SDL_Renderer *r) : mR(r) {}

    TexHandle loadFile(const std::string &path) override;
    TexHandle loadSurface(SDL_Surface *surf, bool whiteMask) override;
    void textureSize(TexHandle handle, int &w, int &h) const override;
    void drawQuad(TexHandle handle, const SpriteVertex verts[4]) override;
    void setClipRect(const SDL_Rect *rect) override { SDL_RenderSetClipRect(mR, rect); }

private:
    SDL_Renderer *mR;
};
