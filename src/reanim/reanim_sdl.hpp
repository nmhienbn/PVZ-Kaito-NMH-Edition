#pragma once

// Renderer for reanim documents: draws every visible part as a textured
// quad through an ISpriteRenderer so the full Flash-style affine transform
// (incl. shear) is honored. Part alpha is applied via vertex color.

#include "reanim.hpp"
#include "sprite_renderer.hpp"

#include <functional>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace reanim {

struct DrawStats {
    int partsDrawn = 0;
    int missingImages = 0;
    bool hasBounds = false;
    float minX = 0, minY = 0, maxX = 0, maxY = 0;
};

struct DrawOptions {
    SDL_Color tint{255, 255, 255, 255}; // multiplicative tint (cold zombie = blue)
    bool flash = false;                 // extra pass with white-silhouette masks
    Uint8 flashAlpha = 90;              // ~ the game's *_blink overlay strength
    float alphaMul = 1.0f;              // instance-wide alpha multiplier
    bool measureOnly = false;           // gather bounds, skip actual rendering
    float rotateDeg = 0.0f;             // extra screen-space rotation (deg, + = CW)
    SDL_FPoint rotatePivot{0, 0};       // pivot in screen pixels for rotateDeg
    // track name (lowercase) -> replacement part image stem (lowercase).
    // Used for degrade stages: anim_cone -> zombie_cone2, ...
    const std::unordered_map<std::string, std::string>* imageOverrides = nullptr;
    // track name (lowercase) -> local-space translation applied on top of the
    // sampled transform. Companion to imageOverrides for swaps whose
    // replacement sprite has different dimensions than the art the pivots
    // were authored for (e.g. peashooter's leaf slot drawing the small
    // three_leaf fan instead of the big leaf2 curl).
    const std::unordered_map<std::string, SDL_FPoint>* imageOverrideOffsets = nullptr;
    // Tracks listed here render even when their frame has f=-1, matching the
    // original game's ReanimShowTrack: artists keep armor/flag/door tracks
    // hidden (f=-1) during walk/eat and the code reveals them per variant.
    const std::unordered_set<std::string>* forceShow = nullptr;
    // Looping segment bounds (-1 = off): interpolation between the segment's
    // last frame and the next segment's first frame would briefly blend into
    // the *next* animation's pose (e.g. arm flips toward the eat pose at the
    // end of walk); wrap interpolation to the segment's first frame instead.
    int loopWrapFirst = -1, loopWrapLast = -1;
    // Wrap ease length in reanim frames (default 1 = the plain seam
    // interpolation above). >1 cross-blends the window's last frames into
    // the loop's first pose; only used for 2Pak takes with unclosed ends.
    float loopBlendFrames = 1.0f;
    // Optional base transform (PopCap's AttachReanimation): every sampled
    // part transform is composed with this one, so the drawn document rides
    // on a track of another document (e.g. the flag pole in the flag hand).
    const Transform* attachFrom = nullptr;
};

class SdlReanimRenderer {
public:
    explicit SdlReanimRenderer(ISpriteRenderer& r) : mR(r) {}

    // Index a directory of part images (Zombie_body.png, ...); lookup is
    // case-insensitive by stem, .png preferred over .jpg.
    bool BindDirectory(const std::string& dir);
    size_t ImageFilesKnown() const { return mFiles.size(); }

    // Draw all visible tracks of the document at absolute frame `absFrame`
    // (fractional = interpolated), offset by (ox, oy) and uniformly scaled.
    DrawStats Draw(const ReanimDoc& doc, float absFrame, float ox, float oy, float scale,
                   const std::function<bool(const std::string&)>& hideTrack = nullptr,
                   const DrawOptions& options = DrawOptions());

    // Draw one loose part image riding on an anchor transform (the part is
    // composed with `base` at relative offset relX/relY, like a one-track
    // AttachReanimation).
    bool DrawAttachedImage(const std::string& stemLower, const Transform& base,
                           float relX, float relY, float ox, float oy, float scale,
                           const DrawOptions& options = DrawOptions());

    // Pixel size of a loose part image (0x0 when unknown/missing); queries
    // the same texture cache Draw uses.
    void partImageSize(const std::string& stemLower, int& w, int& h);

private:
    ISpriteRenderer::TexHandle GetTexture(const std::string& stemLower);
    ISpriteRenderer::TexHandle GetMaskTexture(const std::string& stemLower); // white silhouette

    // Opaque pixel rect of a part image (x0,y0 inclusive; x1,y1 exclusive, in
    // texture pixels), or nullptr-equivalent when the image is fully
    // transparent. measureOnly draws use it to shrink each part's quad: the
    // 2Pak part PNGs carry generous transparent margins, so the raw quad
    // corner bounds overestimate the on-screen size and would shrink the
    // plant's calibrated scale (garlic ~18%, coffee bean ~65%).
    struct OpaqueRect { float x0, y0, x1, y1; bool any = false; };
    const OpaqueRect* GetOpaqueRect(const std::string& stemLower);

    ISpriteRenderer& mR;
    std::unordered_map<std::string, std::string> mFiles;              // stem(lower) -> path
    std::unordered_map<std::string, ISpriteRenderer::TexHandle> mTextures; // stem(lower) -> texture/null
    std::unordered_map<std::string, ISpriteRenderer::TexHandle> mMasks;    // stem(lower) -> mask/null
    std::unordered_map<std::string, OpaqueRect> mOpaque;                 // stem(lower) -> opaque rect
    std::unordered_map<std::string, bool> mWarned;
};

} // namespace reanim
