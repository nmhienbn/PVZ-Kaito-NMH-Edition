#pragma once

// Renderer for PAM documents: a "group" is one pam.json plus its PopStudio
// atlas-map json plus the referenced atlas PNG(s). Every resolved draw part
// is emitted as a textured quad through an ISpriteRenderer so the full
// Flash-style affine transform (incl. shear) is honored; the part color
// multiplier is applied via vertex color. Atlas textures are loaded lazily.

#include "pam.hpp"
#include "sprite_renderer.hpp"

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace pam {

struct PamDrawStats {
    int partsDrawn = 0;
    int missingImages = 0;
    bool hasBounds = false;
    float minX = 0, minY = 0, maxX = 0, maxY = 0;
};

struct PamDrawOptions {
    SDL_Color tint{255, 255, 255, 255}; // multiplicative tint
    float alphaMul = 1.0f;              // instance-wide alpha multiplier
    bool measureOnly = false;           // gather bounds, skip actual rendering
    bool flash = false;                 // extra pass with white-silhouette masks
    Uint8 flashAlpha = 70;              // ~ the game's *_blink overlay strength
};

class SdlPamRenderer {
public:
    explicit SdlPamRenderer(ISpriteRenderer& r) : mR(r) {}

    // Index a PopStudio atlas-map json (resourceId -> sprite rect); atlas
    // PNGs are looked up under atlasDir (atlas "path" name upper-cased, e.g.
    // "atlases\SodRollGroup_1536_00" -> SODROLLGROUP_1536_00.PNG).
    bool LoadAtlasMap(const std::string& mapJsonPath, const std::string& atlasDir,
                      std::string& err);
    size_t ResourcesKnown() const { return mRects.size(); }

    // Draw resolved parts offset by (ox, oy) and uniformly scaled.
    PamDrawStats Draw(const PamDoc& doc, const std::vector<DrawPart>& parts,
                      float ox, float oy, float scale,
                      const PamDrawOptions& options = PamDrawOptions());

private:
    struct AtlasDef {
        std::string id;
        std::string pngPath;
        int w = 0, h = 0;
        bool tried = false;
    };
    struct SpriteRect {
        int atlas = -1; // index into mAtlases
        float x = 0, y = 0, w = 0, h = 0;
    };

    ISpriteRenderer::TexHandle GetTexture(int atlasIndex);
    ISpriteRenderer::TexHandle GetMaskTexture(int atlasIndex); // white silhouette

    ISpriteRenderer& mR;
    std::vector<AtlasDef> mAtlases;
    std::unordered_map<std::string, int> mAtlasById;   // atlas id -> mAtlases idx
    std::unordered_map<std::string, SpriteRect> mRects; // resourceId -> rect
    std::vector<ISpriteRenderer::TexHandle> mTextures;  // per-atlas, lazy/null
    std::vector<ISpriteRenderer::TexHandle> mMasks;     // per-atlas white masks
    std::vector<bool> mMaskTried;
    std::unordered_set<std::string> mWarned;
};

} // namespace pam
