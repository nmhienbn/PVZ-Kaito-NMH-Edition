#include "gl_sprite_batch.hpp"

#include "events/scene_dump/scene_dump.hpp"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

using namespace pvzgl;

namespace {

const char *kVertexShader =
    "#version 120\n"
    "attribute vec2 aPos;\n"
    "attribute vec2 aUV;\n"
    "attribute vec4 aColor;\n"
    "uniform mat4 uMVP;\n"
    "varying vec2 vUV;\n"
    "varying vec4 vColor;\n"
    "void main() {\n"
    "    vUV = aUV;\n"
    "    vColor = aColor;\n"
    "    gl_Position = uMVP * vec4(aPos, 0.0, 1.0);\n"
    "}\n";

const char *kFragmentShader =
    "#version 120\n"
    "uniform sampler2D uTex;\n"
    "varying vec2 vUV;\n"
    "varying vec4 vColor;\n"
    "void main() {\n"
    "    gl_FragColor = texture2D(uTex, vUV) * vColor;\n"
    "}\n";

} // namespace

GLuint_t GlSpriteBatch::compileShader(GLenum_t type, const char *src)
{
    GLuint_t sh = glCreateShader(type);
    GLint_t len = (GLint_t)strlen(src);
    glShaderSource(sh, 1, &src, &len);
    glCompileShader(sh);
    GLint_t ok = PVZ_GL_FALSE;
    glGetShaderiv(sh, PVZ_GL_COMPILE_STATUS, &ok);
    if (ok != PVZ_GL_TRUE)
    {
        char log[1024];
        GLsizei_t n = 0;
        glGetShaderInfoLog(sh, (GLsizei_t)sizeof(log), &n, log);
        printf("pvzgl: shader compile failed: %s\n", log);
    }
    return sh;
}

bool GlSpriteBatch::init(SDL_Window *win, int width, int height)
{
    mWin = win;
    mWidth = width;
    mHeight = height;
    mDebug = getenv("PVZ_DEBUG_BATCH") != nullptr &&
             std::string(getenv("PVZ_DEBUG_BATCH")) != "0";

    GLuint_t vs = compileShader(PVZ_GL_VERTEX_SHADER, kVertexShader);
    GLuint_t fs = compileShader(PVZ_GL_FRAGMENT_SHADER, kFragmentShader);
    mProgram = glCreateProgram();
    glAttachShader(mProgram, vs);
    glAttachShader(mProgram, fs);
    glLinkProgram(mProgram);
    glDeleteShader(vs);
    glDeleteShader(fs);
    GLint_t linked = PVZ_GL_FALSE;
    glGetProgramiv(mProgram, PVZ_GL_LINK_STATUS, &linked);
    if (linked != PVZ_GL_TRUE)
    {
        char log[1024];
        GLsizei_t n = 0;
        glGetProgramInfoLog(mProgram, (GLsizei_t)sizeof(log), &n, log);
        printf("pvzgl: program link failed: %s\n", log);
        return false;
    }

    mLocMvp = glGetUniformLocation(mProgram, "uMVP");
    mLocTex = glGetUniformLocation(mProgram, "uTex");
    mLocPos = glGetAttribLocation(mProgram, "aPos");
    mLocUv = glGetAttribLocation(mProgram, "aUV");
    mLocColor = glGetAttribLocation(mProgram, "aColor");

    glGenBuffers(1, &mVbo);

    // Orthographic projection: pixel coords, (0,0) top-left (y-down).
    const float w = (float)mWidth, h = (float)mHeight;
    const float mvp[16] = {
        2.0f / w, 0, 0, 0,
        0, -2.0f / h, 0, 0,
        0, 0, -1.0f, 0,
        -1.0f, 1.0f, 0, 1.0f};

    glViewport(0, 0, mWidth, mHeight);
    glDisable(PVZ_GL_DEPTH_TEST);
    glEnable(PVZ_GL_BLEND);
    glBlendFunc(PVZ_GL_SRC_ALPHA, PVZ_GL_ONE_MINUS_SRC_ALPHA);
    glUseProgram(mProgram);
    glUniformMatrix4fv(mLocMvp, 1, PVZ_GL_FALSE, mvp);
    glUniform1i(mLocTex, 0);

    // 1x1 white texture backing fill/line/point/rect and untextured quads.
    const uint8_t whitePx[4] = {255, 255, 255, 255};
    glGenTextures(1, &mWhiteTex.id);
    glBindTexture(PVZ_GL_TEXTURE_2D, mWhiteTex.id);
    glTexParameteri(PVZ_GL_TEXTURE_2D, PVZ_GL_TEXTURE_MIN_FILTER, PVZ_GL_NEAREST);
    glTexParameteri(PVZ_GL_TEXTURE_2D, PVZ_GL_TEXTURE_MAG_FILTER, PVZ_GL_NEAREST);
    glTexParameteri(PVZ_GL_TEXTURE_2D, PVZ_GL_TEXTURE_WRAP_S, PVZ_GL_CLAMP_TO_EDGE);
    glTexParameteri(PVZ_GL_TEXTURE_2D, PVZ_GL_TEXTURE_WRAP_T, PVZ_GL_CLAMP_TO_EDGE);
    glTexImage2D(PVZ_GL_TEXTURE_2D, 0, PVZ_GL_RGBA, 1, 1, 0,
                 PVZ_GL_RGBA, PVZ_GL_UNSIGNED_BYTE, whitePx);
    mWhiteTex.w = mWhiteTex.h = 1;

    // lastFrame snapshot target (contents filled by present()).
    glGenTextures(1, &mLastFrame.id);
    glBindTexture(PVZ_GL_TEXTURE_2D, mLastFrame.id);
    glTexParameteri(PVZ_GL_TEXTURE_2D, PVZ_GL_TEXTURE_MIN_FILTER, PVZ_GL_NEAREST);
    glTexParameteri(PVZ_GL_TEXTURE_2D, PVZ_GL_TEXTURE_MAG_FILTER, PVZ_GL_NEAREST);
    glTexParameteri(PVZ_GL_TEXTURE_2D, PVZ_GL_TEXTURE_WRAP_S, PVZ_GL_CLAMP_TO_EDGE);
    glTexParameteri(PVZ_GL_TEXTURE_2D, PVZ_GL_TEXTURE_WRAP_T, PVZ_GL_CLAMP_TO_EDGE);
    glTexImage2D(PVZ_GL_TEXTURE_2D, 0, PVZ_GL_RGBA, mWidth, mHeight, 0,
                 PVZ_GL_RGBA, PVZ_GL_UNSIGNED_BYTE, nullptr);
    mLastFrame.w = mWidth;
    mLastFrame.h = mHeight;

    mVerts.reserve((size_t)MAX_QUADS * 6);
    mReady = true;
    return true;
}

