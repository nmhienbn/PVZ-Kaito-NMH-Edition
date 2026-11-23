#include "reanim_zombie.hpp"

#include "reanim.hpp"
#include "reanim_sdl.hpp"

#include "draw/ImageType.hpp"
// HEAD_SPEED_SCALE (the restored 1.0 head-speed scale): one constant out of
// the zombie header, no class use in here.
#include "elements/zombies/zombie_struct.hpp"
#include "game_param.hpp" // ZombieType enum for charredKindForType (constants only)

#include <algorithm>
#include <cmath>
#include <cstring>
#include <unordered_set>

namespace reanimz {

namespace {

reanim::ReanimDoc gDoc;
reanim::ReanimDoc gFlagDoc;
reanim::SdlReanimRenderer* gRenderer = nullptr;
bool gReady = false;
bool gFlagReady = false;
bool gInitTried = false;
// Flag doc loops over its whole timeline; a seam-duplicate final frame is
// trimmed here (the 2Pak doc closes its loop with a copy of the first pose).
float gFlagLoopLen = 0.0f;

// Placement calibration (computed in init from the walk-segment bbox).
float gScale = 1.0f;
float gAnchorDX = 0.0f; // ox = x_location + gAnchorDX
float gAnchorDY = 0.0f; // oy = y_location + gAnchorDY
float gFloorY = 0.0f;   // reanim-units maxY of the walk bbox (zombie floor line)

// Big Trouble Little Zombie (3-5) shrink factor (1.0 = off), set by the
// game hook (reanim_game_hooks.cpp) from level.special. Applied around the
// feet point so the shrunken art stays planted on the same ground pixel.
float gZombieScaleMul = 1.0f;

// Big Trouble Little Zombie (3-5) animation speed factor (1.0 = off), set by
// the game hook from level.special. The mini zombies also shuffle faster
// (Zombie::speed_px_per_tick), so their reanim playback is sped up to match: the
// drawn frame runs at reanim_time * gZombieTimeMul while the game-side
// reanim_time counter keeps its normal rate (0.2 frames/tick).
float gZombieTimeMul = 1.0f;

// Local time inside a segment after applying the big-trouble timescale.
float timeScaledLocal(float reanim_time, float len) {
    float t = reanim_time < 0 ? 0 : reanim_time;
    return fmodf(t * gZombieTimeMul, len);
}

// reanim_time runs in 12-fps frame units (0.2 per 60Hz tick). Retail pvz1
// docs carry their authored fps in the file (most are 12, but e.g. the digger
// is 16 and the zamboni 30), so skin docs scale time into their own frames to
// keep the original playback speed.
float docFpsMul(const reanim::ReanimDoc& doc) { return doc.fps / 12.0f; }

// Some 2Pak looping segments close with a duplicate of their first frame
// (reanim::SegmentEndsOnLoopSeam detects it per segment: true for the base
// doc's walk/eat/idle, gargantuar walk, pogo, yeti walk; others don't carry
// the duplicate). Playing that duplicate as a full frame shows the loop's
// first pose twice per cycle (a visible hitch), so looping play windows end
// one frame earlier when a seam is detected. The duplicate frame stays
// reachable as SampleTrack's wrap-blend target. One-shot segments keep their
// raw last.
int loopPlayLast(const reanim::ReanimDoc& doc, int first, int last) {
    return reanim::SegmentEndsOnLoopSeam(doc, first, last) ? last - 1 : last;
}

// Extra character skins (pole vaulter, newspaper, football, dancer, and the
// area 3-5 zombies): full clone-anatomy reanim self-contained docs,
// calibrated the same way as the base zombie.

struct SkinGeom {
    float scale = 1.0f, anchorDX = 0.0f, anchorDY = 0.0f, floorY = 0.0f;
};

struct ExtraSkinDef {
    const char* file;
    const char* walkSeg;     // calibration/anim reference segment
    const char* fallbackSeg; // tried when walkSeg is missing (nullptr = skip)
    bool calibNoPaper = false; // newspaper: calibrate on the no-paper body
};

reanim::ReanimDoc gSkinDoc[COUNT_ZOMBIE_SKIN]; // index 0 unused (plain doc)
bool gSkinReady[COUNT_ZOMBIE_SKIN] = {};
SkinGeom gSkinGeom[COUNT_ZOMBIE_SKIN];

const ExtraSkinDef EXTRA_SKINS[COUNT_ZOMBIE_SKIN] = {
    /* 0 PLAIN       */ {nullptr, nullptr, nullptr},
    /* 1 POLEVAULT   */ {"Zombie_polevaulter.reanim", "anim_walk", nullptr},
    /* 2 NEWSPAPER   */ {"Zombie_paper.reanim", "anim_walk_nopaper", "anim_walk"},
    /* 3 FOOTBALL    */ {"Zombie_football.reanim", "anim_walk", nullptr},
    /* 4 DANCER      */ {"Zombie_disco.reanim", "anim_walk", nullptr},
    /* 5 SNORKEL     */ {"Zombie_snorkle.reanim", "anim_walk", nullptr},
    /* 6 ZOMBONI     */ {"Zombie_zamboni.reanim", "anim_drive", nullptr},
    /* 7 BOBSLED     */ {"Zombie_bobsled.reanim", "anim_walk", nullptr},
    /* 8 DOLPHINRIDER*/ {"zombie_dolphinrider.reanim", "anim_walk", nullptr},
    /* 9 JACKBOX     */ {"Zombie_jackbox.reanim", "anim_walk", nullptr},
    /* 10 BALLOON    */ {"Zombie_balloon.reanim", "anim_walk", nullptr},
    /* 11 DIGGER     */ {"Zombie_digger.reanim", "anim_walk", nullptr},
    /* 12 POGO       */ {"Zombie_pogo.reanim", "anim_pogo", "anim_walk"},
    /* 13 BUNGEE     */ {"Zombie_bungi.reanim", "anim_hold", "anim_idle"},
    /* 14 LADDER     */ {"Zombie_ladder.reanim", "anim_ladderwalk", "anim_walk"},
    /* 15 CATAPULT   */ {"Zombie_catapult.reanim", "anim_walk", "anim_idle"},
    /* 16 GARGANTUAR */ {"Zombie_gargantuar.reanim", "anim_walk", "anim_idle"},
    /* 17 IMP        */ {"Zombie_imp.reanim", "anim_walk", nullptr},
    /* 18 ZOMBOSS    */ {"Zombie_boss.reanim", "anim_idle", nullptr},
    /* 19 FIREBALL   */ {"Zombie_boss_fireball.reanim", "anim_role", nullptr},
    /* 20 ICEBALL    */ {"Zombie_boss_iceball.reanim", "anim_role", nullptr},
    /* 21 BACKUP     */ {"Zombie_backup.reanim", "anim_walk", nullptr},
    /* 22 YETI       */ {"Zombie_yeti.reanim", "anim_walk", nullptr},
};

const reanim::ReanimDoc* skinDoc(int skin) {
    if (skin == SKIN_PLAIN) return gReady ? &gDoc : nullptr;
    if (skin > SKIN_PLAIN && skin < COUNT_ZOMBIE_SKIN && gSkinReady[skin])
        return &gSkinDoc[skin];
    return nullptr;
}

SkinGeom baseGeom() { return SkinGeom{gScale, gAnchorDX, gAnchorDY, gFloorY}; }

SkinGeom geomFor(int skin) {
    if (skin > SKIN_PLAIN && skin < COUNT_ZOMBIE_SKIN)
        return gSkinGeom[skin];
    return baseGeom();
}

const char* REANIM_DIR = "./resources/reanim";
const char* REANIM_FILE = "Zombie.reanim";
const char* FLAG_FILE = "Zombie_flagpole.reanim";

// Game conventions: zombie box is 166x144, feet at ART_BOTTOM_Y below the
// box top. Original-engine chain (repvz): Zombie::GetPosYBasedOnRow =
// Board::GetPosYBasedOnRow - 30 (Zombie.cpp:8075-8098), the reanim draw
// origin sits at mPosY - 8 (Zombie.cpp:5323), and the retail walk art's feet
// land ~49 px above the box bottom at final scale -- so the original plants
// the feet at ~y+136 (row top + ~87; verified by measuring DrawStats of the
// walk loop against a lane grid). The old sprite sheets' opaque bottom edge
// was y+141, which stood the reanim zombies ~5 px deeper into their lane
// than the original.
const float ART_BOTTOM_Y = 136.0f;
// On-screen scale shared by every zombie skin. The zombie docs are now the
// 2Pak HD set (resources/reanim, converted from assets_src/2Pak_PvZ compiled
// reanim). The 2Pak walk bbox measures 92.9x117.2 reanim units while the
// retail pvz1 set it replaced measured 93.9x137.6 (retail included the tall
// anim_hair strand; 2Pak has no visible hair), so the scale is bumped to keep
// the ON-SCREEN walk bbox exactly the size it rendered at before the swap:
//   ZOMBIE_SCALE = 1.01 * 137.59 / 117.17 = 1.186
// The extra bulk is the 2Pak art's own proportions (larger head/coat), which
// is intended. This is FIXED, not fitted per frame: re-deriving the scale
// from each doc's bbox would re-scale every zombie for a packaging detail
// instead of art. It is the DEFAULT: the 2Pak docs don't share one unit size
// against the retail set (R scatters 0.77..1.22 over skins), so the skins
// whose walk height missed retail by >10% carry their own scale below
// (scale_audit §8.1: scale = ZOMBIE_SCALE * H_retail / H_2pak).
const float ZOMBIE_SCALE = 1.186f;
// Per-skin on-screen scale; unlisted skins keep the shared ZOMBIE_SCALE.
// Multiplies in at calibration time (initSkin/initCharred/base init), so the
// editor/registry scale-mul (setZombieScaleMul) stays orthogonal on top.
float skin_scale(int skin) {
    switch (skin) {
    case SKIN_BACKUP:     return 0.80f; // = 1.186*142/211 (retail x0.8, Zombie.cpp:657)
    case SKIN_GARGANTUAR: return 0.82f; // = 1.186*237/343
    // Dancer now uses the disco doc (2Pak zombie_jackson: moonwalk/point set),
    // not the bare-backup-body Zombie_dancer.reanim it replaced. Its walk bbox
    // is 160.4 units vs the old doc's 132.4; the factor keeps the dancer's
    // on-screen walk height at the calibrated ~114 px (retail x0.8,
    // Zombie.cpp:638; almanac DISCO_DRAW_SCALE was derived from this size).
    case SKIN_DANCER:     return 0.71f; // = 0.86*132.4/160.4 (disco doc swap)
    case SKIN_ZOMBONI:    return 0.96f; // = 1.186*169/208
    case SKIN_POGO:       return 1.00f; // = 1.186*133/158
    case SKIN_ZOMBOSS:    return 1.00f; // = 1.186*680/806
    case SKIN_BUNGEE:     return 1.01f; // = 1.186*106/125
    case SKIN_DIGGER:     return 1.02f; // = 1.186*153/178
    case SKIN_BALLOON:    return 1.05f; // = 1.186*130/147
    case SKIN_CATAPULT:   return 1.07f; // = 1.186*159/177
    case SKIN_SNORKEL:    return 1.08f; // = 1.186*170/187
    default:              return ZOMBIE_SCALE;
    }
}
// Horizontal anchor of the walk-bbox center. The original draws the zombie
// reanim with its origin at mX + 15 (repvz Zombie.cpp:5309, scale 1), which
// puts the walk-bbox center at about mX + 61. The old 123 aligned the art to
// the clone's own sprite sheets, not the retail game; the retail per-type
// ImageOffsetX/Y below is applied on top of this shared anchor.
const float MASS_CENTER_X = 61.0f;
// Retail per-type ImageOffsetX/Y (repvz ZombieDefinition image_offset): added
// on top of the shared +61 walk-bbox anchor. Skins without a retail entry
// keep (0,0).
void skinImageOffset(int skin, float& dx, float& dy) {
    dx = 0.0f;
    dy = 0.0f;
    switch (skin) {
    case SKIN_POLEVAULT:   dx = -6.0f;  dy = -11.0f; break;
    case SKIN_ZOMBONI:     dx = 68.0f;  dy = -23.0f; break;
    case SKIN_BOBSLED:     dx = 0.0f;   dy = -12.0f; break;
    case SKIN_BALLOON:     dx = 0.0f;   dy = 17.0f;  break;
    case SKIN_POGO:        dx = 0.0f;   dy = 16.0f;  break;
    case SKIN_CATAPULT:    dx = -25.0f; dy = -18.0f; break;
    case SKIN_GARGANTUAR:  dx = 0.0f;   dy = -8.0f;  break;
    case SKIN_FOOTBALL:    dx = 0.0f;   dy = -16.0f; break;
    case SKIN_YETI:        dx = 0.0f;   dy = -20.0f; break;
    default: break;
    }
}
// The current part PNGs are cropped to the opaque art, so the walk-bbox
// maxY IS the feet line; older exports carried ~6px (@game scale) of
// transparent padding under the feet, which this constant compensated.
const float ART_PAD_BOTTOM = 0.0f;

// Big Trouble Little Zombie drawing geometry: apply gZombieScaleMul around
// the walk anchor (bbox center / feet line), so the art shrinks in place
// instead of floating off the lawn.
void zombieDrawGeom(const SkinGeom& geo, float x_location, float y_location,
                    float& scale, float& ox, float& oy) {
    const float k = gZombieScaleMul;
    scale = geo.scale * k;
    ox = x_location + MASS_CENTER_X - k * (MASS_CENTER_X - geo.anchorDX);
    oy = y_location + ART_BOTTOM_Y - k * (ART_BOTTOM_Y + ART_PAD_BOTTOM - geo.anchorDY);
}

std::string lowerOf(std::string s) {
    for (char& c : s) c = (char)tolower((unsigned char)c);
    return s;
}

// Accessory tracks a plain zombie never shows (mirrors the original engine's
// ReanimShowTrack/ReanimShowPrefix calls). 2Pak-only: the doc has no
// anim_tongue/neck/tie tracks (tongue art isn't a separate part there).
const char* kBaseHidden[] = {
    "anim_cone", "anim_bucket", "zombie_flaghand", "anim_screendoor",
    "zombie_innerarm_screendoor", "zombie_innerarm_screendoor_hand",
    "zombie_outerarm_screendoor", "zombie_duckytube", "zombie_whitewater",
    "zombie_whitewater2", "zombie_mustache",
};

struct Spec {
    bool ok = false;
    int first = 0, last = 0;  // absolute frame range in the reanim file
    int skin = SKIN_PLAIN;     // which reanim doc backs this state
    const char* segName = nullptr; // segment name (playback-rate lookup)
    bool tongue = false;      // show anim_tongue (eating)
    bool armless = false;     // outer arm fell off
    bool flag = false;        // flag-bearer hand
    bool ducky = false;       // ducky-tube: plain doc with the tube visible
    int door = 0;             // screen-door degrade stage, 0 = no door
    int cone = 0, bucket = 0; // helmet degrade stage, 0 = none
    int paperStage = 0;       // newspaper damage stage (2, 3), 0 = intact
};

int doorWalkStage(int d) { // 0 = not a door directory
    switch (d) {
    case DOOR_ZOMBIE_WALK1_1_DIRECTORY: case DOOR_ZOMBIE_WALK2_1_DIRECTORY:
    case DOOR_ZOMBIE_EATING_1_DIRECTORY:
        return 1;
    case DOOR_ZOMBIE_WALK1_2_DIRECTORY: case DOOR_ZOMBIE_WALK2_2_DIRECTORY:
    case DOOR_ZOMBIE_EATING_2_DIRECTORY:
        return 2;
    case DOOR_ZOMBIE_WALK1_3_DIRECTORY: case DOOR_ZOMBIE_WALK2_3_DIRECTORY:
    case DOOR_ZOMBIE_EATING_3_DIRECTORY:
        return 3;
    default:
        return 0;
    }
}

// Data note: resources/reanim zombie docs are the 2Pak HD mod assets
// (assets_src/2Pak_PvZ compiled reanim -> XML via tools/convert_2pak_zombies.py).
// 2Pak merged the retail anim_walk (44..90) and anim_walk2 (91..137) labels
// onto one closed 140-frame clip at 31..170 (idle/idle2 both 0..30,
// death/death2 both 383..459); pose analysis (tools/find_walk_loop.py) shows
// the clip is a single gait cycle whose only repeat is the frame-170 ==
// frame-31 loop seam. Both walk labels therefore cover the whole clip:
// anim_walk = anim_walk2 = 31..170 (see the header comment in
// resources/reanim/Zombie.reanim).
bool isWalk2Dir(int d) {
    switch (d) {
    case ZOMBIE_WALK2_DIRECTORY:
    case ARMLESS_ZOMBIE_WALK2_DIRECTORY:
    case FLAG_ZOMBIE_WALK2_1_DIRECTORY:
    case FLAG_ZOMBIE_WALK2_2_DIRECTORY:
    case FLAG_ZOMBIE_WALK2_3_DIRECTORY:
    case DOOR_ZOMBIE_WALK2_1_DIRECTORY:
    case DOOR_ZOMBIE_WALK2_2_DIRECTORY:
    case DOOR_ZOMBIE_WALK2_3_DIRECTORY:
        return true;
    default:
        return false;
    }
}

bool isArmlessDir(int d) {
    return d == ARMLESS_ZOMBIE_WALK1_DIRECTORY ||
           d == ARMLESS_ZOMBIE_WALK2_DIRECTORY ||
           d == ARMLESS_ZOMBIE_EATING_DIRECTORY;
}

bool isFlagDir(int d) {
    switch (d) {
    case FLAG_ZOMBIE_WALK1_1_DIRECTORY: case FLAG_ZOMBIE_WALK2_1_DIRECTORY:
    case FLAG_ZOMBIE_EATING_1_DIRECTORY: case FLAG_ZOMBIE_WALK1_2_DIRECTORY:
    case FLAG_ZOMBIE_WALK2_2_DIRECTORY: case FLAG_ZOMBIE_EATING_2_DIRECTORY:
    case FLAG_ZOMBIE_WALK1_3_DIRECTORY: case FLAG_ZOMBIE_WALK2_3_DIRECTORY:
    case FLAG_ZOMBIE_EATING_3_DIRECTORY:
        return true;
    default:
        return false;
    }
}

bool segmentRange(const char* name, int& first, int& last) {
    auto it = gDoc.segments.find(name);
    if (it == gDoc.segments.end()) return false;
    first = it->second.first;
    last = it->second.last;
    return true;
}

bool docSegmentRange(const reanim::ReanimDoc& doc, const char* name, int& first, int& last) {
    auto it = doc.segments.find(name);
    if (it == doc.segments.end()) return false;
    first = it->second.first;
    last = it->second.last;
    return true;
}

// Reference for doc-based range lookups (empty docs have no segments, so
// uninitialized skins just yield ok=false and the caller falls back to the
// legacy sheets).
const reanim::ReanimDoc& skinDocRef(int skin) {
    if (skin > SKIN_PLAIN && skin < COUNT_ZOMBIE_SKIN)
        return gSkinDoc[skin];
    return gDoc;
}

} // anonymous namespace

// Extra wrap cross-blend windows (reanim frames) for the 2Pak takes whose
// authored ends do not close the loop (closure measured by
// tools/find_walk_loop.py, as a multiple of the take's median per-frame pose
// step): no closed sub-window exists inside the take, so the segment stays
// whole and the loop wrap fades over `frames` instead of easing the gap in
// the default single frame. Keyed by (skin, first, last) of the segment.
const struct WrapBlendRule {
    int skin;
    int first, last;
    float frames;
} kWrapBlendRules[] = {
    {SKIN_BALLOON,       31, 170, 8.0f}, // anim_walk        (5.6x)
    {SKIN_DOLPHINRIDER, 173, 312, 8.0f}, // anim_walk        (3.0x)
    {SKIN_DOLPHINRIDER,  33, 172, 8.0f}, // anim_walkdolphin (2.9x)
    {SKIN_DOLPHINRIDER, 386, 457, 6.0f}, // anim_eat         (2.3x)
    {SKIN_LADDER,        30,  88, 6.0f}, // anim_walk        (2.0x)
    {SKIN_POGO,          74, 166, 6.0f}, // anim_eat         (3.2x)
    {SKIN_IMP,            0,  18, 4.0f}, // anim_eat         (1.6x)
};

float loopBlendFrames(int skin, int first, int last) {
    for (const auto& r : kWrapBlendRules)
        if (r.skin == skin && r.first == first && r.last == last)
            return r.frames;
    return 1.0f;
}

// Pace pin (retail): the walking family (plain roll: normal/cone/bucket/
// door/flag-calm/newspaper-calm/duckytube/balloon/catapult/gargantuar/imp,
// ladder bare, polevault post-vault, digger walk-back) plays at
// HEAD_WALK_FPS, and fast classes scale by pace: head_pace_fps(x) = x / 0.30
// x HEAD_WALK_FPS -- the same px/cs figure the movement constants use
// (zombie_struct.hpp), anchored on the retail plain-roll mean 0.30 px/cs,
// so the walk cycle stays foot-synced with the ground speed. Per-doc
// authored fps no longer drives zombie skins in these states.
// HEAD_WALK_FPS = the retail walk pace: the original's legacy sprite path
// runs mAnimTicksPerFrame = 12 at 100 tps = 8.3 poses/s (PickRandomSpeed,
// Zombie.cpp:1218-1222) and its live reanim path stride-locks the same way
// off the "_ground" track (UpdateAnimSpeed, Zombie.cpp:6685-6694); 8.33
// reproduces that cadence for a 12-frame walk cycle (~1.44 s per stride,
// ~43 px at the mean). The earlier 20-fps pin was tuned for the old 6/11
// NMH pace and scurried ~2.4x too fast at the restored 1.0 scale.
// anim_eat pins are retail verbatim instead of the walk anchor: repvz
// UpdateAnimSpeed chews at ApplyAnimRate(36) for most skins and 20 for
// POLEVAULT/BALLOON/IMP/DIGGER/JACK_IN_THE_BOX/SNORKEL/YETI
// (Zombie.cpp:6655-6664).
// The clone feeds reanim_time in 12-fps units (0.2 per 60 Hz tick,
// docFpsMul then maps onto each doc's own fps), so the multiplier over the
// feed is M = rate / doc.fps and the flat 0.2 feed gives exactly `rate` fps.
// One-shots (jump/gasp/armraise/hold/grab/smash/ride, ...) are deliberately
// absent: they stay tick-synced with the gameplay state machines.
// Dolphin-rider states are also absent: its walk directory phase-switches
// between segments of different lengths, and segmentLength() has no armor
// context, so rescaling its wrap window would desync the ride/jump phases;
// its feed is pace-set in the class instead (dolphinrider_zombie.cpp).
// Since the distance-driven walk feed (kWalkFeedRules + walkFeedPerPx below)
// took over the WALK states' actual rate, these walk entries only size the
// reanim_time wrap window and back up states whose doc lacks a usable
// "_ground" track.
constexpr float HEAD_WALK_FPS = 8.33f;
constexpr float EAT_FPS = 36.0f;       // repvz default chew rate
constexpr float EAT_FPS_SLOW = 20.0f;  // the seven slow-chew skins
constexpr float head_pace_fps(const float retail_px_cs)
{
    return HEAD_WALK_FPS * (retail_px_cs * HEAD_SPEED_SCALE) / 0.30f;
}
const struct PlayRateRule {
    int skin;
    const char* segment;
    float rate; // playback fps of the segment (HEAD pace)
} kPlayRateRules[] = {
    {SKIN_PLAIN,      "anim_walk",         HEAD_WALK_FPS},
    {SKIN_PLAIN,      "anim_walk2",        HEAD_WALK_FPS},
    {SKIN_PLAIN,      "anim_eat",          EAT_FPS},
    {SKIN_POLEVAULT,  "anim_walk",         HEAD_WALK_FPS},
    {SKIN_POLEVAULT,  "anim_run",          head_pace_fps(0.67f)},
    {SKIN_POLEVAULT,  "anim_eat",          EAT_FPS_SLOW},
    {SKIN_NEWSPAPER,  "anim_walk",         HEAD_WALK_FPS},
    {SKIN_NEWSPAPER,  "anim_walk_nopaper", head_pace_fps(0.90f)},
    {SKIN_NEWSPAPER,  "anim_eat",          EAT_FPS},
    {SKIN_NEWSPAPER,  "anim_eat_nopaper",  EAT_FPS},
    {SKIN_FOOTBALL,   "anim_walk",         head_pace_fps(0.67f)},
    {SKIN_FOOTBALL,   "anim_eat",          EAT_FPS},
    {SKIN_DANCER,     "anim_walk",         head_pace_fps(0.45f)},
    {SKIN_DANCER,     "anim_eat",          EAT_FPS},
    {SKIN_BACKUP,     "anim_walk",         head_pace_fps(0.45f)},
    {SKIN_BACKUP,     "anim_eat",          EAT_FPS},
    {SKIN_SNORKEL,    "anim_swim",         head_pace_fps(0.3f)},
    {SKIN_SNORKEL,    "anim_eat",          EAT_FPS_SLOW},
    {SKIN_BOBSLED,    "anim_walk",         head_pace_fps(0.60f)},
    {SKIN_BOBSLED,    "anim_eat",          EAT_FPS},
    {SKIN_JACKBOX,    "anim_walk",         head_pace_fps(0.67f)},
    {SKIN_JACKBOX,    "anim_eat",          EAT_FPS_SLOW},
    {SKIN_BALLOON,    "anim_walk",         HEAD_WALK_FPS},
    {SKIN_BALLOON,    "anim_eat",          EAT_FPS_SLOW},
    {SKIN_DIGGER,     "anim_walk",         HEAD_WALK_FPS},
    {SKIN_DIGGER,     "anim_eat",          EAT_FPS_SLOW},
    {SKIN_POGO,       "anim_pogo",         head_pace_fps(0.45f)},
    {SKIN_POGO,       "anim_eat",          EAT_FPS},
    {SKIN_LADDER,     "anim_ladderwalk",   head_pace_fps(0.80f)},
    {SKIN_LADDER,     "anim_walk",         HEAD_WALK_FPS},
    {SKIN_LADDER,     "anim_laddereat",    EAT_FPS},
    {SKIN_LADDER,     "anim_eat",          EAT_FPS},
    {SKIN_CATAPULT,   "anim_walk",         HEAD_WALK_FPS},
    {SKIN_GARGANTUAR, "anim_walk",         HEAD_WALK_FPS},
    {SKIN_IMP,        "anim_walk",         HEAD_WALK_FPS},
    {SKIN_IMP,        "anim_eat",          EAT_FPS_SLOW},
    {SKIN_YETI,       "anim_walk",         head_pace_fps(0.40f)},
    {SKIN_YETI,       "anim_eat",          EAT_FPS_SLOW},
};

// Distance-driven walk states (retail Zombie::UpdateAnimSpeed, Zombie.cpp:
// 6678-6693): everything NOT in ZombieNotWalking and not on the fixed-rate
// exception list (bobsled-with-sled, catapult, snorkel-in-pool,
// dolphin-riding) locks its anim rate to the ground speed off the doc's
// "_ground" track. The clone expresses the same lock as a per-pixel
// reanim_time feed (walkFeedPerPx below). anim_eat and one-shots are
// deliberately absent: retail plays those at fixed rates.
const struct WalkFeedRule {
    int skin;
    const char* segment;
} kWalkFeedRules[] = {
    {SKIN_PLAIN,      "anim_walk"},
    {SKIN_PLAIN,      "anim_walk2"}, // 2Pak merged both labels onto one clip
    {SKIN_POLEVAULT,  "anim_walk"},  // post-vault shamble
    {SKIN_POLEVAULT,  "anim_run"},   // pre-vault run
    {SKIN_NEWSPAPER,  "anim_walk"},
    {SKIN_NEWSPAPER,  "anim_walk_nopaper"},
    {SKIN_FOOTBALL,   "anim_walk"},
    {SKIN_DANCER,     "anim_walk"},  // disco walk-in
    {SKIN_BACKUP,     "anim_walk"},
    {SKIN_JACKBOX,    "anim_walk"},
    {SKIN_BALLOON,    "anim_walk"},  // after the pop: on foot
    {SKIN_DIGGER,     "anim_walk"},  // walking off after surfacing
    {SKIN_LADDER,     "anim_walk"},
    {SKIN_LADDER,     "anim_ladderwalk"},
    {SKIN_GARGANTUAR, "anim_walk"},
    {SKIN_IMP,        "anim_walk"},
    {SKIN_YETI,       "anim_walk"},
};

// Multiplier over the game-side reanim_time feed for this state's playback
// (render side AND the segmentLength wrap window both apply it, so the
// re-timed loop still wraps seamlessly). 1.0 = authored rate.
float playbackRateMul(const Spec& s) {
    if (!s.ok || s.segName == nullptr) return 1.0f;
    const reanim::ReanimDoc* doc = skinDoc(s.skin);
    if (doc == nullptr || doc->fps <= 0.0f) return 1.0f;
    for (const auto& r : kPlayRateRules)
        if (r.skin == s.skin && strcmp(r.segment, s.segName) == 0)
            return r.rate / doc->fps;
    return 1.0f;
}

/*repvz Zombie::ApplyAnimRate (Zombie.cpp:6676-6682): a chilled zombie plays
its reanim at half rate (the walk cycle's rate is locked to the slowed feet,
same 0.5). Applied at draw time: the game-side reanim_time feed stays
full-rate and the wrap window doubles (segmentLength's chilled flag), so the
loop wraps seamlessly through chill on/off transitions.*/
inline float chillRateMul(bool cold) { return cold ? 0.5f : 1.0f; }

namespace {

// STUB-AREA-AGENT zombie state slots -> (skin, segment). Segment names all
// exist in the 2Pak docs (verified per skin against the converted set); only
// the frame ranges differ from the retail pvz1 docs these used to point at.
struct StubState {
    int dir;
    int skin;
    const char* segment;
};
const StubState STUB_STATES[] = {
    {SNORKEL_WALK_DIRECTORY, SKIN_SNORKEL, "anim_swim"}, // water lanes: the
    // snorkel zombie swims instead of walking (grass "anim_walk" unused)
    {SNORKEL_EAT_DIRECTORY, SKIN_SNORKEL, "anim_eat"},
    {ZOMBONI_DRIVE_DIRECTORY, SKIN_ZOMBONI, "anim_drive"}, // both states
    {BOBSLED_WALK_DIRECTORY, SKIN_BOBSLED, "anim_walk"},
    {BOBSLED_EAT_DIRECTORY, SKIN_BOBSLED, "anim_eat"},
    // Dolphin rider: the walk slot's segment is phase-selected on armor_dir
    // (DOLPHIN_CARRY_STATE / DOLPHIN_INTO_POOL_STATE / DOLPHIN_RIDING_STATE /
    // DOLPHIN_JUMPING_STATE); plain = post-jump wade.
    {DOLPHINRIDER_WALK_DIRECTORY, SKIN_DOLPHINRIDER, "anim_walk"},
    {DOLPHINRIDER_EAT_DIRECTORY, SKIN_DOLPHINRIDER, "anim_eat"},
    {JACKBOX_WALK_DIRECTORY, SKIN_JACKBOX, "anim_walk"},
    {JACKBOX_EAT_DIRECTORY, SKIN_JACKBOX, "anim_eat"},
    {BALLOON_WALK_DIRECTORY, SKIN_BALLOON, "anim_walk"},
    {BALLOON_EAT_DIRECTORY, SKIN_BALLOON, "anim_eat"},
    {DIGGER_WALK_DIRECTORY, SKIN_DIGGER, "anim_walk"},
    {DIGGER_EAT_DIRECTORY, SKIN_DIGGER, "anim_eat"},
    {POGO_WALK_DIRECTORY, SKIN_POGO, "anim_pogo"},   // pogo bounce is its walk
    {POGO_EAT_DIRECTORY, SKIN_POGO, "anim_eat"},
    {BUNGEE_WALK_DIRECTORY, SKIN_BUNGEE, "anim_hold"}, // hanging above the lawn
    {BUNGEE_EAT_DIRECTORY, SKIN_BUNGEE, "anim_grab"},
    {LADDER_WALK_DIRECTORY, SKIN_LADDER, "anim_ladderwalk"}, // carrying it
    {LADDER_EAT_DIRECTORY, SKIN_LADDER, "anim_laddereat"},
    {CATAPULT_WALK_DIRECTORY, SKIN_CATAPULT, "anim_walk"},
    {CATAPULT_EAT_DIRECTORY, SKIN_CATAPULT, "anim_idle"}, // shots stub as idle
    {GARGANTUAR_WALK_DIRECTORY, SKIN_GARGANTUAR, "anim_walk"},
    {GARGANTUAR_EAT_DIRECTORY, SKIN_GARGANTUAR, "anim_smash"},
    {IMP_WALK_DIRECTORY, SKIN_IMP, "anim_walk"},
    {IMP_EAT_DIRECTORY, SKIN_IMP, "anim_eat"},
    {ZOMBOSS_WALK_DIRECTORY, SKIN_ZOMBOSS, "anim_idle"}, // boss idles in place
    {ZOMBOSS_EAT_DIRECTORY, SKIN_ZOMBOSS, "anim_idle"},
    {YETI_WALK_DIRECTORY, SKIN_YETI, "anim_walk"},
    {YETI_EAT_DIRECTORY, SKIN_YETI, "anim_eat"},
};

Spec specFromStateRaw(int directory_num, int armor_dir) {
    Spec s;
    auto pickDoc = [&](int skin, const char* seg) {
        s.segName = seg;
        return docSegmentRange(skinDocRef(skin), seg, s.first, s.last);
    };
    auto pickBase = [&](const char* seg) {
        s.segName = seg;
        return segmentRange(seg, s.first, s.last);
    };
    s.door = doorWalkStage(directory_num);
    switch (armor_dir) {
    case CONE1_DIRECTORY: s.cone = 1; break;
    case CONE2_DIRECTORY: s.cone = 2; break;
    case CONE3_DIRECTORY: s.cone = 3; break;
    case BUCKET1_DIRECTORY: s.bucket = 1; break;
    case BUCKET2_DIRECTORY: s.bucket = 2; break;
    case BUCKET3_DIRECTORY: s.bucket = 3; break;
    default: break;
    }
    s.armless = isArmlessDir(directory_num);
    s.flag = isFlagDir(directory_num);

    // Ducky tube: the pool water-lane visual of the basic zombie. Same doc,
    // the tube/water parts just come out of kBaseHidden.
    if (directory_num == DUCKYTUBE_WALK_DIRECTORY ||
        directory_num == DUCKYTUBE_EAT_DIRECTORY) {
        s.ducky = true;
        s.ok = pickBase(directory_num == DUCKYTUBE_EAT_DIRECTORY ? "anim_eat"
                                                                 : "anim_walk");
        s.tongue = (directory_num == DUCKYTUBE_EAT_DIRECTORY);
        return s;
    }

    // Pole-vaulter / newspaper states live in their own reanim docs.
    switch (directory_num) {
    case POLEVAULT_RUN_DIRECTORY:
        s.skin = SKIN_POLEVAULT;
        s.ok = pickDoc(SKIN_POLEVAULT, "anim_run");
        break;
    case POLEVAULT_VAULT_DIRECTORY:
        s.skin = SKIN_POLEVAULT;
        s.ok = pickDoc(SKIN_POLEVAULT, "anim_jump");
        break;
    case POLEVAULT_WALK_DIRECTORY:
        s.skin = SKIN_POLEVAULT;
        s.ok = pickDoc(SKIN_POLEVAULT, "anim_walk");
        break;
    case POLEVAULT_EAT_DIRECTORY:
        s.skin = SKIN_POLEVAULT;
        s.ok = pickDoc(SKIN_POLEVAULT, "anim_eat");
        break;
    case NEWSPAPER_WALK_DIRECTORY:
        s.skin = SKIN_NEWSPAPER;
        s.ok = pickDoc(SKIN_NEWSPAPER, "anim_walk");
        break;
    case NEWSPAPER_EAT_DIRECTORY:
        s.skin = SKIN_NEWSPAPER;
        s.ok = pickDoc(SKIN_NEWSPAPER, "anim_eat");
        break;
    case NEWSPAPER_GASP_DIRECTORY:
        s.skin = SKIN_NEWSPAPER;
        s.ok = pickDoc(SKIN_NEWSPAPER, "anim_gasp");
        break;
    case NEWSPAPER_WALK_NOPAPER_DIRECTORY:
        s.skin = SKIN_NEWSPAPER;
        s.ok = pickDoc(SKIN_NEWSPAPER, "anim_walk_nopaper");
        break;
    case NEWSPAPER_EAT_NOPAPER_DIRECTORY:
        s.skin = SKIN_NEWSPAPER;
        s.ok = pickDoc(SKIN_NEWSPAPER, "anim_eat_nopaper");
        break;
    case FOOTBALL_RUN_DIRECTORY:
        s.skin = SKIN_FOOTBALL;
        s.ok = pickDoc(SKIN_FOOTBALL, "anim_walk");
        break;
    case FOOTBALL_EAT_DIRECTORY:
        s.skin = SKIN_FOOTBALL;
        s.ok = pickDoc(SKIN_FOOTBALL, "anim_eat");
        break;
    case DANCER_WALK_DIRECTORY:
        s.skin = SKIN_DANCER;
        s.ok = pickDoc(SKIN_DANCER, "anim_walk");
        break;
    // Backup dancers: own retail doc (SKIN_BACKUP). The original gives them
    // the same 4 states as the lead dancer (Zombie.cpp:655 ZOMBIE_BACKUP_DANCER
    // shares the dancer reanim logic), minus the arm raise.
    case BACKUP_WALK_DIRECTORY:
        s.skin = SKIN_BACKUP;
        s.ok = pickDoc(SKIN_BACKUP, "anim_walk");
        break;
    case DANCER_RAISE_DIRECTORY:
        s.skin = SKIN_DANCER;
        s.ok = pickDoc(SKIN_DANCER, "anim_armraise");
        break;
    case DANCER_EAT_DIRECTORY:
        s.skin = SKIN_DANCER;
        s.ok = pickDoc(SKIN_DANCER, "anim_eat");
        break;
    case BACKUP_EAT_DIRECTORY:
        s.skin = SKIN_BACKUP;
        s.ok = pickDoc(SKIN_BACKUP, "anim_eat");
        break;
    default:
        break;
    }
    if (s.skin == SKIN_POLEVAULT) {
        // armor_dir is the pole-vaulter's torn-arm flag (POLE_ARMLESS_STATE)
        s.armless = (armor_dir == POLE_ARMLESS_STATE);
        return s;
    }
    if (s.skin == SKIN_NEWSPAPER) {
        // armor_dir carries the newspaper damage stage / torn-arm flag
        if (armor_dir == PAPER_DAMAGED2_STATE || armor_dir == PAPER_DAMAGED3_STATE)
            s.paperStage = armor_dir;
        s.armless = (armor_dir == PAPER_ARMLESS_STATE);
        return s;
    }
    if (s.skin == SKIN_FOOTBALL) {
        s.armless = (armor_dir == FOOT_ARMLESS_STATE);
        return s;
    }
    if (s.skin == SKIN_DANCER || s.skin == SKIN_BACKUP) {
        s.armless = (armor_dir == DANCER_ARMLESS_STATE);
        return s;
    }

    // STUB-AREA-AGENT skins (snorkel, zomboni, ... , zomboss): directory ->
    // (skin, segment) from STUB_STATES. No armor visuals yet: the stub class
    // only flashes on damage and freezes the last death frame.
    if (s.skin == SKIN_PLAIN) {
        for (const auto& st : STUB_STATES) {
            if (st.dir == directory_num) {
                s.skin = st.skin;
                const char* seg = st.segment;
                if (st.skin == SKIN_DOLPHINRIDER &&
                    directory_num == DOLPHINRIDER_WALK_DIRECTORY) {
                    // armor_dir carries the rider's phase (its class sets it)
                    if (armor_dir == DOLPHIN_RIDING_STATE) seg = "anim_ride";
                    else if (armor_dir == DOLPHIN_JUMPING_STATE) seg = "anim_dolphinjump";
                    else if (armor_dir == DOLPHIN_INTO_POOL_STATE) seg = "anim_jumpinpool";
                    else if (armor_dir == DOLPHIN_CARRY_STATE) seg = "anim_walkdolphin";
                }
                if (st.skin == SKIN_SNORKEL &&
                    directory_num == SNORKEL_WALK_DIRECTORY &&
                    armor_dir == SNORKEL_INTO_POOL_STATE)
                    seg = "anim_jumpinpool";
                s.segName = seg;
                s.ok = docSegmentRange(skinDocRef(st.skin), seg, s.first, s.last);
                return s;
            }
        }
    }

    switch (directory_num) {
    case ZOMBIE_EATING_DIRECTORY:
    case ARMLESS_ZOMBIE_EATING_DIRECTORY:
    case FLAG_ZOMBIE_EATING_1_DIRECTORY:
    case FLAG_ZOMBIE_EATING_2_DIRECTORY:
    case FLAG_ZOMBIE_EATING_3_DIRECTORY:
    case DOOR_ZOMBIE_EATING_1_DIRECTORY:
    case DOOR_ZOMBIE_EATING_2_DIRECTORY:
    case DOOR_ZOMBIE_EATING_3_DIRECTORY:
        s.ok = pickBase("anim_eat");
        s.tongue = true;
        break;
    case ZOMBIE_CREDIT1_DIRECTORY:
    case DOOR_ZOMBIE_CREDIT1_DIRECTORY:
        // 2Pak base doc: both credit states are the two halves of anim_idle
        // (0..30); the old 0..21/22..43 ranges straddled retail idle/idle2 and
        // would now spill into anim_walk (which starts at frame 31).
        s.ok = true; s.first = 0; s.last = 15;
        break;
    case ZOMBIE_CREDIT2_DIRECTORY:
    case DOOR_ZOMBIE_CREDIT2_DIRECTORY:
        s.ok = true; s.first = 16; s.last = 30;
        break;
    default:
        s.ok = pickBase(isWalk2Dir(directory_num) ? "anim_walk2" : "anim_walk");
        break;
    }
    return s;
}

// All directory states are looping clips, so trim a seam-duplicate last frame
// once here: the draw windows (drawZombie/drawSkinZombie/trackStatuses), the
// game-side reanim_time wrap (segmentLength) and effectiveLocalFrame then all
// run on the same trimmed window.
Spec specFromState(int directory_num, int armor_dir) {
    Spec s = specFromStateRaw(directory_num, armor_dir);
    if (s.ok && s.last > s.first)
        s.last = loopPlayLast(skinDocRef(s.skin), s.first, s.last);
    return s;
}

// --- Tossed-part particles ---------------------------------------------------
// Ported from the original game's particle emitter definitions found next to
// the fla exports: ZombieHead.xml (head), ZombieArm.xml (torn arm),
// ZombieHelmet.xml (cone), ZombiePail.xml (bucket), ZombieDoor.xml (screen
// door), ZombieFlag.xml (flag). The original emitters toss a baked whole-part
// image; here the part is recomposed from its reanim tracks (which already
// carry the ~0.78-0.8 particle scale in their transforms).
//
// Unit conversion, anchored once so the head arc looks right:
//   xml LaunchSpeed / 87  -> px per game tick   (330 -> 3.8, 200 -> 2.3)
//   xml AccY       / 40.5 -> px per tick^2      (17 -> 0.42)
//   xml spin deg/s / 60   -> deg per tick
// LaunchAngle convention: math degrees, CCW from +x; 150-185 = up-left .. left.
struct PopPhys {
    float speed;
    float angMin, angMax;
    float grav;
    float restitution;
    float driftX;         // constant extra px/tick (the flag emitter's Velocity field)
    float spinMin, spinMax; // deg/tick launch-spin range (matches the xml sign)
    int lingerTicks;      // part lifetime; 0 = tied to the corpse (death part)
    float fadeFrom;       // life fraction where the fade starts
    float startRot = 0.0f; // initial tilt (xml ParticleSpinAngle; flag = 30)
};

struct PopPose {
    float dx, dy, rotDeg;
};

PopPose simulatePop(int ticks, float floorDy, unsigned rng, const PopPhys& ph) {
    const float D2R = 3.14159265358979f / 180.0f;
    float rAng = ((rng >> 8) & 0xFF) / 255.0f;
    float rSpin = ((rng >> 16) & 0xFF) / 255.0f;
    float a = (ph.angMin + (ph.angMax - ph.angMin) * rAng) * D2R;
    float vx = ph.speed * cosf(a) + ph.driftX;
    float vy = -ph.speed * sinf(a); // screen y is down
    float spin = -(ph.spinMin + (ph.spinMax - ph.spinMin) * rSpin);

    PopPose p{0.0f, 0.0f, 0.0f};
    float x = 0.0f, y = 0.0f, rot = ph.startRot;
    bool resting = false;
    for (int i = 0; i < ticks; i++) {
        if (!resting) vy += ph.grav;
        x += vx;
        y += vy;
        rot += spin;
        if (y >= floorDy && vy > 0.0f) {
            y = floorDy;
            vy = -vy * ph.restitution;
            vx *= 0.55f;
            spin = -spin * 0.25f; // CollisionSpin: reduced tumble after bounce
            if (fabsf(vy) < 0.6f) {
                vy = 0.0f;
                resting = true;
            }
        }
        if (resting) { // rolling friction
            vx *= 0.90f;
            spin *= 0.90f;
        }
    }
    p.dx = x;
    p.dy = y;
    p.rotDeg = rot;
    return p;
}

// Tracks forming the severed head on death (the original engine spawns these
// as a separate tossed part; the death segment itself leaves them hidden).
static const char* kHeadTracks[] = {"anim_head1", "anim_head2", "anim_hair"};
// The torn-off outer arm. The 2Pak base doc bakes the outer-arm hand variants
// as extra tracks all named "zombie_outerarm_lower" (HAND/HAND2/HAND3 images);
// the visible HAND duplicate is pulled in by that name (dormant variants sit
// at f=-1), so there is no separate "zombie_outerarm_hand" track to list.
static const char* kArmTracks[] = {"zombie_outerarm_upper", "zombie_outerarm_lower"};

struct DropSpec {
    int docSkin;                        // 0 = gDoc, SKIN_POLEVAULT = pole doc, SKIN_NEWSPAPER = paper doc
    bool useFlagDoc;                    // draw from gFlagDoc instead
    const char* const* tracks;
    int nTracks;
    const char* imageOverrideStem;      // single-track parts: stage-3 damaged image
    float cx, cy;                       // standing center of the part, reanim units
    float halfH;                        // half image height, reanim units
    PopPhys phys;
};

const DropSpec& dropSpecFor(int drop_img, int skin) {
    static const char* coneTrack[] = {"anim_cone"};
    static const char* bucketTrack[] = {"anim_bucket"};
    static const char* doorTrack[] = {"anim_screendoor"};
    static const char* flagTrack[] = {"zombie_flag"};
    // Extra skins' severed parts (same physics as the matched plain-zombie
    // part): pole-vaulter head/arm from Zombie_polevaulter.reanim, newspaper
    // head/arm from Zombie_paper.reanim. cx/cy = the recomposed part's frame-0
    // union-bbox center in the 2Pak doc (drawPop draws doc frame 0, so the
    // pivot sits at the part's own center); halfH = standing half-height
    // for heads, the lying half-extent for arms so they rest on the ground.
    static const char* poleHeadTracks[] = {"anim_head1", "anim_head2", "anim_hair"};
    static const char* poleArmTracks[] = {"zombie_polevaulter_outerarm_upper",
                                          "zombie_polevaulter_outerarm_lower",
                                          "zombie_outerarm_hand"};
    static const char* paperHeadTracks[] = {"anim_head1", "anim_head_glasses",
                                            "anim_hairpiece", "anim_head_jaw"};
    static const char* paperArmTracks[] = {"zombie_paper_leftarm_upper",
                                           "zombie_paper_leftarm_lower",
                                           "zombie_paper_hands"};
    static const char* footHeadTracks[] = {"anim_head1", "anim_head2"};
    static const char* footArmTracks[] = {"zombie_football_leftarm_upper",
                                          "zombie_football_leftarm_lower",
                                          "zombie_football_leftarm_hand"};
    static const char* danceHeadTracks[] = {"anim_head1", "anim_head2", "anim_hair"};
    // Disco (jackson) doc 2Pak naming: the intact outer upper arm is the
    // Jackson track (the disco "_outerarm_upper_bone" track is the torn stump,
    // f-shown only during anim_death, so the code never toggles it).
    static const char* danceArmTracks[] = {"zombie_jackson_outerarm_upper",
                                           "zombie_outerarm_lower",
                                           "zombie_outerarm_hand"};
    // Backup dancer doc (disco track names, backup images). Its head pop
    // tosses the full head group: crown, jaw and stash (the GOTY particle
    // uses ZombieBackupDancerHead.png, Zombie.cpp:3567-3573).
    static const char* backupHeadTracks[] = {"anim_head1", "anim_head2",
                                             "zombie_backup_stash"};
    static const char* backupArmTracks[] = {"zombie_disco_outerarm_upper",
                                            "zombie_disco_outerarm_lower",
                                            "zombie_disco_outerhand"};
    // HEAD: xml ParticleSpinSpeed is the SYMMETRIC range [-720 720] deg/s =
    // ±12 deg/tick; spin = -(spinMin + (spinMax-spinMin)*r) then yields the
    // full signed range (both tumble directions, uniformly).
    // cx/cy/halfH below were re-measured on the 2Pak docs (frame-0 union bbox
    // of the part tracks, tools/measure_zombie_drop.py) as delta vs. the old
    // retail calibrations, except the backup dancer (retail doc, unchanged)
    // and FLAG (2Pak flag doc frame 0 doesn't reproduce the retail pivot at
    // all -- the pop pivots were hand-tuned, kept as-is).
    static const DropSpec HEAD = {SKIN_PLAIN, false, kHeadTracks, 3, nullptr,
                                  28.5f, 29.0f, 23.5f,
                                  {3.8f, 150.0f, 185.0f, 0.42f, 0.3f, 0.0f,
                                   -12.0f, 12.0f, 0, 0.0f}};
    static const DropSpec POLE_HEAD = {SKIN_POLEVAULT, false, poleHeadTracks, 3, nullptr,
                                       46.5f, 46.0f, 24.5f,
                                       {3.8f, 150.0f, 185.0f, 0.42f, 0.3f, 0.0f,
                                        -12.0f, 12.0f, 0, 0.0f}};
    static const DropSpec PAPER_HEAD = {SKIN_NEWSPAPER, false, paperHeadTracks, 4, nullptr,
                                        39.0f, 32.0f, 25.5f,
                                        {3.8f, 150.0f, 185.0f, 0.42f, 0.3f, 0.0f,
                                         -12.0f, 12.0f, 0, 0.0f}};
    static const DropSpec FOOT_HEAD = {SKIN_FOOTBALL, false, footHeadTracks, 2, nullptr,
                                       56.5f, 38.0f, 23.5f,
                                       {3.8f, 150.0f, 185.0f, 0.42f, 0.3f, 0.0f,
                                        -12.0f, 12.0f, 0, 0.0f}};
    // DANCE_* were re-measured on the disco (2Pak zombie_jackson) doc after it
    // replaced the bare-backup-body dancer doc: frame-0 union bbox of the part
    // tracks, tools/measure_zombie_drop.py -> head (27.0, 20.2, 32.7), arm
    // (20.1, 63.4) with the shared lying half-extent.
    static const DropSpec DANCE_HEAD = {SKIN_DANCER, false, danceHeadTracks, 3, nullptr,
                                        27.0f, 20.0f, 32.5f,
                                        {3.8f, 150.0f, 185.0f, 0.42f, 0.3f, 0.0f,
                                         -12.0f, 12.0f, 0, 0.0f}};
    // ARM: LaunchSpeed 40 and AccY 15 per the xml -- the toss is *meant* to be
    // a weak up-left pop (unlike the flying head at 330/[150 185]). cx/cy are
    // the true center of the recomposed 3-track arm (the rotation pivot and
    // the reference for the fall distance); halfH is the LYING half-extent
    // (the part tumbles flat), so the arm rests on the ground instead of
    // floating a standing-height above it. The arm xml gives a POSITIVE spin
    // range ([0 360] where helmets/door/flag are negative), so its tumble
    // runs opposite to the other parts: negative spinMin/spinMax here.
    static const DropSpec ARM = {SKIN_PLAIN, false, kArmTracks, 2, nullptr,
                                 45.0f, 72.5f, 8.0f,
                                 {0.46f, 90.0f, 185.0f, 0.37f, 0.5f, 0.0f,
                                  -6.0f, -0.5f, 60, 0.9f}};
    static const DropSpec POLE_ARM = {SKIN_POLEVAULT, false, poleArmTracks, 3, nullptr,
                                      66.5f, 91.0f, 8.0f,
                                      {0.46f, 90.0f, 185.0f, 0.37f, 0.5f, 0.0f,
                                       -6.0f, -0.5f, 60, 0.9f}};
    static const DropSpec PAPER_ARM = {SKIN_NEWSPAPER, false, paperArmTracks, 3, nullptr,
                                       61.0f, 71.5f, 8.0f,
                                       {0.46f, 90.0f, 185.0f, 0.37f, 0.5f, 0.0f,
                                        -6.0f, -0.5f, 60, 0.9f}};
    static const DropSpec FOOT_ARM = {SKIN_FOOTBALL, false, footArmTracks, 3, nullptr,
                                      87.0f, 71.5f, 8.0f,
                                      {0.46f, 90.0f, 185.0f, 0.37f, 0.5f, 0.0f,
                                       -6.0f, -0.5f, 60, 0.9f}};
    static const DropSpec DANCE_ARM = {SKIN_DANCER, false, danceArmTracks, 3, nullptr,
                                       20.0f, 63.5f, 8.0f,
                                       {0.46f, 90.0f, 185.0f, 0.37f, 0.5f, 0.0f,
                                        -6.0f, -0.5f, 60, 0.9f}};
    // Backup dancer: own retail doc, whose calibration predates the 2Pak swap
    // and stays valid once its part images are the retail set again (the
    // disco-skeleton note no longer applies: SKIN_DANCER runs a different doc).
    static const DropSpec BACKUP_HEAD = {SKIN_BACKUP, false, backupHeadTracks, 3, nullptr,
                                         74.0f, 14.0f, 35.0f,
                                         {3.8f, 150.0f, 185.0f, 0.42f, 0.3f, 0.0f,
                                          -12.0f, 12.0f, 0, 0.0f}};
    static const DropSpec BACKUP_ARM = {SKIN_BACKUP, false, backupArmTracks, 3, nullptr,
                                        59.0f, 28.5f, 8.0f,
                                        {0.46f, 90.0f, 185.0f, 0.37f, 0.5f, 0.0f,
                                         -6.0f, -0.5f, 60, 0.9f}};
    // CONE (ZombieTrafficCone.xml): LaunchAngle is [100 140] (not [120 180] --
    // that range belongs to the pail/door).
    static const DropSpec CONE = {SKIN_PLAIN, false, coneTrack, 1, "zombie_cone3",
                                  33.0f, -1.0f, 31.0f,
                                  {2.3f, 100.0f, 140.0f, 0.42f, 0.3f, 0.0f,
                                   3.3f, 6.6f, 50, 0.8f}};
    static const DropSpec BUCKET = {SKIN_PLAIN, false, bucketTrack, 1, "zombie_bucket3",
                                    29.5f, 12.0f, 29.0f,
                                    {2.3f, 120.0f, 180.0f, 0.42f, 0.3f, 0.0f,
                                     3.3f, 6.6f, 50, 0.8f}};
    static const DropSpec DOOR = {SKIN_PLAIN, false, doorTrack, 1, "zombie_screendoor3",
                                  21.0f, 67.0f, 54.0f,
                                  {2.3f, 120.0f, 180.0f, 0.42f, 0.3f, 0.0f,
                                   3.3f, 6.6f, 40, 0.6f}};
    // FLAG: xml adds a Velocity X=100 field (constant rightward drift 1.15
    // px/tick) and ParticleSpinAngle=30 (startRot). fadeFrom 0.8 mirrors
    // SystemAlpha "1,80 0,100". The spin-ramp "[-200 -600] -> [-100 -300]"
    // (spin slows over life) is not simulated; the launch range is used.
    static const DropSpec FLAG = {SKIN_PLAIN, true, flagTrack, 1, "zombie_flag3",
                                  -7.0f, 19.0f, 27.0f,
                                  {2.3f, 120.0f, 160.0f, 0.42f, 0.3f, 1.15f,
                                   3.3f, 10.0f, 50, 0.8f, 30.0f}};
    if (skin == SKIN_POLEVAULT) {
        switch (drop_img) {
        case DROP_ARM_IMG: return POLE_ARM;
        default: return POLE_HEAD;
        }
    }
    if (skin == SKIN_NEWSPAPER) {
        switch (drop_img) {
        case DROP_ARM_IMG: return PAPER_ARM;
        default: return PAPER_HEAD;
        }
    }
    if (skin == SKIN_FOOTBALL) {
        switch (drop_img) {
        case DROP_ARM_IMG: return FOOT_ARM;
        default: return FOOT_HEAD;
        }
    }
    if (skin == SKIN_DANCER) {
        switch (drop_img) {
        case DROP_ARM_IMG: return DANCE_ARM;
        default: return DANCE_HEAD;
        }
    }
    if (skin == SKIN_BACKUP) {
        switch (drop_img) {
        case DROP_ARM_IMG: return BACKUP_ARM;
        default: return BACKUP_HEAD;
        }
    }
    switch (drop_img) {
    case DROP_ARM_IMG: return ARM;
    case DROP_CONE_IMG: return CONE;
    case DROP_BUCKET_IMG: return BUCKET;
    case DROP_DOOR_IMG: return DOOR;
    case DROP_FLAG_IMG: return FLAG;
    default: return HEAD;
    }
}

// Draw one tossed part. (ox, oy) = reanim origin of the zombie box in screen
// px; `scale` = the body's effective draw scale (the zombieDrawGeom output:
// skin scale x the big-trouble/editor mul), so the part matches the corpse it
// dropped from (a mini zombie's head pops at mini size). fade multiplies
// alpha (1 = opaque). rng seeds the launch randomness.
void drawPop(const DropSpec& spec, float aging_time, float ox, float oy,
             float scale, bool cold, float fade, unsigned rng) {
    const reanim::ReanimDoc* doc = spec.useFlagDoc ? (gFlagReady ? &gFlagDoc : nullptr)
                                                   : skinDoc(spec.docSkin);
    if (doc == nullptr) return;
    // geo only supplies the doc-space floor line for the bounce plane; every
    // px conversion below runs on the caller's effective scale.
    const SkinGeom geo = geomFor(spec.docSkin);
    float spawnX = ox + scale * spec.cx;
    float spawnY = oy + scale * spec.cy;
    // Bounce plane = visible feet line (floorY is the padded quad bottom).
    float floorDy = (oy + scale * geo.floorY - ART_PAD_BOTTOM) - scale * spec.halfH - spawnY;
    if (floorDy < 0) floorDy = 0;

    PopPose pop = simulatePop((int)(aging_time * 5.0f), floorDy, rng, spec.phys);
    float cx = spawnX + pop.dx, cy = spawnY + pop.dy;
    float popOX = cx - scale * spec.cx, popOY = cy - scale * spec.cy;

    std::unordered_map<std::string, std::string> overrideMap;
    std::string firstTrack = lowerOf(spec.tracks[0]);
    if (spec.imageOverrideStem) overrideMap[firstTrack] = spec.imageOverrideStem;

    reanim::DrawOptions opt;
    opt.tint = cold ? SDL_Color{110, 170, 255, 255} : SDL_Color{255, 255, 255, 255};
    opt.alphaMul = fade;
    opt.rotateDeg = pop.rotDeg;
    opt.rotatePivot = SDL_FPoint{cx, cy};
    if (!overrideMap.empty()) opt.imageOverrides = &overrideMap;
    gRenderer->Draw(*doc, 0.0f, popOX, popOY, scale,
                    [&](const std::string& n) {
                        std::string l = lowerOf(n);
                        for (int i = 0; i < spec.nTracks; i++)
                            if (l == spec.tracks[i]) return false; // keep part tracks
                        return true;                               // hide all else
                    },
                    opt);
}

// Load an extra self-contained character skin (pole vaulter / newspaper) and
// calibrate its placement exactly like the base zombie: the skin's on-screen
// skin_scale (ZOMBIE_SCALE for skins without a §8.1 override), bbox center
// anchored at MASS_CENTER_X, feet line at ART_BOTTOM_Y. The feet
// line is the quad bottom of the foot/toe tracks -- not the union bottom --
// so art that dips below the feet (planted pole tip in the run pose, a
// swinging paper) can't push the zombie off the floor.
bool initSkin(int skin, const char* file, reanim::ReanimDoc& doc, SkinGeom& geo,
              const char* walkSeg, const char* fallbackSeg,
              const std::unordered_set<std::string>& hide) {
    std::string err;
    if (!reanim::LoadReanimXml(std::string(REANIM_DIR) + "/" + file, doc, err)) {
        SDL_Log("reanimz skin: %s", err.c_str());
        return false;
    }
    int first, last;
    if (!docSegmentRange(doc, walkSeg, first, last)) {
        if (fallbackSeg == nullptr ||
            !docSegmentRange(doc, fallbackSeg, first, last)) {
            SDL_Log("reanimz skin: %s has no %s segment", file, walkSeg);
            return false;
        }
        walkSeg = fallbackSeg;
    }
    auto measurePass = [&](const std::function<bool(const std::string&)>& hideFn) {
        reanim::DrawStats acc;
        reanim::DrawOptions measure;
        measure.measureOnly = true;
        for (int fr = first; fr <= last; fr += 4) {
            reanim::DrawStats st = gRenderer->Draw(doc, (float)fr, 0, 0, 1.0f,
                                                   hideFn, measure);
            if (st.hasBounds) {
                if (!acc.hasBounds) {
                    acc = st;
                } else {
                    acc.minX = std::min(acc.minX, st.minX);
                    acc.minY = std::min(acc.minY, st.minY);
                    acc.maxX = std::max(acc.maxX, st.maxX);
                    acc.maxY = std::max(acc.maxY, st.maxY);
                }
            }
        }
        return acc;
    };
    reanim::DrawStats acc = measurePass(
        [&](const std::string& n) { return hide.count(lowerOf(n)) > 0; });
    if (!acc.hasBounds) {
        SDL_Log("reanimz skin: %s walk segment has no visible parts", file);
        return false;
    }
    reanim::DrawStats feet = measurePass([&](const std::string& n) {
        std::string l = lowerOf(n);
        return hide.count(l) > 0 ||
               (l.find("foot") == std::string::npos && l.find("toe") == std::string::npos);
    });
    float floorU = feet.hasBounds ? feet.maxY : acc.maxY;
    geo.scale = skin_scale(skin);
    float imgOffX, imgOffY;
    skinImageOffset(skin, imgOffX, imgOffY);
    geo.anchorDX = MASS_CENTER_X + imgOffX - geo.scale * (acc.minX + acc.maxX) * 0.5f;
    geo.anchorDY = ART_BOTTOM_Y + ART_PAD_BOTTOM + imgOffY - geo.scale * floorU;
    geo.floorY = floorU;
    SDL_Log("reanimz skin: %s ready; walk bbox %.0fx%.0f, scale %.3f, anchor (%.1f, %.1f)",
            file, acc.maxX - acc.minX, acc.maxY - acc.minY, geo.scale,
            geo.anchorDX, geo.anchorDY);
    return true;
}

// Generic draw for the extra skins (pole vaulter, newspaper): the authored
// data already drives per-segment part visibility (pole held/planted, paper
// torn away mid-gasp, mad head), so the code only models state-machine
// deltas: torn arm and the paper's damage-stage image swap.
bool drawSkinZombie(const Spec& s, float reanim_time, int x_location, int y_location,
                    bool cold, bool attacked,
                    const std::unordered_set<std::string>* extraHide,
                    const std::unordered_set<std::string>* extraShow,
                    int* partsDrawn, float attack_fade) {
    const reanim::ReanimDoc* doc = skinDoc(s.skin);
    if (doc == nullptr) return false; // assets missing -> sprite-sheet fallback
    const SkinGeom geo = geomFor(s.skin);

    float len = (float)(s.last - s.first + 1);
    float local = timeScaledLocal(reanim_time * playbackRateMul(s) *
                                      chillRateMul(cold) * docFpsMul(*doc),
                                  len);
    float absFrame = (float)s.first + local;

    std::unordered_set<std::string> hide, forceShow;
    std::unordered_map<std::string, std::string> overrides;
    if (s.skin == SKIN_POLEVAULT && s.armless) {
        hide.insert("zombie_polevaulter_outerarm_lower");
        hide.insert("zombie_outerarm_hand");
        overrides["zombie_polevaulter_outerarm_upper"] = "zombie_polevaulter_outerarm_upper2";
    }
    if (s.skin == SKIN_NEWSPAPER) {
        if (s.paperStage == 2) overrides["zombie_paper_paper"] = "zombie_paper_paper2";
        if (s.paperStage == 3) overrides["zombie_paper_paper"] = "zombie_paper_paper3";
        if (s.armless) {
            hide.insert("zombie_paper_leftarm_lower");
            hide.insert("zombie_paper_hands");
            overrides["zombie_paper_leftarm_upper"] = "zombie_paper_leftarm_upper2";
        }
    }
    if (s.skin == SKIN_FOOTBALL && s.armless) {
        // no torn upper-arm variant in the retail pvz1 set either: keep the
        // shoulder flesh and drop the lower arm + hand
        hide.insert("zombie_football_leftarm_lower");
        hide.insert("zombie_football_leftarm_hand");
    }
    if (s.skin == SKIN_DANCER && s.armless) {
        hide.insert("zombie_outerarm_lower");
        hide.insert("zombie_outerarm_hand");
    }
    if (s.skin == SKIN_BACKUP && s.armless) {
        // Backup doc reuses the disco track names (retail fla export); the
        // original hides these two when the arm drops (Zombie.cpp:3694-3698).
        hide.insert("zombie_disco_outerarm_lower");
        hide.insert("zombie_disco_outerhand");
    }
    if (extraHide) {
        for (const auto& t : *extraHide) hide.insert(lowerOf(t));
    }
    if (extraShow) {
        for (const auto& t : *extraShow) {
            hide.erase(lowerOf(t));
            forceShow.insert(lowerOf(t));
        }
    }

    reanim::DrawOptions opt;
    opt.tint = cold ? SDL_Color{110, 170, 255, 255} : SDL_Color{255, 255, 255, 255};
    opt.flash = attacked;
    // fade-out over the blink budget (same ramp as drawZombie's plain path)
    opt.flashAlpha = (Uint8)std::clamp(lroundf(opt.flashAlpha * attack_fade), 0l, 255l);
    opt.loopWrapFirst = s.first;
    opt.loopWrapLast = s.last;
    opt.loopBlendFrames = loopBlendFrames(s.skin, s.first, s.last);
    if (!overrides.empty()) opt.imageOverrides = &overrides;
    if (!forceShow.empty()) opt.forceShow = &forceShow;

    float scale, ox, oy;
    zombieDrawGeom(geo, (float)x_location, (float)y_location, scale, ox, oy);
    auto hideTrack = [&](const std::string& n) { return hide.count(lowerOf(n)) > 0; };
    reanim::DrawStats stats = gRenderer->Draw(*doc, absFrame, ox, oy, scale, hideTrack, opt);
    if (partsDrawn) *partsDrawn = stats.partsDrawn;
    return true;
}

// --- Charred (explosion/burn) corpses ---------------------------------------
// repvz Zombie::ApplyBurn (Zombie.cpp:8756-8905): the burn kill despawns the
// zombie immediately and spawns a SEPARATE charred reanimation that plays
// once (default REANIM_PLAY_ONCE, Reanimation::Reanimation mLoopType,
// Reanimator.cpp:289-294) and dies at the end of its timeline. Six docs (the
// set is the same in 2Pak, now loaded from resources/reanim): the generic
// body for every common zombie and dedicated art for imp / digger (2
// variants) / zamboni / catapult / gargantuar.
struct CharredDef {
    const char* file;
    const char* seg;      // play-once segment (nullptr: whole timeline)
    const char* calibSeg; // calibration reference (nullptr: play segment)
};

const CharredDef CHARRED_DEFS[] = {
    // 2Pak charred docs carry no anim_blink (calibration folds back to the
    // play segment); their fps also changed (12, digger 15) vs retail (15).
    /* CHARRED_GENERIC    */ {"Zombie_charred.reanim", nullptr, nullptr},
    /* CHARRED_IMP        */ {"Zombie_charred_imp.reanim", nullptr, nullptr},
    /* CHARRED_DIGGER     */ {"Zombie_charred_digger.reanim", "anim_crumble", nullptr},
    /* CHARRED_ZAMBONI    */ {"Zombie_charred_zamboni.reanim", nullptr, nullptr},
    /* CHARRED_CATAPULT   */ {"Zombie_charred_catapult.reanim", nullptr, nullptr},
    /* CHARRED_GARGANTUAR */ {"Zombie_charred_gargantuar.reanim", nullptr, nullptr},
    /* CHARRED_DIGGER_NOAXE*/{"Zombie_charred_digger.reanim", "anim_crumble_noaxe", nullptr},
};
const int COUNT_CHARRED = (int)(sizeof(CHARRED_DEFS) / sizeof(CHARRED_DEFS[0]));

reanim::ReanimDoc gCharredDoc[COUNT_CHARRED];
bool gCharredReady[COUNT_CHARRED] = {};
bool gCharredTried[COUNT_CHARRED] = {};
int gCharredFirst[COUNT_CHARRED] = {}, gCharredLast[COUNT_CHARRED] = {};
SkinGeom gCharredGeom[COUNT_CHARRED];

// A burnt zombie's ash pile must swap in at the same on-screen size the living
// zombie stood at (retail draws both at native retail resolution,
// Zombie.cpp:8824-8863), so a heavy type's charred doc takes its skin's
// per-skin scale; the generic/imp corpses keep the shared ZOMBIE_SCALE.
int charredSkin(int kind) {
    switch (kind) {
    case CHARRED_DIGGER:
    case CHARRED_DIGGER_NOAXE: return SKIN_DIGGER;
    case CHARRED_ZAMBONI:      return SKIN_ZOMBONI;
    case CHARRED_CATAPULT:     return SKIN_CATAPULT;
    case CHARRED_GARGANTUAR:   return SKIN_GARGANTUAR;
    default:                   return SKIN_PLAIN;
    }
}

// Charred docs carry no foot/toe-named parts (body/pile/tail), so the floor
// line is the union bottom (the ash pile sits on the ground).
bool initCharred(int kind) {
    if (gCharredTried[kind]) return gCharredReady[kind];
    gCharredTried[kind] = true;
    if (gRenderer == nullptr) return false;
    const CharredDef& def = CHARRED_DEFS[kind];
    reanim::ReanimDoc& doc = gCharredDoc[kind];
    std::string err;
    if (!reanim::LoadReanimXml(std::string(REANIM_DIR) + "/" + def.file, doc, err)) {
        SDL_Log("reanimz charred: %s", err.c_str());
        return false;
    }
    int first = 0, last = doc.frameCount - 1;
    if (def.seg != nullptr && !docSegmentRange(doc, def.seg, first, last)) {
        SDL_Log("reanimz charred: %s has no %s segment", def.file, def.seg);
        return false;
    }
    gCharredFirst[kind] = first;
    gCharredLast[kind] = last;
    int cf = first, cl = last;
    if (def.calibSeg != nullptr)
        docSegmentRange(doc, def.calibSeg, cf, cl);
    // Measure the (standing) calibration frames and anchor the doc like every
    // other zombie art: bbox mass center at MASS_CENTER_X, floor at
    // ART_BOTTOM_Y.
    reanim::DrawStats acc;
    reanim::DrawOptions measure;
    measure.measureOnly = true;
    for (int fr = cf; fr <= cl; fr += 2) {
        reanim::DrawStats st = gRenderer->Draw(doc, (float)fr, 0, 0, 1.0f,
                                               nullptr, measure);
        if (st.hasBounds) {
            if (!acc.hasBounds)
                acc = st;
            else {
                acc.minX = std::min(acc.minX, st.minX);
                acc.minY = std::min(acc.minY, st.minY);
                acc.maxX = std::max(acc.maxX, st.maxX);
                acc.maxY = std::max(acc.maxY, st.maxY);
            }
        }
    }
    if (!acc.hasBounds) {
        SDL_Log("reanimz charred: %s has no visible parts", def.file);
        return false;
    }
    SkinGeom& geo = gCharredGeom[kind];
    geo.scale = skin_scale(charredSkin(kind));
    geo.anchorDX = MASS_CENTER_X - geo.scale * (acc.minX + acc.maxX) * 0.5f;
    geo.anchorDY = ART_BOTTOM_Y + ART_PAD_BOTTOM - geo.scale * acc.maxY;
    geo.floorY = acc.maxY;
    SDL_Log("reanimz charred: %s ready; calib bbox %.0fx%.0f, anchor (%.1f, %.1f)",
            def.file, acc.maxX - acc.minX, acc.maxY - acc.minY, geo.anchorDX, geo.anchorDY);
    gCharredReady[kind] = true;
    return true;
}

} // namespace

bool init(ISpriteRenderer& renderer) {
    if (gInitTried) return gReady;
    gInitTried = true;
    std::string err;
    if (!reanim::LoadReanimXml(std::string(REANIM_DIR) + "/" + REANIM_FILE, gDoc, err)) {
        SDL_Log("reanimz: %s (falling back to sprite sheets)", err.c_str());
        return gReady;
    }
    static reanim::SdlReanimRenderer sRenderer(renderer);
    gRenderer = &sRenderer;
    if (!gRenderer->BindDirectory(REANIM_DIR)) {
        SDL_Log("reanimz: no part images in %s (falling back to sprite sheets)", REANIM_DIR);
        return gReady;
    }
    if (reanim::LoadReanimXml(std::string(REANIM_DIR) + "/" + FLAG_FILE, gFlagDoc, err)) {
        gFlagReady = true;
        gFlagLoopLen = (float)(loopPlayLast(gFlagDoc, 0, gFlagDoc.frameCount - 1) + 1);
    } else
        SDL_Log("reanimz: %s (flag zombie loses its flag)", err.c_str());

    // Calibration: measure the walk-segment bbox (in reanim units) with an
    // invisible measure-only pass, then anchor it into the game's 166x144 box
    // at the skin's scale (PLAIN has no §8.1 override: the sheet-proven 1.186).
    std::unordered_set<std::string> hide;
    for (const char* h : kBaseHidden) hide.insert(h);

    int first, last;
    if (!segmentRange("anim_walk", first, last)) return gReady;
    reanim::DrawStats acc;
    reanim::DrawOptions measure;
    measure.measureOnly = true;
    for (int fr = first; fr <= last; fr += 4) {
        reanim::DrawStats st = gRenderer->Draw(
            gDoc, (float)fr, 0, 0, 1.0f,
            [&](const std::string& n) { return hide.count(lowerOf(n)) > 0; }, measure);
        if (st.hasBounds) {
            if (!acc.hasBounds) {
                acc = st;
            } else {
                acc.minX = std::min(acc.minX, st.minX);
                acc.minY = std::min(acc.minY, st.minY);
                acc.maxX = std::max(acc.maxX, st.maxX);
                acc.maxY = std::max(acc.maxY, st.maxY);
            }
        }
    }
    if (!acc.hasBounds) return gReady;
    gScale = skin_scale(SKIN_PLAIN);
    gAnchorDX = MASS_CENTER_X - gScale * (acc.minX + acc.maxX) * 0.5f;
    gAnchorDY = ART_BOTTOM_Y + ART_PAD_BOTTOM - gScale * acc.maxY;
    gFloorY = acc.maxY;
    SDL_Log("reanimz: ready; walk bbox %.0fx%.0f, scale %.3f, anchor (%.1f, %.1f)",
            acc.maxX - acc.minX, acc.maxY - acc.minY, gScale, gAnchorDX, gAnchorDY);
    gReady = true;

    // Extra character skins; failures only disable that skin's reanim path.
    std::unordered_set<std::string> noHide;
    for (int skin = 1; skin < COUNT_ZOMBIE_SKIN; skin++)
    {
        const ExtraSkinDef& def = EXTRA_SKINS[skin];
        if (def.file == nullptr) continue;
        gSkinReady[skin] = initSkin(skin, def.file, gSkinDoc[skin], gSkinGeom[skin],
                                    def.walkSeg, def.fallbackSeg, noHide);
    }
    return gReady;
}

bool ready() { return gReady; }

void setZombieScaleMul(float k) { gZombieScaleMul = (k > 0.0f ? k : 1.0f); }

void setZombieTimeMul(float k) { gZombieTimeMul = (k > 0.0f ? k : 1.0f); }

float zombieTimeMul() { return gZombieTimeMul; }

float effectiveLocalFrame(int directory_num, float reanim_time, bool chilled) {
    if (!gReady) return 0.0f;
    Spec s = specFromState(directory_num, -1);
    if (!s.ok) return 0.0f;
    float len = (float)(s.last - s.first + 1);
    if (len <= 0.0f) return 0.0f;
    return timeScaledLocal(reanim_time * playbackRateMul(s) * chillRateMul(chilled), len);
}

// On-screen head anchor for the kernel-pult butter pat (repvz
// Zombie::DrawButter, Zombie.cpp:6274-6321, which pins the splat to the
// zombie's anim_head1 track): head CX/top in screen px of the head image at
// the zombie's currently drawn frame, through the same geometry chain the
// body draw uses (per-skin scale x the big-trouble/editor mul), plus that
// mul itself for sizing the pat. False when the reanim path isn't driving
// this zombie (caller keeps the legacy box-top offset).
bool headAnchor(int directory_num, int armor_dir, float reanim_time,
                bool cold, int x_location, int y_location,
                float& headCX, float& headTopY, float& artMul) {
    if (!gReady) return false;
    Spec s = specFromState(directory_num, armor_dir);
    if (!s.ok) return false;
    const reanim::ReanimDoc* doc = skinDoc(s.skin);
    if (doc == nullptr) return false;
    float len = (float)(s.last - s.first + 1);
    if (len <= 0.0f) return false;
    // drawZombie / drawSkinZombie's frame math, duplicated so the anchor sits
    // on the exact pose on screen (skin docs add their authored-fps mul).
    float feed = reanim_time * playbackRateMul(s) * chillRateMul(cold);
    if (s.skin != SKIN_PLAIN) feed *= docFpsMul(*doc);
    float absFrame = (float)s.first + timeScaledLocal(feed, len);
    // The head part is anim_head1 on most docs; the catapult only authors the
    // driver's head (repvz DrawButter keys Zombie_catapult_driver_head there,
    // Zombie.cpp:6285-6287).
    static const char* kHeadTracks[] = {"anim_head1", "zombie_catapult_driver_head"};
    const reanim::Track* head = nullptr;
    for (const char* want : kHeadTracks) {
        for (const reanim::Track& tr : doc->tracks)
            if (lowerOf(tr.name) == want) { head = &tr; break; }
        if (head) break;
    }
    if (head == nullptr || !head->hasAnyImage) return false;
    reanim::Transform t = reanim::SampleTrack(*head, absFrame, s.first, s.last,
                                              loopBlendFrames(s.skin, s.first, s.last));
    if (t.f < -0.5f || t.image < 0 || t.image >= (int)doc->images.size())
        return false;
    int hw = 0, hh = 0;
    gRenderer->partImageSize(lowerOf(reanim::ImageFileStem(doc->images[t.image])),
                             hw, hh);
    if (hw <= 0 || hh <= 0) return false;
    // Same vertex mapping as SdlReanimRenderer::Draw (shear included, no
    // rotation option here), so the anchor lands on the head as rendered.
    static const float D2R = 3.14159265358979323846f / 180.0f;
    float a = cosf(t.kx * D2R) * t.sx, b = sinf(t.kx * D2R) * t.sx;
    float c = sinf(-t.ky * D2R) * t.sy, d = cosf(-t.ky * D2R) * t.sy;
    float scale, ox, oy;
    zombieDrawGeom(geomFor(s.skin), (float)x_location, (float)y_location,
                   scale, ox, oy);
    const float cu[4] = {0.0f, (float)hw, (float)hw, 0.0f};
    const float cv[4] = {0.0f, 0.0f, (float)hh, (float)hh};
    float minX = 1e30f, maxX = -1e30f, minY = 1e30f;
    for (int i = 0; i < 4; i++) {
        float X = ox + scale * (a * cu[i] + c * cv[i] + t.x);
        float Y = oy + scale * (b * cu[i] + d * cv[i] + t.y);
        minX = std::min(minX, X); maxX = std::max(maxX, X);
        minY = std::min(minY, Y);
    }
    headCX = (minX + maxX) * 0.5f;
    headTopY = minY;
    artMul = scale / ZOMBIE_SCALE;
    // Retail draws the pat 1.2x on gargantuars (Zombie.cpp:6296-6299).
    if (s.skin == SKIN_GARGANTUAR) artMul *= 1.2f;
    return true;
}

// Wrap window of the game-side reanim_time counter (zombie_struct.cpp): with
// a playback-rate multiplier M the drawn pose completes the trimmed segment
// in len/M feed units, so the counter wraps there and the loop stays
// seamless (render computes fmod(M * t, len)). Chill halves the draw-side
// rate (chillRateMul), doubling the window.
float segmentLength(int directory_num, bool chilled) {
    if (!gReady) return 0.0f;
    Spec s = specFromState(directory_num, -1);
    if (!s.ok) return 0.0f;
    return (float)(s.last - s.first + 1) / (playbackRateMul(s) * chillRateMul(chilled));
}

// Public twin of playbackRateMul(Spec) for debug tooling: the retail
// playback-rate multiplier of a directory state (1.0 for unlisted/one-shot
// states and before init).
float playbackRateForDir(int directory_num, int armor_dir) {
    if (!gReady) return 1.0f;
    return playbackRateMul(specFromState(directory_num, armor_dir));
}

// --- Distance-driven walk feed (retail stride lock) ---------------------------
//
// repvz Zombie::UpdateAnimSpeed (Zombie.cpp:6683-6693): while a zombie is in a
// foot-walk state, the original derives the body anim rate from the ground
// speed so the drawn feet stay planted on the lawn:
//   aDistance = _ground TransX travel over the played segment window
//   aAnimRate = mVelX * (mFrameCount / aDistance) * 47.0f / mScaleZombie
// (Reanimation::Update, Reanimator.cpp:430: mAnimRate is doc-frames/s, and the
// track-velocity movement in UpdateZombieWalking, Zombie.cpp:4127-4129, then
// covers exactly that much ground per second).
//
// The clone's movement is authoritative (speed_px_per_tick, which is NOT
// changed here), so the same lock is expressed as reanim_time FEED UNITS PER
// BOARD PIXEL for the state's walk segment (negative = not a distance-driven
// walk, caller keeps its pinned rate). The draw chain (drawZombie /
// drawSkinZombie) advances the drawn doc frame by
//   playbackRateMul * chillRateMul * [docFpsMul on skin docs] * gZombieTimeMul
// per feed unit, and one doc frame spans (aDistance/len) * skin scale *
// gZombieScaleMul board px -- dividing the per-pixel budget by exactly that
// product makes the drawn _ground travel equal the ground the zombie really
// covered, at every speed roll. The chilled pair cancels (movement x0.4 --
// walk_v already carries it -- vs draw x0.5, so the 0.5 is divided back out),
// and so does the big-trouble pair (halved art gives 2x from the scale while
// the doubled time factor is undone), leaving the minis exactly foot-synced at
// their faster pace.
float walkFeedPerPx(int directory_num, int armor_dir, bool cold) {
    if (!gReady) return -1.0f;
    Spec s = specFromState(directory_num, armor_dir);
    if (!s.ok || s.segName == nullptr) return -1.0f;
    bool driven = false;
    for (const auto& r : kWalkFeedRules)
        if (r.skin == s.skin && strcmp(r.segment, s.segName) == 0) {
            driven = true;
            break;
        }
    if (!driven) return -1.0f;
    const reanim::ReanimDoc* doc = skinDoc(s.skin);
    if (doc == nullptr || doc->fps <= 0.0f) return -1.0f;
    // _ground TransX travel over the played (seam-trimmed) window. Doc-static,
    // so it is cached per (skin, first, last); 0/missing -> the retail guard
    // (aDistance >= 1e-6, Zombie.cpp:6687) falls back to the pinned rate.
    const int key = (s.skin << 20) + (s.first << 10) + s.last;
    static std::map<int, float> sGroundTravel;
    float aDistance;
    auto it = sGroundTravel.find(key);
    if (it != sGroundTravel.end()) {
        aDistance = it->second;
    } else {
        aDistance = 0.0f;
        for (const auto& tr : doc->tracks) {
            if (lowerOf(tr.name) != "_ground") continue;
            if ((int)tr.frames.size() > s.last)
                aDistance = tr.frames[s.last].x - tr.frames[s.first].x;
            break;
        }
        sGroundTravel[key] = aDistance;
    }
    if (aDistance < 1e-6f) return -1.0f;
    const float len = (float)(s.last - s.first + 1);
    // multiplier chain the draw applies per feed unit (the plain path skips
    // docFpsMul; its doc is 12 fps, so the factor would be 1 anyway)
    float chain = playbackRateMul(s) * gZombieTimeMul;
    if (s.skin != SKIN_PLAIN) chain *= docFpsMul(*doc);
    // board px one doc frame spans on screen
    const SkinGeom geo = geomFor(s.skin);
    const float pxPerDocFrame = (aDistance / len) * geo.scale * gZombieScaleMul;
    return 1.0f / (chain * pxPerDocFrame * chillRateMul(cold));
}

// The hide/forceShow sets the state machine implies (mirrors the original
// engine's ReanimShowTrack/ReanimShowPrefix calls): base-hidden accessories,
// per-variant reveals, torn-arm/part swaps. Shared by drawZombie and the
// debug snapshot trackStatuses().
static void buildVisibility(const Spec &s, std::unordered_set<std::string> &hide,
                            std::unordered_set<std::string> &forceShow) {
    for (const char *h : kBaseHidden) hide.insert(h);
    if (s.tongue) hide.erase("anim_tongue");
    if (s.ducky) {
        // Pool water lanes: the basic zombie wears its duck tube.
        hide.erase("zombie_duckytube");
        hide.erase("zombie_whitewater");
        hide.erase("zombie_whitewater2");
        forceShow.insert("zombie_duckytube");
        forceShow.insert("zombie_whitewater");
        forceShow.insert("zombie_whitewater2");
    }
    if (s.cone) hide.erase("anim_cone");
    if (s.bucket) hide.erase("anim_bucket");
    if (s.armless) {
        // arm fell off: hide the loose parts, swap in the torn upper-arm stump.
        // (2Pak: the outer-arm hand is a same-named "zombie_outerarm_lower"
        // duplicate, so it is covered here; there is no zombie_outerarm_hand
        // track in the base doc.)
        hide.insert("zombie_outerarm_lower");
    }
    if (s.flag) {
        // flag hand replaces the whole outer arm (it never reaches out)
        hide.erase("zombie_flaghand");
        hide.erase("zombie_innerarm_screendoor");
        hide.insert("anim_innerarm2");
        hide.insert("anim_innerarm3");
    }
    if (s.door) {
        hide.erase("anim_screendoor");
        hide.erase("zombie_innerarm_screendoor");
        if (s.tongue) {
            // Eating: the data flags the door-holding hand/pole-arm f=-1 in
            // anim_eat -- the zombie claws the plant with its regular arms,
            // the arm segment behind the door stays.
            hide.insert("zombie_innerarm_screendoor_hand");
            hide.insert("zombie_outerarm_screendoor");
        } else {
            // The door variant replaces both regular arms with its own parts.
            hide.erase("zombie_innerarm_screendoor_hand");
            hide.erase("zombie_outerarm_screendoor");
            hide.insert("anim_innerarm1");
            hide.insert("anim_innerarm2");
            hide.insert("anim_innerarm3");
            hide.insert("zombie_outerarm_hand");
            hide.insert("zombie_outerarm_upper");
            hide.insert("zombie_outerarm_lower");
        }
    }
    // f=-1 tracks the code deliberately reveals (ReanimShowTrack semantics):
    // the art for armor/flag/door parts follows the head/neck across every
    // animation but stays flagged hidden in the data.
    if (s.cone) forceShow.insert("anim_cone");
    if (s.bucket) forceShow.insert("anim_bucket");
    if (s.flag) forceShow.insert("zombie_flaghand");
    if (s.door) {
        forceShow.insert("anim_screendoor");
        forceShow.insert("zombie_innerarm_screendoor");
        if (!s.tongue) { // eating: the data hides the door arms itself (f=-1)
            forceShow.insert("zombie_innerarm_screendoor_hand");
            forceShow.insert("zombie_outerarm_screendoor");
        }
    }
}

// Delta transform of "zombie_flaghand" vs. its reference pose (first frame of
// anim_walk, where the artist aligned the pole: hand kx 328 deg ~= pole
// 326.6 deg). out gets M_cur * M_ref^-1 decomposed back to a Transform, so an
// attached document keeps its authored pose and only follows the hand's
// rocking/translation. refReady must persist across calls (static at caller).
static bool flagHandDelta(float absFrame, int segFirst, int segLast,
                          reanim::Transform& out, bool& refReady) {
    static const float D2R = 3.14159265358979323846f / 180.0f;
    auto toMatrix = [&](const reanim::Transform& t,
                        float& a, float& b, float& c, float& d) {
        float kxr = t.kx * D2R, kyr = -t.ky * D2R;
        a = cosf(kxr) * t.sx; b = sinf(kxr) * t.sx;
        c = sinf(kyr) * t.sy; d = cosf(kyr) * t.sy;
    };
    const reanim::Track* hand = nullptr;
    for (const reanim::Track& tr : gDoc.tracks) {
        if (lowerOf(tr.name) == "zombie_flaghand") { hand = &tr; break; }
    }
    if (!hand) return false;
    static float rA, rB, rC, rD, rX, rY;
    if (!refReady) {
        // The alignment pose is the head of the merged 2Pak walk clip (abs
        // frame 31), which is anim_walk's first frame.
        auto it = gDoc.segments.find("anim_walk");
        int refFrame = it != gDoc.segments.end() ? it->second.first : 0;
        reanim::Transform ref = reanim::SampleTrack(*hand, (float)refFrame);
        toMatrix(ref, rA, rB, rC, rD);
        rX = ref.x; rY = ref.y;
        refReady = true;
    }
    reanim::Transform cur = reanim::SampleTrack(*hand, absFrame, segFirst, segLast);
    float ca, cb, cc, cd;
    toMatrix(cur, ca, cb, cc, cd);
    float det = rA * rD - rB * rC;
    if (fabsf(det) < 1e-6f) return false;
    // inverse of M_ref
    float ia = rD / det, ib = -rB / det, ic = -rC / det, id = rA / det;
    float ix = -(ia * rX + ic * rY), iy = -(ib * rX + id * rY);
    // delta = M_cur * M_ref^-1
    float da = ca * ia + cc * ib, db = cb * ia + cd * ib;
    float dc = ca * ic + cc * id, dd = cb * ic + cd * id;
    float dx = ca * ix + cc * iy + cur.x, dy = cb * ix + cd * iy + cur.y;
    out.x = dx; out.y = dy;
    out.sx = hypotf(da, db); out.kx = atan2f(db, da) / D2R;
    out.sy = hypotf(dc, dd); out.ky = atan2f(-dc, dd) / D2R;
    out.a = 1.0f; out.f = 0.0f; out.image = -1;
    return true;
}

bool drawZombie(int directory_num, int armor_dir, float reanim_time,
                int x_location, int y_location, bool cold, bool attacked,
                const std::unordered_set<std::string> *extraHide,
                const std::unordered_set<std::string> *extraShow,
                int *partsDrawn, reanim::DrawStats *stats, float attack_fade) {
    if (!gReady) return false;
    Spec s = specFromState(directory_num, armor_dir);
    if (!s.ok) return false;
    attack_fade = std::clamp(attack_fade, 0.0f, 1.0f);
    if (s.skin != SKIN_PLAIN)
        return drawSkinZombie(s, reanim_time, x_location, y_location, cold,
                              attacked, extraHide, extraShow, partsDrawn,
                              attack_fade);

    float len = (float)(s.last - s.first + 1);
    float local = timeScaledLocal(reanim_time * playbackRateMul(s) *
                                      chillRateMul(cold), len);
    float absFrame = (float)s.first + local;

    std::unordered_set<std::string> hide, forceShow;
    buildVisibility(s, hide, forceShow);
    if (extraHide) {
        for (const auto& t : *extraHide) hide.insert(lowerOf(t));
    }
    if (extraShow) { // reveal instead: un-hide AND force through f=-1 frames
        for (const auto& t : *extraShow) {
            hide.erase(lowerOf(t));
            forceShow.insert(lowerOf(t));
        }
    }

    std::unordered_map<std::string, std::string> overrides;
    if (s.cone == 2) overrides["anim_cone"] = "zombie_cone2";
    if (s.cone == 3) overrides["anim_cone"] = "zombie_cone3";
    if (s.bucket == 2) overrides["anim_bucket"] = "zombie_bucket2";
    if (s.bucket == 3) overrides["anim_bucket"] = "zombie_bucket3";
    if (s.door == 2) overrides["anim_screendoor"] = "zombie_screendoor2";
    if (s.door == 3) overrides["anim_screendoor"] = "zombie_screendoor3";
    if (s.armless) overrides["zombie_outerarm_upper"] = "zombie_outerarm_upper2";

    reanim::DrawOptions opt;
    opt.tint = cold ? SDL_Color{110, 170, 255, 255} : SDL_Color{255, 255, 255, 255};
    opt.flash = attacked;
    // fade-out over the blink budget (repvz mJustGotShotCounter grayness
    // ramp Zombie.cpp:5744-5750 compressed into the 10-tick blink window)
    opt.flashAlpha = (Uint8)std::clamp(lroundf(opt.flashAlpha * attack_fade), 0l, 255l);
    opt.loopWrapFirst = s.first;
    opt.loopWrapLast = s.last;
    opt.loopBlendFrames = loopBlendFrames(s.skin, s.first, s.last);
    if (!overrides.empty()) opt.imageOverrides = &overrides;
    if (!forceShow.empty()) opt.forceShow = &forceShow;

    float scale, ox, oy;
    zombieDrawGeom(baseGeom(), (float)x_location, (float)y_location, scale, ox, oy);
    auto hideTrack = [&](const std::string& n) { return hide.count(lowerOf(n)) > 0; };
    reanim::DrawStats body =
        gRenderer->Draw(gDoc, absFrame, ox, oy, scale, hideTrack, opt);

    int extraParts = 0;
    reanim::DrawStats flagStat;
    bool haveFlagStat = false;
    if (s.flag && gFlagReady) {
        // The waving flag is its own reanim, delta-attached to the body's
        // "zombie_flaghand" track: the child doc was authored in zombie space
        // aligned with the hand's rest pose (hand kx 328 deg vs pole 326.6 deg
        // at the walk-clip head -> reference), so we apply only the hand's delta
        // from that pose -- the pole keeps its authored pose and rocks/bobs
        // with the hand instead of inheriting its full matrix.
        float ft = timeScaledLocal(reanim_time * playbackRateMul(s),
                                   gFlagLoopLen > 0.0f ? gFlagLoopLen
                                                       : (float)gFlagDoc.frameCount);
        reanim::DrawOptions flagOpt = opt;
        static reanim::Transform sDeltaT;
        static bool sRefReady = false;
        if (flagHandDelta(absFrame, s.first, s.last, sDeltaT, sRefReady))
            flagOpt.attachFrom = &sDeltaT;
        // flagdoc tracks are all drawn by default; extraHide/extraShow apply.
        reanim::DrawStats flag =
            (extraHide || extraShow)
                ? gRenderer->Draw(gFlagDoc, ft, ox, oy, scale, hideTrack, flagOpt)
                : gRenderer->Draw(gFlagDoc, ft, ox, oy, scale, nullptr, flagOpt);
        extraParts = flag.partsDrawn;
        flagStat = flag;
        haveFlagStat = true;
    }
    if (partsDrawn) *partsDrawn = body.partsDrawn + extraParts;
    if (stats) {
        *stats = body;
        if (haveFlagStat) {
            stats->partsDrawn += flagStat.partsDrawn;
            stats->missingImages += flagStat.missingImages;
            if (flagStat.hasBounds) {
                if (stats->hasBounds) {
                    stats->minX = std::min(stats->minX, flagStat.minX);
                    stats->minY = std::min(stats->minY, flagStat.minY);
                    stats->maxX = std::max(stats->maxX, flagStat.maxX);
                    stats->maxY = std::max(stats->maxY, flagStat.maxY);
                } else {
                    *stats = flagStat;
                }
            }
        }
    }
    return true;
}

std::vector<std::string> trackNames() {
    std::vector<std::string> out;
    if (!gReady) return out;
    for (const auto& tr : gDoc.tracks) out.push_back(lowerOf(tr.name));
    if (gFlagReady)
        for (const auto& tr : gFlagDoc.tracks) out.push_back(lowerOf(tr.name));
    return out;
}

std::vector<TrackStatus> trackStatuses(int directory_num, int armor_dir,
                                       float reanim_time) {
    std::vector<TrackStatus> out;
    if (!gReady) return out;
    if (reanim_time < 0) reanim_time = 0;
    Spec s = specFromState(directory_num, armor_dir);
    std::unordered_set<std::string> hide, forceShow;
    buildVisibility(s, hide, forceShow);
    float absFrame;
    if (s.ok) {
        float len = (float)(s.last - s.first + 1);
        absFrame = (float)s.first + fmodf(reanim_time * playbackRateMul(s), len);
    } else {
        absFrame = fmodf(reanim_time, (float)std::max(1, gDoc.frameCount));
    }
    for (const auto& tr : gDoc.tracks) {
        reanim::Transform t = reanim::SampleTrack(tr, absFrame);
        std::string n = lowerOf(tr.name);
        out.push_back({n, tr.hasAnyImage, hide.count(n) > 0,
                       t.f < -0.5f, forceShow.count(n) > 0});
    }
    if (gFlagReady) {
        float ft = fmodf(reanim_time * (s.ok ? playbackRateMul(s) : 1.0f),
                         gFlagLoopLen > 0.0f ? gFlagLoopLen
                                             : (float)gFlagDoc.frameCount);
        for (const auto& tr : gFlagDoc.tracks) {
            reanim::Transform t = reanim::SampleTrack(tr, ft);
            std::string n = lowerOf(tr.name);
            // drawZombie only draws the flag doc for flag-bearer states
            bool notDrawnThisState = !s.flag;
            out.push_back({n, tr.hasAnyImage, hide.count(n) > 0 || notDrawnThisState,
                           t.f < -0.5f, forceShow.count(n) > 0});
        }
    }
    return out;
}

// Death-anim playback rate in reanim frames/sec, repvz Zombie::PlayDeathAnim
// (Zombie.cpp:9078-9108): plain zombies tumble at a per-corpse
// RandRangeFloat(24,30); football 24, snorkel/yeti/gargantuar 14, digger 18.
// The clone has no mZombieType here, so the rate keys off the skin (the
// subclasses pass their own doc index). `seed` makes the band deterministic
// per corpse (draw side and linger side must agree).
float deathRateFps(int skin, unsigned seed) {
    switch (skin) {
    case SKIN_FOOTBALL: return 24.0f;
    case SKIN_GARGANTUAR: return 14.0f;
    case SKIN_SNORKEL: return 14.0f;
    case SKIN_DIGGER: return 18.0f;
    case SKIN_YETI: return 14.0f;
    default: return 24.0f + 6.0f * ((seed >> 8) & 0xFFFF) / 65535.0f;
    }
}

// Seconds-real-time layout of a death: the fall (clip length / rate), then
// the corpse holds the final pose 0.9s and fades over the last 0.1s before
// the part is erased (repvz mZombieFade = 100 cs, alpha pinned 255 until
// mZombieFade <= 10 -- Zombie.cpp:9384-9387 + the DrawZombie fade ramp).
// In-pool deaths (anim_waterdeath) linger a tenth of that: mZombieFade = 10.
int deathLingerTicks(int skin, bool in_pool, unsigned seed) {
    const reanim::ReanimDoc* doc = skinDoc(skin);
    if (doc == nullptr) return 60;
    int first, last;
    const char* seg = "anim_death";
    if (in_pool && docSegmentRange(*doc, "anim_waterdeath", first, last))
        seg = "anim_waterdeath";
    if (!docSegmentRange(*doc, seg, first, last)) return 60;
    float rate = deathRateFps(skin, seed);
    float fall_s = (float)(last - first + 1) / rate;
    return (int)ceilf(fall_s * 60.0f) + (in_pool ? 6 : 60);
}

void drawDeathPart(float aging_time, int x_location, int y_location, bool cold,
                   int skin, bool in_pool) {
    const reanim::ReanimDoc* doc = skinDoc(skin);
    if (doc == nullptr) return;
    unsigned seed = (unsigned)x_location * 2654435761u ^ (unsigned)y_location * 40503u;
    int first, last;
    const char* seg = "anim_death";
    if (in_pool && !docSegmentRange(*doc, "anim_waterdeath", first, last))
        in_pool = false;
    if (!docSegmentRange(*doc, in_pool ? "anim_waterdeath" : seg, first, last)) return;
    const SkinGeom geo = geomFor(skin);
    // aging_time runs in 12-fps reanim frames (0.2 per 60 Hz tick); the death
    // plays at its own rate (24-30 fps etc.), NOT at the walk feed rate.
    const float rate = deathRateFps(skin, seed);
    float absFrame = (float)first + aging_time * rate / 12.0f;
    if (absFrame > (float)last) absFrame = (float)last; // freeze at final pose

    const float t_s = aging_time / 12.0f;
    const float animEnd_s = (float)(last - first + 1) / rate;
    // hold full opacity for fadeHold seconds past the fall, then fade out
    const float fadeHold = in_pool ? 0.0f : 0.9f; // pool: mZombieFade 10 cs
    const float fadeLen = 0.1f;                    // the original's 10 cs ramp
    float fade = std::clamp((animEnd_s + fadeHold + fadeLen - t_s) / fadeLen,
                            0.0f, 1.0f);

    float scale, ox, oy;
    zombieDrawGeom(geo, (float)x_location, (float)y_location, scale, ox, oy);

    std::unordered_set<std::string> hide;
    if (skin == SKIN_PLAIN)
        for (const char* h : kBaseHidden) hide.insert(h);
    // The death segment hides the head group in the data (the engine severs
    // it); the head itself is re-added below as the tossed head pop.

    reanim::DrawOptions opt;
    opt.tint = cold ? SDL_Color{110, 170, 255, 255} : SDL_Color{255, 255, 255, 255};
    opt.alphaMul = fade;
    std::unordered_set<std::string> forceShow;
    if (in_pool && skin == SKIN_PLAIN) {
        // died in the water wearing the ducky tube: keep the tube + wake
        // tracks visible through the water-death segment (same reveal rule
        // as buildVisibility's duckytube state).
        hide.erase("zombie_duckytube");
        hide.erase("zombie_whitewater");
        hide.erase("zombie_whitewater2");
        forceShow.insert("zombie_duckytube");
        forceShow.insert("zombie_whitewater");
        forceShow.insert("zombie_whitewater2");
        opt.forceShow = &forceShow;
    }
    gRenderer->Draw(*doc, absFrame, ox, oy, scale,
                    [&](const std::string& n) { return hide.count(lowerOf(n)) > 0; }, opt);

    // Tossed head pop: only the skins with a dedicated DropSpec (plain..dancer
    // and the backup dancer); the area 3-5 skins, yeti and ducky tube play
    // the clean corpse.
    if ((skin >= SKIN_PLAIN && skin <= SKIN_DANCER) || skin == SKIN_BACKUP)
        drawPop(dropSpecFor(DEATH_PART_IMG, skin), aging_time, ox, oy, scale,
                cold, fade, seed);
}

void drawDropPart(int drop_img, float aging_time, int x_location, int y_location,
                  bool cold, int skin) {
    if (!gReady) return;
    const DropSpec& spec = dropSpecFor(drop_img, skin);
    if (spec.useFlagDoc && !gFlagReady) return;
    if (!spec.useFlagDoc && skinDoc(spec.docSkin) == nullptr) return;
    int ticks = (int)(aging_time * 5.0f);
    float t01 = spec.phys.lingerTicks > 0
                    ? std::clamp((float)ticks / (float)spec.phys.lingerTicks, 0.0f, 1.0f)
                    : 0.0f;
    float span = 1.0f - spec.phys.fadeFrom;
    float fade = span > 0.0f
                     ? 1.0f - std::clamp((t01 - spec.phys.fadeFrom) / span, 0.0f, 1.0f)
                     : 1.0f;
    // Same origin/scale chain as the corpse/body draw (zombieDrawGeom):
    // with scale-mul 1 this lands exactly on the old x+anchorDX anchor, and
    // big-trouble minis shrink + re-anchor around the feet like the body.
    const SkinGeom geo = geomFor(skin);
    float scale, ox, oy;
    zombieDrawGeom(geo, (float)x_location, (float)y_location, scale, ox, oy);
    unsigned seed = (unsigned)x_location * 2654435761u ^ (unsigned)y_location * 40503u
                    ^ (unsigned)drop_img * 97u;
    drawPop(spec, aging_time, ox, oy, scale, cold, fade, seed);
}

int dropPartLinger(int drop_img) {
    return dropSpecFor(drop_img, 0).phys.lingerTicks;
}

// Burn/explosion kills (cherry bomb, jalapeno, doom-shroom, potato mine,
// explode-o-nut): the generic charred doc for common zombies, dedicated art
// per heavy type (repvz Zombie.cpp:8824-8862). Bungee/yeti/flying zombies
// burn in place in the original (PHASE_ZOMBIE_BURNED freeze, no charred art);
// here every burned zombie gets the generic doc like the old burnt sheet did.
int charredKindForType(int zombie_type) {
    switch (zombie_type) {
    case IMP_TYPE: return CHARRED_IMP;
    case DIGGER_TYPE: return CHARRED_DIGGER;
    case ZOMBONI_TYPE: return CHARRED_ZAMBONI;
    case CATAPULT_TYPE: return CHARRED_CATAPULT;
    case GARGANTUAR_TYPE: return CHARRED_GARGANTUAR;
    default: return CHARRED_GENERIC;
    }
}

// One frame of the charred corpse; `ticks` runs in game ticks. The timeline
// is played once at the doc's authored fps, clamped on its last frame (the
// part erases itself at charredLingerTicks).
void drawCharredPart(int kind, int ticks, int x_location, int y_location) {
    if (!gReady || kind < 0 || kind >= COUNT_CHARRED) return;
    if (!initCharred(kind)) return;
    const reanim::ReanimDoc& doc = gCharredDoc[kind];
    int first = gCharredFirst[kind], last = gCharredLast[kind];
    float len = (float)(last - first + 1);
    if (len <= 0.0f) return;
    float local = ticks * gZombieTimeMul * (doc.fps / 60.0f);
    if (local > len - 1.0f) local = len - 1.0f;
    float absFrame = (float)first + local;

    float scale, ox, oy;
    zombieDrawGeom(gCharredGeom[kind], (float)x_location, (float)y_location, scale, ox, oy);
    reanim::DrawOptions opt;
    gRenderer->Draw(doc, absFrame, ox, oy, scale, nullptr, opt);
}

int charredLingerTicks(int kind) {
    if (kind < 0 || kind >= COUNT_CHARRED || !initCharred(kind)) return 120;
    const reanim::ReanimDoc& doc = gCharredDoc[kind];
    float frames = (float)(gCharredLast[kind] - gCharredFirst[kind] + 1);
    // timeline frames at the authored fps -> game ticks
    return (int)ceilf(frames * 60.0f / doc.fps);
}

// Horizontal distance the vault animation carries the vaulter, converted to
// game pixels with the pole skin's calibration. The clone advances the
// logical x by this amount while the jump segment plays, so the landing pose
// flows straight into the no-pole walk.
float poleVaultTravelPx() {
    if (!gSkinReady[SKIN_POLEVAULT]) return 150.0f;
    int jf, jl, wf, wl;
    if (!docSegmentRange(gSkinDoc[SKIN_POLEVAULT], "anim_jump", jf, jl) ||
        !docSegmentRange(gSkinDoc[SKIN_POLEVAULT], "anim_walk", wf, wl))
        return 150.0f;
    const reanim::Track* body = nullptr;
    for (const reanim::Track& tr : gSkinDoc[SKIN_POLEVAULT].tracks)
        if (lowerOf(tr.name) == "zombie_polevaulter_body1") { body = &tr; break; }
    if (body == nullptr) return 150.0f;
    float xJumpEnd = reanim::SampleTrack(*body, (float)jl).x;
    float xWalkStart = reanim::SampleTrack(*body, (float)wf).x;
    float travel = (xWalkStart - xJumpEnd) * gSkinGeom[SKIN_POLEVAULT].scale;
    return travel > 0.0f ? travel : 150.0f;
}

} // namespace reanimz

