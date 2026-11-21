#pragma once

// Batched 2D sprite renderer on OpenGL 2.1 (GLSL #version 120).
// Quads sharing one texture accumulate into a streaming VBO and are drawn
// with a single GL_TRIANGLES draw call; the batch flushes on texture change
// or when full, so overall draw order == call order (no sorting).

#include "gl_loader.hpp"

#include <SDL.h>

#include <string>
#include <vector>

struct GlTexture
{
    pvzgl::GLuint_t id = 0;
    int w = 0;
    int h = 0;
    // Mirrors SDL_CreateTextureFromSurface's blend-mode choice: textures from
    // surfaces without an alpha channel are SDL_BLENDMODE_NONE, so the SDL
    // alpha mod had no effect on them. The GL path always blends, so the
    // alpha mod must be ignored for these (drawn with effective alpha 255).
    bool has_alpha = true;
};

class GlSpriteBatch
{
public:
    GlSpriteBatch() = default;
    ~GlSpriteBatch() { shutdown(); }

    // Creates the shader program, streaming VBO and the 1x1 white texture.
    // Requires a current GL context and pvzgl::init() having succeeded.
    bool init(SDL_Window *win, int width, int height);
    void shutdown();
    bool ready() const { return mReady; }

    // Uploads an SDL_Surface (converted to RGBA as needed) as a GL texture.
    GlTexture upload(SDL_Surface *surf);
    void free(GlTexture &tex);

    // Textured quad. Source rect is in texture pixels and may extend past the
    // texture bounds (edge-clamped, matching SDL_RenderCopy's clamp sampling).
    // Rotation is clockwise degrees about the destination rect center
    // (SDL_RenderCopyEx center=NULL semantics); flips mirror the UVs.
    // r,g,b,a are a multiplicative tint (SDL_SetTextureColorMod/AlphaMod).
    void drawQuad(const GlTexture &tex,
                  float sx, float sy, float sw, float sh,
                  float dx, float dy, float dw, float dh,
                  float angleDeg = 0.0f, bool flipH = false, bool flipV = false,
                  uint8_t r = 255, uint8_t g = 255, uint8_t b = 255, uint8_t a = 255);

    // Arbitrary quad: 4 vertices (TL TR BR BL) with normalized UVs, drawn as
    // the two triangles {0,1,2},{0,2,3} (mirrors SDL_RenderGeometry usage).
    struct Vert
    {
        float x, y, u, v;
        uint8_t r, g, b, a;
    };
    void drawGeometry(pvzgl::GLuint_t texId, const Vert verts[4]);

    // Untextured geometry helpers (drawn with the 1x1 white texture).
    void fillRect(float x, float y, float w, float h,
                  uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);
    void drawPoint(int x, int y, uint8_t r, uint8_t g, uint8_t b);
    void drawLine(int x1, int y1, int x2, int y2, uint8_t r, uint8_t g, uint8_t b);
    void drawRect(int x, int y, int w, int h, uint8_t r, uint8_t g, uint8_t b);

    void clear(float r, float g, float b, float a);

    void flush();
    // Screen-space scissor in batch pixel coords (SDL_Rect, top-left origin),
    // applied to everything queued after the call. Changing the clip flushes
    // the pending batch so draw order is unaffected. nullptr clears it; the
    // clip is also reset by clear()/present() so it cannot leak across frames.
    // (repvz Graphics::SetClipRect; the grave-rise emergence clips the zombie
    // body to the ground line.)
    void setClipRect(const SDL_Rect *rect);
    // flush + snapshot the frame into lastFrame() + SDL_GL_SwapWindow +
    // clear the new back buffer to opaque black. GL double-buffer contents
    // are undefined after a swap; clearing keeps frames independent of driver
    // swap semantics (copy vs discard), so old frame contents can never bleed
    // into the next frame.
    void present();
    // Persistent copy of the last presented frame (updated by present()
    // unless frozen). GL-origin texture: draw it with flipV=true.
    const GlTexture &lastFrame() const { return mLastFrame; }
    // Freeze/unfreeze the lastFrame snapshot (Window::fade_out freezes it so
    // its own present() calls do not overwrite the captured scene).
    void setLastFrameFrozen(bool frozen) { mLastFrameFrozen = frozen; }

    const GlTexture &whiteTexture() const { return mWhiteTex; }

private:
    struct GpuVert
    {
        float x, y;    // position
        float u, v;    // texcoord
        float r, g, b, a; // color, normalized
    };

    void pushVert(float x, float y, float u, float v,
                  uint8_t r, uint8_t g, uint8_t b, uint8_t a);
    void pushQuad(pvzgl::GLuint_t texId, const Vert verts[4]);
    pvzgl::GLuint_t compileShader(pvzgl::GLenum_t type, const char *src);

    static const int MAX_QUADS = 16384;

    SDL_Window *mWin = nullptr;
    int mWidth = 0, mHeight = 0;
    bool mReady = false;
    bool mDebug = false;

    pvzgl::GLuint_t mProgram = 0;
    pvzgl::GLuint_t mVbo = 0;
    int mLocMvp = -1;
    int mLocTex = -1;
    int mLocPos = -1;
    int mLocUv = -1;
    int mLocColor = -1;

    GlTexture mWhiteTex;
    GlTexture mLastFrame;      // framebuffer-sized copy taken in present()
    bool mLastFrameFrozen = false;
    pvzgl::GLuint_t mCurTex = 0;
    std::vector<GpuVert> mVerts;

    // per-frame stats (reset by present())
    int mFrameQuads = 0;
    int mFrameDrawCalls = 0;
};