void GlSpriteBatch::shutdown()
{
    if (!mReady)
        return;
    flush();
    if (mWhiteTex.id != 0)
    {
        glDeleteTextures(1, &mWhiteTex.id);
        mWhiteTex.id = 0;
    }
    if (mLastFrame.id != 0)
    {
        glDeleteTextures(1, &mLastFrame.id);
        mLastFrame.id = 0;
    }
    if (mVbo != 0)
    {
        glDeleteBuffers(1, &mVbo);
        mVbo = 0;
    }
    if (mProgram != 0)
    {
        glDeleteProgram(mProgram);
        mProgram = 0;
    }
    mReady = false;
}

GlTexture GlSpriteBatch::upload(SDL_Surface *surf)
{
    GlTexture tex;
    if (surf == nullptr)
        return tex;
    // Detect alpha from the ORIGINAL surface (before RGBA conversion gives an
    // Amask to everything): SDL_BLENDMODE_BLEND vs NONE in
    // SDL_CreateTextureFromSurface. Paletted PNGs may carry alpha via tRNS.
    bool has_alpha = surf->format->Amask != 0;
    if (!has_alpha && surf->format->palette != nullptr)
        for (int i = 0; i < surf->format->palette->ncolors && !has_alpha; i++)
            has_alpha = surf->format->palette->colors[i].a != 255;
    tex.has_alpha = has_alpha;
    // GL_RGBA/GL_UNSIGNED_BYTE wants R first in memory == SDL RGBA32 on
    // little-endian.
    SDL_Surface *rgba = surf;
    if (surf->format->format != SDL_PIXELFORMAT_RGBA32)
        rgba = SDL_ConvertSurfaceFormat(surf, SDL_PIXELFORMAT_RGBA32, 0);
    if (rgba == nullptr)
        return tex;

    // GL 2.1 only guarantees GL_MAX_TEXTURE_SIZE >= 2048 per dimension, and
    // some PvZ2 atlases are ~2376 px tall: downscale oversized surfaces so
    // they still describe a texture on capped hardware.
    static GLint_t maxTexSize = 0;
    if (maxTexSize == 0)
        glGetIntegerv(PVZ_GL_MAX_TEXTURE_SIZE, &maxTexSize);
    if (maxTexSize > 0 && (rgba->w > maxTexSize || rgba->h > maxTexSize))
    {
        double sc = (double)maxTexSize / (rgba->w > rgba->h ? rgba->w : rgba->h);
        SDL_Surface *scaled = SDL_CreateRGBSurfaceWithFormat(
            0, (int)(rgba->w * sc), (int)(rgba->h * sc), 32, SDL_PIXELFORMAT_RGBA32);
        if (scaled != nullptr)
        {
            SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
            SDL_Rect dst = {0, 0, scaled->w, scaled->h};
            SDL_BlitScaled(rgba, nullptr, scaled, &dst);
            if (rgba != surf)
                SDL_FreeSurface(rgba);
            rgba = scaled;
            printf("pvzgl: downscaled texture to %dx%d (GL_MAX_TEXTURE_SIZE=%d)\n",
                   scaled->w, scaled->h, (int)maxTexSize);
        }
    }

    // Textures upload lazily mid-frame (load_texture() during draw): flush the
    // quads batched so far BEFORE rebinding, or they would be drawn later
    // while the just-uploaded texture (or none) is bound — black/garbage
    // sprites for the rest of the frame.
    flush();

    glGenTextures(1, &tex.id);
    glBindTexture(PVZ_GL_TEXTURE_2D, tex.id);
    glTexParameteri(PVZ_GL_TEXTURE_2D, PVZ_GL_TEXTURE_MIN_FILTER, PVZ_GL_LINEAR);
    glTexParameteri(PVZ_GL_TEXTURE_2D, PVZ_GL_TEXTURE_MAG_FILTER, PVZ_GL_LINEAR);
    glTexParameteri(PVZ_GL_TEXTURE_2D, PVZ_GL_TEXTURE_WRAP_S, PVZ_GL_CLAMP_TO_EDGE);
    glTexParameteri(PVZ_GL_TEXTURE_2D, PVZ_GL_TEXTURE_WRAP_T, PVZ_GL_CLAMP_TO_EDGE);
    glTexImage2D(PVZ_GL_TEXTURE_2D, 0, PVZ_GL_RGBA, rgba->w, rgba->h, 0,
                 PVZ_GL_RGBA, PVZ_GL_UNSIGNED_BYTE, rgba->pixels);
    tex.w = rgba->w;
    tex.h = rgba->h;
    if (rgba != surf)
        SDL_FreeSurface(rgba);
    return tex;
}