namespace reanimz {

// (Roof agent) Free-segment drivers for the roof zombie state machines and
// the Zomboss boss fight. Reuses the skin docs/geometry calibrated above.

float skinSegmentLength(int skin, const char* segment) {
    if (skin < SKIN_PLAIN || skin >= COUNT_ZOMBIE_SKIN)
        return 0.0f;
    const reanim::ReanimDoc* doc = skinDoc(skin);
    if (doc == nullptr)
        return 0.0f;
    int first, last;
    if (!docSegmentRange(*doc, segment, first, last))
        return 0.0f;
    // Matches drawSkinSegment's looping play window (seam-duplicate last frame
    // trimmed), which is what gameplay wraps looping segments against.
    return (float)(loopPlayLast(*doc, first, last) - first + 1);
}

bool drawSkinSegment(int skin, const char* segment, float reanim_time,
                     int x_location, int y_location, bool cold, bool attacked,
                     int ddx, int ddy, float scale_mul, bool loop,
                     const std::unordered_set<std::string>* extraHide,
                     const std::unordered_set<std::string>* extraShow,
                     int* partsDrawn, reanim::DrawStats* stats) {
    const reanim::ReanimDoc* doc;
    if (skin == SKIN_FLAGPOLE) {
        if (!gFlagReady) return false;
        doc = &gFlagDoc;
    } else {
        doc = skinDoc(skin);
        if (doc == nullptr) return false;
    }
    int first, last;
    if (!docSegmentRange(*doc, segment, first, last)) return false;
    const SkinGeom geo = skin == SKIN_FLAGPOLE ? baseGeom() : geomFor(skin);

    float len = (float)((loop ? loopPlayLast(*doc, first, last) : last) - first + 1);
    float local = reanim_time < 0 ? 0 : reanim_time;
    local *= gZombieTimeMul; // big-trouble segments play faster too
    local *= docFpsMul(*doc); // authored doc fps (digger 16, zamboni 30, ...)
    if (loop)
        local = fmodf(local, len);
    else if (local > len - 1)
        local = len - 1; // one-shot segments freeze on their final pose
    float absFrame = (float)first + local;

    reanim::DrawOptions opt;
    opt.tint = cold ? SDL_Color{110, 170, 255, 255} : SDL_Color{255, 255, 255, 255};
    opt.flash = attacked;
    opt.loopWrapFirst = first;
    opt.loopWrapLast = first + (int)len - 1; // trimmed last when looping
    if (loop)
        opt.loopBlendFrames = loopBlendFrames(skin, first, first + (int)len - 1);

    // Debug overrides (anim viewer; drawZombie semantics). SKIN_PLAIN starts
    // from the game's base hide set so the viewer shows the game look; an
    // extraShow un-hides a track AND lets it render on f=-1 frames.
    std::unordered_set<std::string> hideL, showL;
    if (skin == SKIN_PLAIN)
        for (const char* h : kBaseHidden) hideL.insert(h);
    if (extraHide)
        for (const auto& t : *extraHide) hideL.insert(lowerOf(t));
    if (extraShow)
        for (const auto& t : *extraShow) {
            hideL.erase(lowerOf(t));
            showL.insert(lowerOf(t));
        }
    if (!showL.empty()) opt.forceShow = &showL;
    auto hideTrack = [&](const std::string& n) { return hideL.count(lowerOf(n)) > 0; };
    const bool anyHide = skin == SKIN_PLAIN || extraHide != nullptr || extraShow != nullptr;

    float ox = (float)x_location + geo.anchorDX * scale_mul + (float)ddx;
    float oy = (float)y_location + geo.anchorDY * scale_mul + (float)ddy;
    reanim::DrawStats st;
    if (anyHide)
        st = gRenderer->Draw(*doc, absFrame, ox, oy, geo.scale * scale_mul, hideTrack, opt);
    else
        st = gRenderer->Draw(*doc, absFrame, ox, oy, geo.scale * scale_mul, nullptr, opt);
    if (partsDrawn) *partsDrawn = st.partsDrawn;
    if (stats) *stats = st;
    return true;
}

// --- debug tooling (anim viewer): doc introspection --------------------------

const reanim::ReanimDoc* debugDoc(int skin) {
    if (skin == SKIN_FLAGPOLE) return gFlagReady ? &gFlagDoc : nullptr;
    return skinDoc(skin);
}

std::vector<SegmentInfo> skinSegments(int skin) {
    std::vector<SegmentInfo> out;
    const reanim::ReanimDoc* doc = debugDoc(skin);
    if (doc == nullptr) return out;
    for (const auto& kv : doc->segments) out.push_back({kv.first, kv.second.first, kv.second.last});
    std::sort(out.begin(), out.end(),
              [](const SegmentInfo& a, const SegmentInfo& b) { return a.first < b.first; });
    return out;
}

std::vector<std::string> skinTrackNames(int skin) {
    std::vector<std::string> out;
    const reanim::ReanimDoc* doc = debugDoc(skin);
    if (doc == nullptr) return out;
    for (const auto& tr : doc->tracks) out.push_back(lowerOf(tr.name));
    return out;
}

std::vector<TrackStatus> segmentTrackStatuses(int skin, const char* segment,
                                              float reanim_time) {
    std::vector<TrackStatus> out;
    const reanim::ReanimDoc* doc = debugDoc(skin);
    if (doc == nullptr) return out;
    int first, last;
    if (!docSegmentRange(*doc, segment, first, last)) return out;
    int playLast = loopPlayLast(*doc, first, last); // mirrors drawSkinSegment's loop
    float len = (float)(playLast - first + 1);
    float local = reanim_time < 0 ? 0 : fmodf(reanim_time * docFpsMul(*doc), len);
    float absFrame = (float)first + local;
    for (const auto& tr : doc->tracks) {
        reanim::Transform t = reanim::SampleTrack(tr, absFrame, first, playLast);
        std::string n = lowerOf(tr.name);
        bool codeHidden = false;
        if (skin == SKIN_PLAIN)
            for (const char* h : kBaseHidden)
                if (n == h) { codeHidden = true; break; }
        out.push_back({n, tr.hasAnyImage, codeHidden, t.f < -0.5f, false});
    }
    return out;
}

} // namespace reanimz
