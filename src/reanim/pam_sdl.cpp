#include "pam_sdl.hpp"

#include "nlohmann/json.hpp"

#include <SDL_image.h>

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>

namespace pam {

namespace {

std::string upperOf(std::string s) {
    for (char& c : s) c = (char)toupper((unsigned char)c);
    return s;
}

// "atlases\SodRollGroup_1536_00" -> "<dir>/SODROLLGROUP_1536_00.PNG"
std::string resolveAtlasPng(const std::string& dir, const std::string& entryPath) {
    std::string name = entryPath;
    size_t sep = name.find_last_of("\\/");
    if (sep != std::string::npos) name = name.substr(sep + 1);
    namespace fs = std::filesystem;
    std::error_code ec;
    const std::string cand[4] = {
        dir + "/" + upperOf(name) + ".PNG",
        dir + "/" + name + ".png",
        dir + "/" + name + ".PNG",
        dir + "/" + upperOf(name) + ".png",
    };
    for (const std::string& c : cand)
        if (fs::is_regular_file(c, ec)) return c;
    return cand[0]; // for the error message
}

} // namespace

bool SdlPamRenderer::LoadAtlasMap(const std::string& mapJsonPath,
                                  const std::string& atlasDir, std::string& err) {
    std::ifstream in(mapJsonPath);
    if (!in) {
        err = "cannot open " + mapJsonPath;
        return false;
    }
    nlohmann::json j;
    try {
        in >> j;
    } catch (const std::exception& e) {
        err = mapJsonPath + ": " + e.what();
        return false;
    }
    if (!j.is_object()) {
        err = mapJsonPath + ": not a json object";
        return false;
    }

    // First pass: atlas roots; second: sprite rects (children reference them
    // by id, order in the json is not guaranteed).
    for (auto& [id, v] : j.items()) {
        if (!v.is_object() || !v.value("atlas", false)) continue;
        AtlasDef a;
        a.id = id;
        a.pngPath = resolveAtlasPng(atlasDir, v.value("path", ""));
        a.w = v.value("width", 0);
        a.h = v.value("height", 0);
        std::error_code ec;
        if (!std::filesystem::is_regular_file(a.pngPath, ec))
            SDL_Log("pam: atlas png missing: %s", a.pngPath.c_str());
        mAtlasById[a.id] = (int)mAtlases.size();
        mAtlases.push_back(a);
    }
    for (auto& [id, v] : j.items()) {
        if (!v.is_object() || v.value("atlas", false)) continue;
        auto pit = mAtlasById.find(v.value("parent", ""));
        if (pit == mAtlasById.end()) continue;
        SpriteRect r;
        r.atlas = pit->second;
        r.x = v.value("ax", 0.0f);
        r.y = v.value("ay", 0.0f);
        r.w = v.value("aw", 0.0f);
        r.h = v.value("ah", 0.0f);
        mRects[id] = r;
    }
    mTextures.assign(mAtlases.size(), nullptr);
    mMasks.assign(mAtlases.size(), nullptr);
    mMaskTried.assign(mAtlases.size(), false);
    if (mAtlases.empty()) {
        err = mapJsonPath + ": no atlas entries";
        return false;
    }
    return !mRects.empty();
}

ISpriteRenderer::TexHandle SdlPamRenderer::GetTexture(int atlasIndex) {
    AtlasDef& a = mAtlases[atlasIndex];
    if (a.tried) return mTextures[atlasIndex];
    a.tried = true;
    ISpriteRenderer::TexHandle tex = mR.loadFile(a.pngPath);
    if (!tex)
        SDL_Log("pam: cannot load atlas %s: %s", a.pngPath.c_str(), IMG_GetError());
    mTextures[atlasIndex] = tex;
    return tex;
}

ISpriteRenderer::TexHandle SdlPamRenderer::GetMaskTexture(int atlasIndex) {
    if (mMaskTried[atlasIndex]) return mMasks[atlasIndex];
    mMaskTried[atlasIndex] = true;
    ISpriteRenderer::TexHandle tex = nullptr;
    SDL_Surface* raw = IMG_Load(mAtlases[atlasIndex].pngPath.c_str());
    if (raw) {
        tex = mR.loadSurface(raw, /*whiteMask=*/true);
        SDL_FreeSurface(raw);
    }
    mMasks[atlasIndex] = tex;
    return tex;
}

PamDrawStats SdlPamRenderer::Draw(const PamDoc& doc,
                                  const std::vector<DrawPart>& parts, float ox,
                                  float oy, float scale,
                                  const PamDrawOptions& options) {
    PamDrawStats stats;
    static const float us[4] = {0, 1, 1, 0}; // quad corners TL TR BR BL
    static const float vs[4] = {0, 0, 1, 1};
    for (const DrawPart& part : parts) {
        const ImageDef& img = doc.images[part.image];
        auto it = mRects.find(img.resourceId);
        if (it == mRects.end()) {
            stats.missingImages++;
            if (mWarned.insert(img.resourceId).second)
                SDL_Log("pam: no atlas rect for '%s'", img.resourceId.c_str());
            continue;
        }
        float fa = part.color.a * options.alphaMul;
        if (fa <= 0.0f) continue;
        const SpriteRect& rect = it->second;
        ISpriteRenderer::TexHandle tex = GetTexture(rect.atlas);
        if (!tex) {
            stats.missingImages++;
            continue;
        }
        int tw = 0, th = 0;
        mR.textureSize(tex, tw, th);
        if (tw <= 0 || th <= 0) continue;

        SpriteVertex vtx[4];
        Uint8 vr = (Uint8)std::clamp(lroundf(options.tint.r * part.color.r), 0l, 255l);
        Uint8 vg = (Uint8)std::clamp(lroundf(options.tint.g * part.color.g), 0l, 255l);
        Uint8 vb = (Uint8)std::clamp(lroundf(options.tint.b * part.color.b), 0l, 255l);
        Uint8 va = (Uint8)std::clamp(lroundf(fa * 255.0f), 0l, 255l);
        for (int i = 0; i < 4; i++) {
            float u = us[i] * img.w, v = vs[i] * img.h;
            const Mat& m = part.world;
            float X = ox + scale * (m.a * u + m.c * v + m.tx);
            float Y = oy + scale * (m.b * u + m.d * v + m.ty);
            vtx[i].x = X;
            vtx[i].y = Y;
            vtx[i].r = vr;
            vtx[i].g = vg;
            vtx[i].b = vb;
            vtx[i].a = va;
            // Sprite rect is at atlas (1536-class) pixel density; stretch it
            // onto the PAM display size (0,0)-(img.w, img.h).
            vtx[i].u = (rect.x + us[i] * rect.w) / (float)tw;
            vtx[i].v = (rect.y + vs[i] * rect.h) / (float)th;
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
                ISpriteRenderer::TexHandle mask = GetMaskTexture(rect.atlas);
                if (mask) {
                    for (int i = 0; i < 4; i++) {
                        vtx[i].r = vtx[i].g = vtx[i].b = 255;
                        // fade ramp * the part's own alpha, same as the reanim
                        // path (reanim_sdl.cpp flash pass)
                        vtx[i].a = (Uint8)std::clamp(lroundf(options.flashAlpha * (va / 255.0f)), 0l, 255l);
                    }
                    mR.drawQuad(mask, vtx);
                }
            }
        }
    }
    return stats;
}

} // namespace pam