void GlSpriteBatch::free(GlTexture &tex)
{
    if (tex.id != 0)
    {
        // SDL_DestroyTexture flushes SDL's deferred render queue first; do the
        // same so quads already batched against this texture are not lost.
        flush();
        glDeleteTextures(1, &tex.id);
        if (mCurTex == tex.id)
            mCurTex = 0;
        tex.id = 0;
    }
    tex.w = tex.h = 0;
}

void GlSpriteBatch::pushVert(float x, float y, float u, float v,
                             uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    GpuVert gv;
    gv.x = x;
    gv.y = y;
    gv.u = u;
    gv.v = v;
    gv.r = r / 255.0f;
    gv.g = g / 255.0f;
    gv.b = b / 255.0f;
    gv.a = a / 255.0f;
    mVerts.push_back(gv);
}

void GlSpriteBatch::pushQuad(GLuint_t texId, const Vert verts[4])
{
    // PVZ_SCENE_DUMP: while one entity's display is being measured, union this
    // quad's screen-space corners into its drawn bbox. Inert (one branch) in
    // normal play.
    if (scene_dump::g_capture_active)
    {
        float xs[4] = {verts[0].x, verts[1].x, verts[2].x, verts[3].x};
        float ys[4] = {verts[0].y, verts[1].y, verts[2].y, verts[3].y};
        scene_dump::capture_quad(xs, ys);
    }
    if (texId != mCurTex || (int)mVerts.size() + 6 > MAX_QUADS * 6)
    {
        flush();
        mCurTex = texId;
    }
    static const int idx[6] = {0, 1, 2, 0, 2, 3};
    for (int i = 0; i < 6; i++)
    {
        const Vert &v = verts[idx[i]];
        pushVert(v.x, v.y, v.u, v.v, v.r, v.g, v.b, v.a);
    }
    mFrameQuads++;
}

