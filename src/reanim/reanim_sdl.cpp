#include "reanim_sdl.hpp"

#include <SDL_image.h>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <filesystem>

namespace reanim {

namespace {

std::string lowerOf(std::string s) {
    for (char& c : s) c = (char)tolower((unsigned char)c);
    return s;
}

bool endsWith(const std::string& s, const char* suffix) {
    size_t n = strlen(suffix);
    return s.size() >= n && s.compare(s.size() - n, n, suffix) == 0;
}

} // namespace

bool SdlReanimRenderer::BindDirectory(const std::string& dir) {
    namespace fs = std::filesystem;
    std::error_code ec;
    for (const auto& entry : fs::directory_iterator(dir, ec)) {
        if (!entry.is_regular_file()) continue;
        std::string ext = lowerOf(entry.path().extension().string());
        if (ext != ".png" && ext != ".jpg" && ext != ".jpeg") continue;
        std::string stem = lowerOf(entry.path().stem().string());
        auto it = mFiles.find(stem);
        if (it == mFiles.end() ||
            (ext == ".png" && !endsWith(lowerOf(it->second), ".png"))) {
            mFiles[stem] = entry.path().string();
        }
    }
    return !mFiles.empty();
}

ISpriteRenderer::TexHandle SdlReanimRenderer::GetTexture(const std::string& stemLower) {
    auto cached = mTextures.find(stemLower);
    if (cached != mTextures.end()) return cached->second;

    ISpriteRenderer::TexHandle tex = nullptr;
    auto fileIt = mFiles.find(stemLower);
    if (fileIt != mFiles.end())
        tex = mR.loadFile(fileIt->second);
    mTextures[stemLower] = tex;
    return tex;
}

ISpriteRenderer::TexHandle SdlReanimRenderer::GetMaskTexture(const std::string& stemLower) {
    auto cached = mMasks.find(stemLower);
    if (cached != mMasks.end()) return cached->second;

    ISpriteRenderer::TexHandle tex = nullptr;
    auto fileIt = mFiles.find(stemLower);
    if (fileIt != mFiles.end()) {
        SDL_Surface* raw = IMG_Load(fileIt->second.c_str());
        if (raw) {
            tex = mR.loadSurface(raw, /*whiteMask=*/true);
            SDL_FreeSurface(raw);
        }
    }
    mMasks[stemLower] = tex;
    return tex;
}

void SdlReanimRenderer::partImageSize(const std::string& stemLower, int& w, int& h) {
    w = h = 0;
    ISpriteRenderer::TexHandle tex = GetTexture(stemLower);
    if (tex) mR.textureSize(tex, w, h);
}

const SdlReanimRenderer::OpaqueRect*
SdlReanimRenderer::GetOpaqueRect(const std::string& stemLower) {
    auto cached = mOpaque.find(stemLower);
    if (cached != mOpaque.end()) return &cached->second;

    OpaqueRect r{0, 0, 0, 0, false};
    auto fileIt = mFiles.find(stemLower);
    if (fileIt != mFiles.end()) {
        SDL_Surface* raw = IMG_Load(fileIt->second.c_str());
        if (raw) {
            SDL_Surface* s = SDL_ConvertSurfaceFormat(raw, SDL_PIXELFORMAT_RGBA32, 0);
            SDL_FreeSurface(raw);
            if (s) {
                const int w = s->w, h = s->h;
                int x0 = w, y0 = h, x1 = -1, y1 = -1;
                SDL_LockSurface(s);
                for (int y = 0; y < h; y++) {
                    const Uint32* row =
                        (const Uint32*)((const Uint8*)s->pixels + (size_t)y * s->pitch);
                    for (int x = 0; x < w; x++) {
                        if ((row[x] >> 24) > 8) { // alpha byte, RGBA32 byte order
                            if (x < x0) x0 = x;
                            if (x > x1) x1 = x;
                            if (y < y0) y0 = y;
                            if (y > y1) y1 = y;
                        }
                    }
                }
                SDL_UnlockSurface(s);
                SDL_FreeSurface(s);
                r.any = x1 >= x0;
                if (r.any) {
                    r.x0 = (float)x0; r.y0 = (float)y0;
                    r.x1 = (float)(x1 + 1); r.y1 = (float)(y1 + 1);
                }
            }
        }
    }
    return &mOpaque.emplace(stemLower, r).first->second;
}

DrawStats SdlReanimRenderer::Draw(const ReanimDoc& doc, float absFrame,
                                  float ox, float oy, float scale,
                                  const std::function<bool(const std::string&)>& hideTrack,
                                  const DrawOptions& options) {
    static const float D2R = 3.14159265358979323846f / 180.0f;
    DrawStats stats;
    for (const Track& tr : doc.tracks) {
        if (!tr.hasAnyImage) continue;
        std::string nameLower = lowerOf(tr.name);
        if (hideTrack && hideTrack(tr.name)) continue;
        Transform t = SampleTrack(tr, absFrame,
                                  options.loopWrapFirst, options.loopWrapLast,
                                  options.loopBlendFrames);
        if (t.f < -0.5f) {                             // hidden
            if (!options.forceShow || !options.forceShow->count(nameLower))
                continue;
        }
        if (t.image < 0 || t.image >= (int)doc.images.size()) continue;

        std::string stem;
        SDL_FPoint off{0, 0};
        if (options.imageOverrides) {
            auto ov = options.imageOverrides->find(nameLower);
            if (ov != options.imageOverrides->end()) stem = ov->second;
        }
        if (options.imageOverrideOffsets) {
            auto oo = options.imageOverrideOffsets->find(nameLower);
            if (oo != options.imageOverrideOffsets->end()) off = oo->second;
        }
        if (stem.empty())
            stem = ImageFileStem(doc.images[t.image]);

        float af = t.a * options.alphaMul;
        Uint8 alpha = (Uint8)std::clamp(lroundf(af * 255.0f), 0l, 255l);
        if (alpha == 0) continue;

        ISpriteRenderer::TexHandle tex = GetTexture(stem);
        if (!tex) {
            stats.missingImages++;
            if (!mWarned[stem]) {
                mWarned[stem] = true;
                SDL_Log("reanim: no image file for part '%s'", stem.c_str());
            }
            continue;
        }

        int w = 0, h = 0;
        mR.textureSize(tex, w, h);
        // measureOnly bounds follow the part images' opaque pixels, not the
        // full quads (2Pak part PNGs have wide transparent margins).
        const OpaqueRect* opaq = nullptr;
        if (options.measureOnly) {
            opaq = GetOpaqueRect(stem);
            if (!opaq->any) continue; // fully translucent part: no size
        }
        float kxr = t.kx * D2R, kyr = -t.ky * D2R;
        float a = cosf(kxr) * t.sx, b = sinf(kxr) * t.sx;
        float c = sinf(kyr) * t.sy, d = cosf(kyr) * t.sy;
        float tx = t.x + off.x, ty = t.y + off.y;
        if (options.attachFrom) {
            const Transform& p = *options.attachFrom;
            float pkxr = p.kx * D2R, pkyr = -p.ky * D2R;
            float A = cosf(pkxr) * p.sx, B = sinf(pkxr) * p.sx;
            float C = sinf(pkyr) * p.sy, D = cosf(pkyr) * p.sy;
            tx = A * t.x + C * t.y + p.x;
            ty = B * t.x + D * t.y + p.y;
            float na = A * a + C * b, nb = B * a + D * b;
            float nc = A * c + C * d, nd = B * c + D * d;
            a = na; b = nb; c = nc; d = nd;
        }

        const float us[4] = {0, 1, 1, 0};
        const float vs[4] = {0, 0, 1, 1};
        const bool doRot = options.rotateDeg != 0.0f;
        float rc = 1.0f, rs = 0.0f;
        if (doRot) {
            float rr = options.rotateDeg * D2R;
            rc = cosf(rr);
            rs = sinf(rr);
        }
        SpriteVertex vtx[4];
        for (int i = 0; i < 4; i++) {
            float u = us[i] * (float)w, vv = vs[i] * (float)h;
            if (opaq) {
                u = opaq->x0 + us[i] * (opaq->x1 - opaq->x0);
                vv = opaq->y0 + vs[i] * (opaq->y1 - opaq->y0);
            }
            float X = ox + scale * (a * u + c * vv + tx);
            float Y = oy + scale * (b * u + d * vv + ty);
            if (doRot) {
                float dx = X - options.rotatePivot.x, dy = Y - options.rotatePivot.y;
                X = options.rotatePivot.x + rc * dx - rs * dy;
                Y = options.rotatePivot.y + rs * dx + rc * dy;
            }
            vtx[i].x = X;
            vtx[i].y = Y;
            vtx[i].r = options.tint.r;
            vtx[i].g = options.tint.g;
            vtx[i].b = options.tint.b;
            vtx[i].a = alpha;
            vtx[i].u = us[i];
            vtx[i].v = vs[i];
            if (!stats.hasBounds) {
                stats.minX = stats.maxX = X;
                stats.minY = stats.maxY = Y;
                stats.hasBounds = true;
            } else {
                stats.minX = std::min(stats.minX, X);
                stats.maxX = std::max(stats.maxX, X);
                stats.minY = std::min(stats.minY, Y);
                stats.maxY = std::max(stats.maxY, Y);
            }
        }
        if (!options.measureOnly) {
            mR.drawQuad(tex, vtx);
            stats.partsDrawn++;

            if (options.flash) {
                ISpriteRenderer::TexHandle mask = GetMaskTexture(stem);
                if (mask) {
                    for (int i = 0; i < 4; i++) {
                        vtx[i].r = vtx[i].g = vtx[i].b = 255;
                        vtx[i].a = (Uint8)std::clamp(lroundf(options.flashAlpha * (vtx[i].a / 255.0f)), 0l, 255l);
                    }
                    mR.drawQuad(mask, vtx);
                }
            }
        }
    }
    return stats;
}

bool SdlReanimRenderer::DrawAttachedImage(const std::string& stemLower, const Transform& base,
                                          float relX, float relY, float ox, float oy, float scale,
                                          const DrawOptions& options) {
    ISpriteRenderer::TexHandle tex = GetTexture(stemLower);
    if (!tex) {
        if (!mWarned[stemLower]) {
            mWarned[stemLower] = true;
            SDL_Log("reanim: no image file for part '%s'", stemLower.c_str());
        }
        return false;
    }
    // child transform = identity at (relX, relY), composed with the anchor
    // (same composition as DrawOptions::attachFrom inside Draw).
    static const float D2R = 3.14159265358979323846f / 180.0f;
    float pkxr = base.kx * D2R, pkyr = -base.ky * D2R;
    float A = cosf(pkxr) * base.sx, B = sinf(pkxr) * base.sx;
    float C = sinf(pkyr) * base.sy, D = cosf(pkyr) * base.sy;
    float tx = A * relX + C * relY + base.x;
    float ty = B * relX + D * relY + base.y;

    int w = 0, h = 0;
    mR.textureSize(tex, w, h);
    const float us[4] = {0, 1, 1, 0};
    const float vs[4] = {0, 0, 1, 1};
    SpriteVertex vtx[4];
    for (int i = 0; i < 4; i++) {
        float u = us[i] * (float)w, vv = vs[i] * (float)h;
        vtx[i].x = ox + scale * (A * u + C * vv + tx);
        vtx[i].y = oy + scale * (B * u + D * vv + ty);
        vtx[i].r = options.tint.r;
        vtx[i].g = options.tint.g;
        vtx[i].b = options.tint.b;
        vtx[i].a = (Uint8)std::clamp(lroundf(options.alphaMul * 255.0f), 0l, 255l);
        vtx[i].u = us[i];
        vtx[i].v = vs[i];
    }
    if (!options.measureOnly) {
        mR.drawQuad(tex, vtx);
        if (options.flash) {
            ISpriteRenderer::TexHandle mask = GetMaskTexture(stemLower);
            if (mask) {
                for (int i = 0; i < 4; i++) {
                    vtx[i].r = vtx[i].g = vtx[i].b = 255;
                    vtx[i].a = (Uint8)std::clamp(lroundf(options.flashAlpha * (vtx[i].a / 255.0f)), 0l, 255l);
                }
                mR.drawQuad(mask, vtx);
            }
        }
    }
    return true;
}

} // namespace reanim
