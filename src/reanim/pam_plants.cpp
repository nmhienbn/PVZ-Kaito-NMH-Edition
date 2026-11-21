#include "pam_plants.hpp"

#include "draw/ImageType.hpp"
#include "draw/world_scale.hpp"
#include "game_param.hpp"

#include <algorithm>
#include <cmath>
#include <unordered_map>

namespace pamplant {

namespace {

const char* PAM_DIR = "./resources/pvz2anim";

struct PlantGroup {
    const char* pamFile;
    const char* mapFile;
    const char* refSegment; // placement calibration reference
    pam::PamDoc doc;
    pam::SdlPamRenderer* renderer = nullptr;
    bool loaded = false;
    // placement calibration measured from refSegment, in PAM units
    bool calibrated = false;
    float scale = 1.0f;
    float refCX = 0.0f, refBottom = 0.0f;
};

// Indexed by enum PlantType (game_param.hpp); nullptr entries = no PAM.
const int GROUP_COUNT = 6;
PlantGroup gGroups[GROUP_COUNT] = {
    /* PEASHOOTER_TYPE */ {"pam/PEASHOOTER.pam.json", "atlas_map/PlantPeashooter_1536.json", "idle"},
    /* SUNFLOWER_TYPE */ {"pam/SUNFLOWER.pam.json", "atlas_map/PlantSunflower_1536.json", "idle"},
    /* WALLNUT_TYPE */ {"pam/WALLNUT.pam.json", "atlas_map/PlantWallnut_1536.json", "idle"},
    /* SNOWPEA_TYPE */ {"pam/SNOWPEA.pam.json", "atlas_map/PlantSnowPea_1536.json", "idle"},
    /* POTATOMINE_TYPE */ {"pam/POTATOMINE.pam.json", "atlas_map/PlantPotatomine_1536.json", "idle"},
    /* CHERRYBOMB_TYPE */ {"pam/CHERRYBOMB.pam.json", "atlas_map/PlantCherryBomb_1536.json", "idle"},
};

bool gReady = false;
bool gInitTried = false;

struct SegPick {
    const char* segment; // nullptr = no PAM mapping (fallback)
    bool sync = true;    // true: local frame from sheet frame; false: pam clock loop
    bool clamp = false;  // one-shot look: hold the last frame at sheet end
};

// sheet directory -> pam segment, per plant type
SegPick pickSegment(int type, int dir, bool sheetPinned) {
    switch (type) {
    case PEASHOOTER_TYPE:
    case SNOWPEA_TYPE: {
        int sheet = type == PEASHOOTER_TYPE ? PEASHOOTER_SHEET_DIRECTORY
                                            : SNOWPEA_SHEET_DIRECTORY;
        int attack = type == PEASHOOTER_TYPE ? PEASHOOTER_ATTACK_DIRECTORY
                                             : SNOWPEA_ATTACK_DIRECTORY;
        if (dir == sheet) return {"idle", true, false};
        if (dir == attack) return {"attack", true, false};
        return {nullptr};
    }
    case SUNFLOWER_TYPE:
        if (dir == SUNFLOWER_SHEET_DIRECTORY) return {"idle", true, false};
        if (dir == SUNFLOWER_HAPPY_DIRECTORY) return {"special", true, false};
        return {nullptr};
    case WALLNUT_TYPE:
        // 5 sheet stages over 4 look segments: 1 unharmed, 2-3 cracked,
        // 4 worse, 5 nearly dead (stage picked by health in
        // WallNut::determine_appearance).
        switch (dir) {
        case WALLNUT_1_DIRECTORY: return {"idle", true, false};
        case WALLNUT_2_DIRECTORY:
        case WALLNUT_3_DIRECTORY: return {"damage", true, false};
        case WALLNUT_4_DIRECTORY: return {"damage2", true, false};
        case WALLNUT_5_DIRECTORY: return {"damage3", true, false};
        default: return {nullptr};
        }
    case CHERRYBOMB_TYPE:
        // growing sheet = the attack segment; the explosion sheet has no pam
        // counterpart and keeps the sprite-sheet fallback.
        if (dir == CHERRYBOMB_SHEET_DIRECTORY) return {"attack", true, true};
        return {nullptr};
    case POTATOMINE_TYPE:
        if (dir == POTATOMINE_UNARMED_DIRECTORY) {
            // sheet pins its last frame until arming time: play `plant` once,
            // then hold on the 2-frame `plant_idle` breathing loop.
            if (sheetPinned) return {"plant_idle", false, false};
            return {"plant", true, true};
        }
        if (dir == POTATOMINE_RECOVER_DIRECTORY) return {"recover", true, true};
        if (dir == POTATOMINE_IDLE_DIRECTORY) return {"idle", true, false};
        if (dir == POTATOMINE_ATTACK_DIRECTORY) return {"attack", true, true};
        return {nullptr}; // explosion sheet: fallback
    default:
        return {nullptr};
    }
}

// Measure the reference segment bounds (PAM units) and derive scale/anchor so
// the pam drawing lands where the sprite sheet used to be.
bool calibrate(PlantGroup& g, int gameHeight) {
    pam::Segment ref;
    if (!pam::PamPlayer(g.doc).SegmentRange(g.refSegment, ref)) return false;
    pam::PamDrawOptions measure;
    measure.measureOnly = true;
    pam::PamDrawStats acc;
    pam::PamPlayer player(g.doc);
    std::vector<pam::DrawPart> parts;
    for (int f = ref.first; f <= ref.last; f += 4) {
        parts.clear();
        player.EvalSprite(-1, f, pam::Mat{}, pam::ColorMul{}, parts);
        pam::PamDrawStats st = g.renderer->Draw(g.doc, parts, 0, 0, 1.0f, measure);
        if (!st.hasBounds) continue;
        if (!acc.hasBounds) {
            acc = st;
        } else {
            acc.minX = std::min(acc.minX, st.minX);
            acc.minY = std::min(acc.minY, st.minY);
            acc.maxX = std::max(acc.maxX, st.maxX);
            acc.maxY = std::max(acc.maxY, st.maxY);
        }
    }
    if (!acc.hasBounds) return false;
    float h = acc.maxY - acc.minY;
    if (h <= 1.0f) return false;
    g.scale = gameHeight / h;
    g.refCX = (acc.minX + acc.maxX) * 0.5f;
    g.refBottom = acc.maxY;
    g.calibrated = true;
    return true;
}

} // namespace

bool init(ISpriteRenderer& renderer) {
    if (gInitTried) return gReady;
    gInitTried = true;
    int loadedCount = 0;
    for (int t = 0; t < GROUP_COUNT; t++) {
        PlantGroup& g = gGroups[t];
        std::string err;
        if (!pam::LoadPamJson(std::string(PAM_DIR) + "/" + g.pamFile, g.doc, err)) {
            SDL_Log("pamplant: %s (plant %d keeps sprite sheets)", err.c_str(), t);
            continue;
        }
        g.renderer = new pam::SdlPamRenderer(renderer);
        if (!g.renderer->LoadAtlasMap(std::string(PAM_DIR) + "/" + g.mapFile,
                                      PAM_DIR + std::string("/atlases"), err)) {
            SDL_Log("pamplant: %s (plant %d keeps sprite sheets)", err.c_str(), t);
            continue;
        }
        g.loaded = true;
        loadedCount++;
    }
    gReady = loadedCount > 0;
    SDL_Log("pamplant: %d/%d pam groups loaded", loadedCount, GROUP_COUNT);
    return gReady;
}

bool ready() { return gReady; }

float frameRate(int plantType) {
    if (plantType < 0 || plantType >= GROUP_COUNT) return 0.0f;
    if (!gGroups[plantType].loaded) return 0.0f;
    return gGroups[plantType].doc.frameRate;
}

bool drawPlant(int plantType, int sheetDir, float sframe, int nSheet,
               float pamClock, int cellX1, int cellX2, int footY,
               int gameHeight, bool flash, float flash_fade) {
    if (!gReady) return false;
    if (plantType < 0 || plantType >= GROUP_COUNT) return false;
    PlantGroup& g = gGroups[plantType];
    if (!g.loaded) return false;

    bool sheetPinned = nSheet > 0 && sframe >= (float)(nSheet - 1);
    SegPick pick = pickSegment(plantType, sheetDir, sheetPinned);
    if (!pick.segment) return false;
    pam::Segment seg;
    if (!pam::PamPlayer(g.doc).SegmentRange(pick.segment, seg)) return false;
    int segLen = seg.last - seg.first + 1;
    if (segLen <= 0) return false;

    float local;
    if (pick.sync) {
        // Fractional sheet frame maps fractionally into the segment: the PAM
        // evaluator interpolates between frames, so the pose moves every game
        // tick instead of stepping at sheet fps.
        local = nSheet > 0 ? sframe * (float)segLen / (float)nSheet : 0.0f;
        if (local > (float)(segLen - 1))
            local = (float)(segLen - 1); // one-shot sheets clamp at the end
    } else {
        local = pamClock;
    }

    if (!g.calibrated && !calibrate(g, gameHeight)) return false;
    // World knob: grow about the same bottom-center anchor (footY, cell
    // center). No preview path calls this (only the game hook).
    const float scale = g.scale * WORLD_SCALE;
    float ox = (cellX1 + cellX2) * 0.5f - scale * g.refCX;
    float oy = footY - scale * g.refBottom;

    std::vector<pam::DrawPart> parts;
    pam::PamPlayer(g.doc).EvalSegment(seg, local, parts);
    pam::PamDrawOptions opt;
    opt.flash = flash;
    // fade-out over the blink budget (game hook feeds attacked_time /
    // MAX_TIME_BLINK); clamps keep the default strength when unfaded
    opt.flashAlpha = (Uint8)std::clamp(lroundf(opt.flashAlpha * flash_fade), 0l, 255l);
    pam::PamDrawStats st = g.renderer->Draw(g.doc, parts, ox, oy, scale, opt);
    return st.partsDrawn > 0;
}

} // namespace pamplant