void GlSpriteBatch::drawQuad(const GlTexture &tex,
                             float sx, float sy, float sw, float sh,
                             float dx, float dy, float dw, float dh,
                             float angleDeg, bool flipH, bool flipV,
                             uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    if (!mReady || tex.id == 0 || tex.w <= 0 || tex.h <= 0)
        return;

    // Corner positions, TL TR BR BL, rotated about the dst center.
    float px[4] = {dx, dx + dw, dx + dw, dx};
    float py[4] = {dy, dy, dy + dh, dy + dh};
    if (angleDeg != 0.0f)
    {
        // y is down on screen, so the plain CCW formula looks clockwise.
        const float rad = angleDeg * (float)M_PI / 180.0f;
        const float c = cosf(rad), s = sinf(rad);
        const float cx = dx + dw * 0.5f, cy = dy + dh * 0.5f;
        for (int i = 0; i < 4; i++)
        {
            float ox = px[i] - cx, oy = py[i] - cy;
            px[i] = cx + c * ox - s * oy;
            py[i] = cy + s * ox + c * oy;
        }
    }

    // UVs, mirrored on flip. Src rect may exceed the texture (clamped by
    // GL_CLAMP_TO_EDGE sampling, like SDL_RenderCopy with an oversized src).
    float u0 = sx / tex.w, u1 = (sx + sw) / tex.w;
    float v0 = sy / tex.h, v1 = (sy + sh) / tex.h;
    if (flipH)
    {
        float t = u0;
        u0 = u1;
        u1 = t;
    }
    if (flipV)
    {
        float t = v0;
        v0 = v1;
        v1 = t;
    }
    Vert verts[4] = {
        {px[0], py[0], u0, v0, r, g, b, a},
        {px[1], py[1], u1, v0, r, g, b, a},
        {px[2], py[2], u1, v1, r, g, b, a},
        {px[3], py[3], u0, v1, r, g, b, a},
    };
    pushQuad(tex.id, verts);
}

void GlSpriteBatch::drawGeometry(GLuint_t texId, const Vert verts[4])
{
    if (!mReady || texId == 0)
        return;
    pushQuad(texId, verts);
}

void GlSpriteBatch::fillRect(float x, float y, float w, float h,
                             uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    if (w <= 0 || h <= 0)
        return;
    drawQuad(mWhiteTex, 0, 0, 1, 1, x, y, w, h, 0.0f, false, false, r, g, b, a);
}

void GlSpriteBatch::drawPoint(int x, int y, uint8_t r, uint8_t g, uint8_t b)
{
    fillRect((float)x, (float)y, 1, 1, r, g, b);
}

void GlSpriteBatch::drawLine(int x1, int y1, int x2, int y2,
                             uint8_t r, uint8_t g, uint8_t b)
{
    // SDL_RenderDrawLine includes both endpoints; extend the end point one
    // pixel along the line direction.
    float ex = (float)(x2 + (x2 > x1) - (x2 < x1));
    float ey = (float)(y2 + (y2 > y1) - (y2 < y1));
    float vx = ex - x1, vy = ey - y1;
    float len = sqrtf(vx * vx + vy * vy);
    if (len < 0.5f)
    {
        drawPoint(x1, y1, r, g, b);
        return;
    }
    // half-pixel perpendicular offset keeps the line 1px thick
    float nx = -vy / len * 0.5f, ny = vx / len * 0.5f;
    Vert verts[4] = {
        {x1 - nx, y1 - ny, 0, 0, r, g, b, 255},
        {x1 + nx, y1 + ny, 1, 0, r, g, b, 255},
        {ex + nx, ey + ny, 1, 1, r, g, b, 255},
        {ex - nx, ey - ny, 0, 1, r, g, b, 255},
    };
    pushQuad(mWhiteTex.id, verts);
}

void GlSpriteBatch::drawRect(int x, int y, int w, int h,
                             uint8_t r, uint8_t g, uint8_t b)
{
    drawLine(x, y, x + w, y, r, g, b);
    drawLine(x, y, x, y + h, r, g, b);
    drawLine(x, y + h, x + w, y + h, r, g, b);
    drawLine(x + w, y, x + w, y + h, r, g, b);
}

void GlSpriteBatch::setClipRect(const SDL_Rect *rect)
{
    if (!mReady)
        return;
    flush(); // the clip applies strictly after what is already queued
    if (rect != nullptr)
    {
        glEnable(PVZ_GL_SCISSOR_TEST);
        // GL scissor counts from the bottom-left; batch coords are top-left.
        glScissor(rect->x, mHeight - (rect->y + rect->h), rect->w, rect->h);
    }
    else
    {
        glDisable(PVZ_GL_SCISSOR_TEST);
    }
}

void GlSpriteBatch::clear(float r, float g, float b, float a)
{
    if (!mReady)
        return;
    // SDL_RenderClear runs in-order in SDL's command queue: quads batched
    // before the clear must be drawn before it.
    flush();
    glDisable(PVZ_GL_SCISSOR_TEST); // a stale clip never shrinks the clear
    glClearColor(r, g, b, a);
    glClear(PVZ_GL_COLOR_BUFFER_BIT);
}

void GlSpriteBatch::flush()
{
    if (!mReady || mVerts.empty())
        return;

    glUseProgram(mProgram);
    glActiveTexture(PVZ_GL_TEXTURE0);
    glBindTexture(PVZ_GL_TEXTURE_2D, mCurTex);

    glBindBuffer(PVZ_GL_ARRAY_BUFFER, mVbo);
    glBufferData(PVZ_GL_ARRAY_BUFFER, (GLsizeiptr_t)(mVerts.size() * sizeof(GpuVert)),
                 nullptr, PVZ_GL_STREAM_DRAW); // orphan previous contents
    glBufferSubData(PVZ_GL_ARRAY_BUFFER, 0,
                    (GLsizeiptr_t)(mVerts.size() * sizeof(GpuVert)), mVerts.data());

    const GLsizei_t stride = (GLsizei_t)sizeof(GpuVert);
    glVertexAttribPointer((GLuint_t)mLocPos, 2, PVZ_GL_FLOAT, PVZ_GL_FALSE, stride,
                          (const void *)offsetof(GpuVert, x));
    glEnableVertexAttribArray((GLuint_t)mLocPos);
    glVertexAttribPointer((GLuint_t)mLocUv, 2, PVZ_GL_FLOAT, PVZ_GL_FALSE, stride,
                          (const void *)offsetof(GpuVert, u));
    glEnableVertexAttribArray((GLuint_t)mLocUv);
    glVertexAttribPointer((GLuint_t)mLocColor, 4, PVZ_GL_FLOAT, PVZ_GL_FALSE, stride,
                          (const void *)offsetof(GpuVert, r));
    glEnableVertexAttribArray((GLuint_t)mLocColor);

    glDrawArrays(PVZ_GL_TRIANGLES, 0, (GLsizei_t)mVerts.size());

    mVerts.clear();
    mCurTex = 0;
    mFrameDrawCalls++;
}

void GlSpriteBatch::present()
{
    flush();
    if (mReady && mWin != nullptr)
    {
        // Keep a GPU-side copy of the frame (GL-origin) for fade_out before
        // the swap destroys/garbles the visible buffer.
        if (mLastFrame.id != 0 && !mLastFrameFrozen)
        {
            glBindTexture(PVZ_GL_TEXTURE_2D, mLastFrame.id);
            glCopyTexSubImage2D(PVZ_GL_TEXTURE_2D, 0, 0, 0, 0, 0, mWidth, mHeight);
        }
        SDL_GL_SwapWindow(mWin);
        // The post-swap back buffer is undefined; reset it so the next frame
        // never depends on (or leaks) leftover contents.
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(PVZ_GL_COLOR_BUFFER_BIT);
        // Never carry a stale scissor clip into the next frame (the grave-rise
        // emergence sets one around the riser's ground line).
        glDisable(PVZ_GL_SCISSOR_TEST);
    }
    if (mDebug)
        printf("pvzgl batch: %d quads, %d draw calls\n", mFrameQuads, mFrameDrawCalls);
    mFrameQuads = 0;
    mFrameDrawCalls = 0;
}
