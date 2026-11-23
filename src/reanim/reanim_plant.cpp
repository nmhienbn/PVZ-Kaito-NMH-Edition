#include "reanim_plant.hpp"

#include "draw/ImageType.hpp"
#include "draw/plant_icon_dir.hpp"
#include "game_param.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace reanimplant {

namespace {

const char* REANIM_DIR = "./resources/reanim";

struct PlantSkin {
    const char* reanimFile;
    const char* refSegment; // calibration reference; "" = use refFirst/refLast
    int refFirst, refLast;
    // --- placement targets (measured from the sprite sheets, in screen px) ---
    float artH;      // on-screen idle height (px) the art is pinned to (retail targets)
    float padBottom; // sheet art bottom distance above the frame's bottom edge
    float dx, dy;    // extra screen-px offset (after scaling; +dy = down)
    // -------------------------------------------------------------------------
    reanim::ReanimDoc doc;
    bool loaded = false;
    bool calibrated = false;
    float scale = 1.0f;
    float refCX = 0.0f, refBottom = 0.0f; // measured in reanim units
};

// Indexed by enum PlantType (game_param.hpp).
// artH is the on-screen height (px) that calibrate() pins each skin's idle
// median to; the pinned values are the retail PvZ1 median idle heights
// measured from the reference reanim set (assets_src/general_sprites/reanim,
// engine scale 1.0, PotatoMine x0.8) -- see build_artifacts/scale_audit.md
// table A / S8.2. padBottom keeps the original sheet's foot line, and W
// follows the 2Pak art's own aspect (the audit's S8.2 aspect-tradeoff note:
// height wins, width may still ride 15-35% off retail on some plants).
const int SKIN_COUNT = PLANT_COUNT;
PlantSkin gSkins[SKIN_COUNT] = {
    /* PEASHOOTER_TYPE */ {"PeaShooter.reanim", "anim_full_idle", 0, 0, 52.0f, 0.0f, 0.0f, 0.0f},
    /* SUNFLOWER_TYPE  */ {"Sunflower.reanim", "", 3, 32, 70.0f, 1.0f, 0.0f, 0.0f},
    /* WALLNUT_TYPE    */ {"Wallnut.reanim", "anim_idle", 0, 0, 72.0f, 1.0f, 0.0f, 0.0f},
    /* SNOWPEA_TYPE    */ {"Snowpea.reanim", "anim_full_idle", 0, 0, 68.0f, 2.0f, 0.0f, 0.0f},
    /* POTATOMINE_TYPE */ {"PotatoMine.reanim", "anim_armed", 0, 0, 54.0f, 0.7f, 0.0f, 0.0f},
    /* CHERRYBOMB_TYPE */ {"CherryBomb.reanim", "anim_idle", 0, 0, 64.0f, 1.3f, 0.0f, 0.0f},
    // Repeater shares the PeaShooter body; its eyebrows/leaf swap in via
    // imageOverrides on the two baked-in slots (see the drawPlant note).
    /* REPEATER_TYPE   */ {"PeaShooter.reanim", "anim_full_idle", 0, 0, 52.0f, 0.0f, 0.0f, 0.0f},
    // Chomper: retail-relative ratio (retail chomper 98.5 vs sunflower 70)
    // would land artH ~139, which reads over-sized next to the other plants;
    // artH 95 lands it at ~82 px wide (cell-matched, H within 4% of retail).
    /* CHOMPER_TYPE    */ {"Chomper.reanim", "anim_idle", 0, 0, 95.0f, 2.0f, 0.0f, 0.0f},
    /* PUFFSHROOM_TYPE */ {"Puffshroom.reanim", "anim_idle", 0, 0, 35.0f, 2.0f, 0.0f, 0.0f},
    // Night mushrooms (2-1..2-10 set). The small Sun-shroom uses the small
    // idle as the calibration ref (the grown form is drawn by the same doc).
    /* SUNSHROOM_TYPE    */ {"SunShroom.reanim", "anim_idle", 0, 0, 37.0f, 2.0f, 0.0f, 0.0f},
    /* FUMESHROOM_TYPE   */ {"FumeShroom.reanim", "anim_idle", 0, 0, 76.0f, 2.0f, 0.0f, 0.0f},
    /* GRAVEBUSTER_TYPE  */ {"GraveBuster.reanim", "anim_idle", 0, 0, 72.0f, 0.0f, 0.0f, 0.0f},
    /* HYPNOSHROOM_TYPE  */ {"HypnoShroom.reanim", "anim_idle", 0, 0, 76.0f, 2.0f, 0.0f, 0.0f},
    /* SCAREDYSHROOM_TYPE*/ {"ScaredyShroom.reanim", "anim_idle", 0, 0, 66.0f, 2.0f, 0.0f, 0.0f},
    /* ICESHROOM_TYPE    */ {"IceShroom.reanim", "anim_idle", 0, 0, 72.0f, 2.0f, 0.0f, 0.0f},
    /* DOOMSHROOM_TYPE   */ {"DoomShroom.reanim", "anim_idle", 0, 0, 80.0f, 2.0f, 0.0f, 0.0f},

    // Area 3-5 set (STUB-AREA-AGENT wrappers idle on anim_idle). Spikeweed
    // uses the Caltrop reanim (canon art name); Kernel-pult is Cornpult in
    // the 2Pak set; Flower Pot ships as Pot.reanim, Marigold lowercase.
    /* LILYPAD_TYPE     */ {"Lilypad.reanim", "anim_idle", 0, 0, 56.0f, 1.0f, 0.0f, 0.0f},
    /* SQUASH_TYPE      */ {"Squash.reanim", "anim_idle", 0, 0, 82.0f, 2.0f, 0.0f, 0.0f},
    // ThreePeater has no anim_idle marker; the 4..21 sway window is the
    // (all-three-heads) idle, 22..33 the shooting flare.
    /* THREEPEATER_TYPE */ {"ThreePeater.reanim", "", 4, 21, 60.0f, 2.0f, 0.0f, 0.0f},
    /* TANGLEKELP_TYPE  */ {"Tanglekelp.reanim", "anim_idle", 0, 0, 64.0f, 1.0f, 0.0f, 0.0f},
    /* JALAPENO_TYPE    */ {"Jalapeno.reanim", "anim_idle", 0, 0, 70.0f, 2.0f, 0.0f, 0.0f},
    /* SPIKEWEED_TYPE   */ {"Caltrop.reanim", "anim_idle", 0, 0, 33.0f, 1.0f, 0.0f, 0.0f},
    /* TORCHWOOD_TYPE   */ {"Torchwood.reanim", "anim_idle", 0, 0, 92.0f, 2.0f, 0.0f, 0.0f},
    // Tallnut's anim_idle marker is empty; the body rests static at 86..120.
    /* TALLNUT_TYPE     */ {"Tallnut.reanim", "", 86, 120, 118.0f, 2.0f, 0.0f, 0.0f},
    // Fog set (area 4): labels without f markers do not register as segments,
    // so the refs are explicit timeline windows (fps 12).
    /* SEASHROOM_TYPE   */ {"SeaShroom.reanim", "", 0, 17, 47.0f, 2.0f, 0.0f, 0.0f},
    /* PLANTERN_TYPE    */ {"Plantern.reanim", "", 0, 90, 86.0f, 2.0f, 0.0f, 0.0f},
    /* CACTUS_TYPE      */ {"Cactus.reanim", "", 0, 33, 79.0f, 2.0f, 0.0f, 0.0f},
    /* BLOVER_TYPE      */ {"Blover.reanim", "", 0, 28, 86.0f, 2.0f, 0.0f, 0.0f},
    // Split Pea calibrates on the union of its three cluster windows (leaves
    // 7..23, back head 24..53, front head 54..87); drawSplitPea composes it.
    /* SPLITPEA_TYPE    */ {"Splitpea.reanim", "", 7, 70, 65.0f, 2.0f, 0.0f, 0.0f},
    /* STARFRUIT_TYPE   */ {"Starfruit.reanim", "", 0, 24, 62.0f, 2.0f, 0.0f, 0.0f},
    /* PUMPKIN_TYPE: width-anchored, not height: retail PvZ1 Pumpkin.reanim
       idle union is 92.0 px wide (= board cell 80 px + 15%, the shell wraps
       the plant); the 2Pak shell art is flatter (median W/H 96.0x45.6), so
       artH = 45.6 * 92 / 96 = 43.7 keeps the retail width and shell aspect. */
    /* PUMPKIN_TYPE     */ {"Pumpkin.reanim", "", 0, 24, 44.0f, 1.0f, 0.0f, 0.0f},
    /* MAGNETSHROOM_TYPE*/ {"Magnetshroom.reanim", "", 0, 44, 72.0f, 2.0f, 0.0f, 0.0f},
    /* CABBAGEPULT_TYPE */ {"Cabbagepult.reanim", "anim_idle", 0, 0, 76.0f, 2.0f, 0.0f, 0.0f},
    // Flower Pot: the old sheet target (game_height 34) was tuned for the flat
    // legacy sprite; against retail PvZ1 the pot is 65.6x64.9 px for a ~70 px
    // sunflower. artH 60 puts the 2Pak pot at 61 px wide (~77% of the cell),
    // stubby under the plant like retail instead of pebble-sized.
    /* FLOWERPOT_TYPE   */ {"Pot.reanim", "anim_idle", 0, 0, 60.0f, 0.0f, 0.0f, 0.0f},
    /* KERNELPULT_TYPE  */ {"Cornpult.reanim", "anim_idle", 0, 0, 90.0f, 2.0f, 0.0f, 0.0f},
    /* COFFEEBEAN_TYPE  */ {"Coffeebean.reanim", "anim_idle", 0, 0, 59.0f, 2.0f, 0.0f, 0.0f},
    /* GARLIC_TYPE      */ {"Garlic.reanim", "anim_idle", 0, 0, 57.0f, 2.0f, 0.0f, 0.0f},
    /* UMBRELLALEAF_TYPE*/ {"Umbrellaleaf.reanim", "anim_idle", 0, 0, 65.0f, 2.0f, 0.0f, 0.0f},
    /* MARIGOLD_TYPE    */ {"marigold.reanim", "anim_idle", 0, 0, 74.0f, 2.0f, 0.0f, 0.0f},
    /* MELONPULT_TYPE   */ {"Melonpult.reanim", "anim_idle", 0, 0, 90.0f, 2.0f, 0.0f, 0.0f},
};

reanim::SdlReanimRenderer* gRenderer = nullptr;
bool gReady = false;
bool gInitTried = false;

// Pumpkin damage look: the shell keeps one reanim/skin for all hp stages;
// the current shell's hp is posted here by Pumpkin::display right before it
// draws (game plants are drawn one at a time, so a stash is enough).
// <0 = no pumpkin context (almanac/demo callers): always the intact look.
int gPumpkinHp = -1;
int gPumpkinHpMax = 0;

// Kernel-pult loaded-ammo swap (same post-per-draw convention): the 2Pak
// Cornpult doc leaves BOTH the kernal and butter tracks visible through the
// windup; the original instead hides the butter track at plant init
// (repvz Plant.cpp:432-435) and unhides it only for a lob that rolled
// butter, hiding the kernal in exchange (Plant.cpp:784-792). PultBase posts
// the armed state; isHidden() turns it into the track swap.
bool gKernelPultButterArmed = false;

std::string lowerOf(std::string s) {
    for (char& c : s) c = (char)tolower((unsigned char)c);
    return s;
}

// A sheet directory -> reanim frame range.
struct SegPick {
    bool mapped = false;
    const char* segment = nullptr; // segment name, or nullptr for explicit range
    int first = 0, last = 0;       // used when segment == nullptr
    bool clamp = false;            // one-shot: hold the last frame at sheet end
    bool idleCycle = false;        // idle state: drive frame from reanimClock + fidget plan
    SDL_Color tint{255, 255, 255, 255};  // multiplicative tint (extra hurt cue)
    const char* faceOverride = nullptr;  // replacement stem for the anim_face track
};

// The reanim files are one continuous timeline; the anim_* pseudo-tracks only
// mark windows inside it. Ranges below were verified against the XMLs:
//   PeaShooter/Snowpea: parts visible 8..59/61; windows anim_idle(4..7, parts
//     hidden there), anim_head_idle(8..24), anim_shooting(25..42/44),
//     anim_full_idle(43..59/45..61).
//   Sunflower: idle look = frames 3..32 (plain face), anim_producing 33..54
//     swaps in the *_producing face, anim_watering 55..76, anim_blink 0..2.
//   Wallnut: anim_idle = frame 0 (static body); blink windows 1..86. 2Pak quirk:
//     anim_face (the BODY) is f=-1 across the whole blink window, where only
//     eye/lid patch layers draw; buildIdlePlan un-orphans the body there.
//   PotatoMine: 0..319 underground loop (body/stem bob low), anim_rise
//     320..326, anim_armed 327..338, mashed pose = frame 357.
//   CherryBomb: anim_explode 0..15, anim_idle 16..26.
//   Chomper: anim_idle 0..99, anim_bite 100..122, anim_chew 123..139,
//     anim_swallow 140..186 (then watering). The chew loop keeps the
//     zombie_arm/head parts sticking out of the mouth; swallow gulps them in.
//   Puffshroom: anim_idle 0..12, anim_shooting 13..26, anim_sleep 27..43.
SegPick pickSegment(int type, int dir) {
    switch (type) {
    case PEASHOOTER_TYPE:
    case SNOWPEA_TYPE: {
        int sheet = type == PEASHOOTER_TYPE ? PEASHOOTER_SHEET_DIRECTORY
                                            : SNOWPEA_SHEET_DIRECTORY;
        int attack = type == PEASHOOTER_TYPE ? PEASHOOTER_ATTACK_DIRECTORY
                                             : SNOWPEA_ATTACK_DIRECTORY;
        if (dir == sheet) return {true, "anim_full_idle", 0, 0, false, true};
        if (dir == attack) return {true, "anim_shooting", 0, 0, false};
        return {};
    }
    case SUNFLOWER_TYPE:
        if (dir == SUNFLOWER_SHEET_DIRECTORY) return {true, nullptr, 3, 32, false, true};
        if (dir == SUNFLOWER_HAPPY_DIRECTORY) return {true, "anim_producing", 0, 0, false};
        return {};
    case WALLNUT_TYPE: {
        // All 5 damage stages play the anim_idle body with the idle fidget
        // cycle (blink_twice/blink_thrice/twitch); the cracked looks come from
        // swapping the anim_face track's image (same mechanism as the
        // cone/bucket degrade in reanim_zombie.cpp): wallnut_cracked1/2.png
        // ship with the reanim part set.
        int stage = dir - WALLNUT_1_DIRECTORY; // 0 = unharmed .. 4 = nearly dead
        if (stage < 0 || stage > 4) return {};
        SegPick pick{true, "anim_idle", 0, 0, false, true};
        if (stage == 2)
            pick.faceOverride = "wallnut_cracked1";
        else if (stage >= 3) {
            pick.faceOverride = "wallnut_cracked2";
            // slight extra darkening on the last stages, on top of the crack
            int v = stage == 3 ? 240 : 220;
            pick.tint = SDL_Color{(Uint8)v, (Uint8)v, (Uint8)v, 255};
        }
        return pick;
    }
    case CHERRYBOMB_TYPE:
        if (dir == CHERRYBOMB_SHEET_DIRECTORY) return {true, "anim_idle", 0, 0, false};
        if (dir == CHERRYBOMB_EXPLOSION_DIRECTORY) return {true, "anim_explode", 0, 0, true};
        return {};
    case POTATOMINE_TYPE:
        if (dir == POTATOMINE_UNARMED_DIRECTORY) return {true, nullptr, 0, 319, false};
        if (dir == POTATOMINE_RECOVER_DIRECTORY) return {true, "anim_rise", 0, 0, true};
        if (dir == POTATOMINE_IDLE_DIRECTORY) return {true, "anim_armed", 0, 0, false};
        // attack + explosion ("SPUDOW!"): no reanim counterpart (frames 357+
        // of PotatoMine.reanim are empty) -> fall back to the sprite sheets.
        return {};
    case REPEATER_TYPE:
        if (dir == PEASHOOTER_SHEET_DIRECTORY) return {true, "anim_full_idle", 0, 0, false, true};
        if (dir == PEASHOOTER_ATTACK_DIRECTORY) return {true, "anim_shooting", 0, 0, false};
        return {};
    case CHOMPER_TYPE:
        if (dir == CHOMPER_SHEET_DIRECTORY) return {true, "anim_idle", 0, 0, false};
        if (dir == CHOMPER_ATTACK_DIRECTORY) return {true, "anim_bite", 0, 0, true};
        if (dir == CHOMPER_CHEW_DIRECTORY) return {true, "anim_chew", 0, 0, false};
        if (dir == CHOMPER_SWALLOW_DIRECTORY) return {true, "anim_swallow", 0, 0, true};
        return {};
    case PUFFSHROOM_TYPE:
        if (dir == PUFFSHROOM_SHEET_DIRECTORY) return {true, "anim_idle", 0, 0, false};
        if (dir == PUFFSHROOM_ATTACK_DIRECTORY) return {true, "anim_shooting", 0, 0, false};
        if (dir == PUFFSHROOM_SLEEP_DIRECTORY) return {true, "anim_sleep", 0, 0, false};
        return {};
    // Night mushrooms: segment names verified against the 2Pak XMLs. One-shot
    // segments (grow, land, explode, the scared duck/raise) clamp on the last
    // frame; the game state machine switches off them by its own tick counts.
    case SUNSHROOM_TYPE:
        if (dir == SUNSHROOM_SHEET_DIRECTORY) return {true, "anim_idle", 0, 0, false};
        if (dir == SUNSHROOM_HAPPY_DIRECTORY) return {true, "anim_producing", 0, 0, false};
        if (dir == SUNSHROOM_GROW_DIRECTORY) return {true, "anim_grow", 0, 0, true};
        if (dir == SUNSHROOM_BIG_DIRECTORY) return {true, "anim_bigidle", 0, 0, false};
        if (dir == SUNSHROOM_BIG_HAPPY_DIRECTORY) return {true, "anim_bigproducing", 0, 0, false};
        if (dir == SUNSHROOM_SLEEP_DIRECTORY) return {true, "anim_sleep", 0, 0, false};
        return {};
    case FUMESHROOM_TYPE:
        if (dir == FUMESHROOM_SHEET_DIRECTORY) return {true, "anim_idle", 0, 0, false};
        if (dir == FUMESHROOM_ATTACK_DIRECTORY) return {true, "anim_shooting", 0, 0, false};
        if (dir == FUMESHROOM_SLEEP_DIRECTORY) return {true, "anim_sleep", 0, 0, false};
        return {};
    case GRAVEBUSTER_TYPE:
        if (dir == GRAVEBUSTER_IDLE_DIRECTORY) return {true, "anim_idle", 0, 0, false};
        if (dir == GRAVEBUSTER_LANDING_DIRECTORY) return {true, "anim_land", 0, 0, true};
        if (dir == GRAVEBUSTER_EATING_DIRECTORY) return {true, "anim_busting", 0, 0, false};
        return {};
    case HYPNOSHROOM_TYPE:
        if (dir == HYPNOSHROOM_SHEET_DIRECTORY) return {true, "anim_idle", 0, 0, false};
        if (dir == HYPNOSHROOM_SLEEP_DIRECTORY) return {true, "anim_sleep", 0, 0, false};
        return {};
    case SCAREDYSHROOM_TYPE:
        if (dir == SCAREDYSHROOM_SHEET_DIRECTORY) return {true, "anim_idle", 0, 0, false};
        if (dir == SCAREDYSHROOM_ATTACK_DIRECTORY) return {true, "anim_shooting", 0, 0, false};
        if (dir == SCAREDYSHROOM_SCARED_DIRECTORY) return {true, "anim_scared", 0, 0, true};
        if (dir == SCAREDYSHROOM_COWER_DIRECTORY) return {true, "anim_scaredidle", 0, 0, false};
        if (dir == SCAREDYSHROOM_RAISE_DIRECTORY) return {true, "anim_grow", 0, 0, true};
        if (dir == SCAREDYSHROOM_SLEEP_DIRECTORY) return {true, "anim_sleep", 0, 0, false};
        return {};
    case ICESHROOM_TYPE:
        // The blast is the icetrap overlay the plant draws itself; the body
        // simply holds the idle pose while the flash plays.
        if (dir == ICESHROOM_SHEET_DIRECTORY) return {true, "anim_idle", 0, 0, false};
        if (dir == ICESHROOM_EXPLODE_DIRECTORY) return {true, "anim_idle", 0, 0, true};
        if (dir == ICESHROOM_SLEEP_DIRECTORY) return {true, "anim_sleep", 0, 0, false};
        return {};
    case DOOMSHROOM_TYPE:
        if (dir == DOOMSHROOM_SHEET_DIRECTORY) return {true, "anim_idle", 0, 0, false};
        if (dir == DOOMSHROOM_EXPLODE_DIRECTORY) return {true, "anim_explode", 0, 0, true};
        if (dir == DOOMSHROOM_SLEEP_DIRECTORY) return {true, "anim_sleep", 0, 0, false};
        return {};
    // Fog plants (area 4): SeaShroom/Cactus/Blover/Starfruit/Pumpkin/Magnet
    // idle labels carry no f markers, so idle refs are explicit windows.
    case SEASHROOM_TYPE:
        if (dir == SEASHROOM_SHEET_DIRECTORY) return {true, nullptr, 0, 17, false};
        if (dir == SEASHROOM_ATTACK_DIRECTORY) return {true, "anim_shooting", 0, 0, false};
        if (dir == SEASHROOM_SLEEP_DIRECTORY) return {true, "anim_sleep", 0, 0, false};
        return {};
    case PLANTERN_TYPE:
        if (dir == PLANTERN_SHEET_DIRECTORY) return {true, nullptr, 0, 90, false};
        return {};
    case CACTUS_TYPE:
        if (dir == CACTUS_SHEET_DIRECTORY) return {true, nullptr, 0, 33, false};
        if (dir == CACTUS_ATTACK_DIRECTORY) return {true, "anim_shooting", 0, 0, false};
        if (dir == CACTUS_RISE_DIRECTORY) return {true, "anim_rise", 0, 0, true};
        if (dir == CACTUS_IDLE_HIGH_DIRECTORY) return {true, "anim_idlehigh", 0, 0, false};
        if (dir == CACTUS_ATTACK_HIGH_DIRECTORY) return {true, "anim_shootinghigh", 0, 0, false};
        if (dir == CACTUS_LOWER_DIRECTORY) return {true, "anim_lower", 0, 0, true};
        return {};
    case BLOVER_TYPE:
        if (dir == BLOVER_SHEET_DIRECTORY) return {true, nullptr, 0, 28, false};
        if (dir == BLOVER_WIND_DIRECTORY) return {true, nullptr, 53, 81, true};
        return {};
    case SPLITPEA_TYPE:
        // Rendered by drawSplitPea before pickSegment resolution (the skin
        // composites leaves + both heads out of three staggered windows).
        return {};
    case STARFRUIT_TYPE:
        if (dir == STARFRUIT_SHEET_DIRECTORY) return {true, nullptr, 0, 24, false};
        if (dir == STARFRUIT_ATTACK_DIRECTORY) return {true, "anim_shoot", 0, 0, false};
        return {};
    case PUMPKIN_TYPE:
        if (dir == PUMPKIN_SHEET_DIRECTORY) return {true, nullptr, 0, 24, false};
        return {};
    case MAGNETSHROOM_TYPE:
        if (dir == MAGNETSHROOM_SHEET_DIRECTORY) return {true, nullptr, 0, 44, false};
        if (dir == MAGNETSHROOM_SUCK_DIRECTORY) return {true, "anim_shooting", 0, 0, true};
        if (dir == MAGNETSHROOM_CHARGE_DIRECTORY) return {true, "anim_nonactive_idle2", 0, 0, false};
        if (dir == MAGNETSHROOM_SLEEP_DIRECTORY) return {true, "anim_sleep", 0, 0, false};
        return {};
    // Pool plants (area 3). Segment windows measured against the 2Pak XMLs:
    //   Squash: idle 0..29, lookleft 30..43, lookright 44..57, jumpup 58..73,
    //     jumpdown 74..135 (one-shots clamp on their last pose).
    //   ThreePeater: no anim_idle marker; the 4..21 sway IS the all-heads
    //     idle, 22..33 the shooting flare (heads 1/3 rest on keyframe holds).
    //   Tanglekelp: idle 0..76, grab 77..91.
    //   Jalapeno: idle 0..5, explode 6..13 (held while the fuse runs).
    //   Caltrop (Spikeweed): idle 0..108, attack 109..117.
    //   Torchwood: idle 4..14.  Tallnut: anim_idle marker empty; body rests
    //     static 86..121 (blink_thrice is 21..84), idle = 86..120.
    case LILYPAD_TYPE:
        if (dir == LILYPAD_SHEET_DIRECTORY) return {true, "anim_idle", 0, 0, false};
        return {};
    case SQUASH_TYPE:
        if (dir == SQUASH_SHEET_DIRECTORY) return {true, "anim_idle", 0, 0, false};
        if (dir == SQUASH_LOOK_DIRECTORY) return {true, "anim_lookright", 0, 0, true};
        if (dir == SQUASH_JUMPUP_DIRECTORY) return {true, "anim_jumpup", 0, 0, true};
        if (dir == SQUASH_JUMPDOWN_DIRECTORY) return {true, "anim_jumpdown", 0, 0, true};
        return {};
    case THREEPEATER_TYPE:
        if (dir == THREEPEATER_SHEET_DIRECTORY) return {true, nullptr, 4, 21, false};
        if (dir == THREEPEATER_ATTACK_DIRECTORY) return {true, nullptr, 22, 33, false};
        return {};
    case TANGLEKELP_TYPE:
        if (dir == TANGLEKELP_SHEET_DIRECTORY) return {true, "anim_idle", 0, 0, false};
        if (dir == TANGLEKELP_GRAB_DIRECTORY) return {true, "anim_grab", 0, 0, true};
        return {};
    case JALAPENO_TYPE:
        if (dir == JALAPENO_SHEET_DIRECTORY) return {true, "anim_idle", 0, 0, false};
        if (dir == JALAPENO_EXPLODE_DIRECTORY) return {true, "anim_explode", 0, 0, true};
        return {};
    case SPIKEWEED_TYPE:
        if (dir == SPIKEWEED_SHEET_DIRECTORY) return {true, "anim_idle", 0, 0, false};
        if (dir == SPIKEWEED_ATTACK_DIRECTORY) return {true, "anim_attack", 0, 0, false};
        return {};
    case TORCHWOOD_TYPE:
        if (dir == TORCHWOOD_SHEET_DIRECTORY) return {true, "anim_idle", 0, 0, false};
        return {};
    case TALLNUT_TYPE:
        if (dir == TALLNUT_SHEET_DIRECTORY) return {true, nullptr, 86, 120, false};
        return {};
    // Roof plants (area 5). Segment windows verified against the 2Pak XMLs
    // (fps 12): Cabbagepult idle 0..75 / shooting 76..104; Cornpult idle
    // 5..17 / shooting 18..60; Melonpult idle 0..16 / shooting 16..53;
    // Pot idle 0..19; Coffeebean idle 0..82 / crumble 83..111; Garlic idle
    // 138..234 (degrade reads as progressive tint, one body mesh ships);
    // Umbrellaleaf idle 0..37 / block 38..53; Marigold idle 0..35 /
    // producing 36..50.
    case CABBAGEPULT_TYPE:
        if (dir == CABBAGEPULT_DIRECTORY) return {true, "anim_idle", 0, 0, false};
        if (dir == CABBAGEPULT_ATTACK_DIRECTORY) return {true, "anim_shooting", 0, 0, false};
        return {};
    case KERNELPULT_TYPE:
        if (dir == KERNELPULT_DIRECTORY) return {true, "anim_idle", 0, 0, false};
        if (dir == KERNELPULT_ATTACK_DIRECTORY) return {true, "anim_shooting", 0, 0, false};
        return {};
    case MELONPULT_TYPE:
        if (dir == MELONPULT_DIRECTORY) return {true, "anim_idle", 0, 0, false};
        if (dir == MELONPULT_ATTACK_DIRECTORY) return {true, "anim_shooting", 0, 0, false};
        return {};
    case FLOWERPOT_TYPE:
        if (dir == FLOWERPOT_DIRECTORY) return {true, "anim_idle", 0, 0, false};
        return {};
    case COFFEEBEAN_TYPE:
        if (dir == COFFEEBEAN_DIRECTORY) return {true, "anim_idle", 0, 0, false};
        if (dir == COFFEEBEAN_CRUMBLE_DIRECTORY) return {true, "anim_crumble", 0, 0, true};
        return {};
    case GARLIC_TYPE: {
        if (dir == GARLIC_DIRECTORY) return {true, "anim_idle", 0, 0, false};
        if (dir == GARLIC_HURT1_DIRECTORY)
            return {true, "anim_idle", 0, 0, false, false, SDL_Color{225, 235, 205, 255}};
        if (dir == GARLIC_HURT2_DIRECTORY)
            return {true, "anim_idle", 0, 0, false, false, SDL_Color{190, 215, 170, 255}};
        return {};
    }
    case UMBRELLALEAF_TYPE:
        if (dir == UMBRELLALEAF_DIRECTORY) return {true, "anim_idle", 0, 0, false};
        if (dir == UMBRELLALEAF_BLOCK_DIRECTORY) return {true, "anim_block", 0, 0, true};
        return {};
    case MARIGOLD_TYPE:
        if (dir == MARIGOLD_DIRECTORY) return {true, "anim_idle", 0, 0, false};
        if (dir == MARIGOLD_PRODUCE_DIRECTORY) return {true, "anim_producing", 0, 0, false};
        return {};
    default:
        // Area 3-5 stubs: one packet icon per plant (draw/plant_icon_dir.hpp),
        // all idling on anim_idle.
        if (type >= LILYPAD_TYPE && type < PLANT_COUNT &&
            dir == (int)PLANT_ICON_DIR[type])
            return {true, "anim_idle", 0, 0, false};
        return {};
    }
}

// ---------------------------------------------------------------------------
// Idle lifecycle: the original plants never loop a frozen idle — after each
// idle pass they sometimes slip in a blink/fidget clip. The plan is resolved
// once at load; at draw time the frame is chosen from the plant's free-running
// reanimClock (12 fps reanim time), so idle cadence no longer follows the
// sprite-sheet frame counter. Fidget choice is hashed from the plant's cell +
// cycle index: deterministic per plant, but no two plants blink in sync.
//
// Data found in the XMLs (verified against the pseudo-track visibility
// windows, fps = 12 everywhere):
//   PeaShooter/Snowpea: idle = anim_full_idle (17f); no blink frames exist in
//     the timeline (anim_idle = 4..7 has every part hidden), but closed-eye
//     art ships in the part set (eye squint during shooting): blink is a
//     synthetic 4-frame clip appended to the doc — idle pose with the eye
//     parts swapped to the closed image on the two middle frames.
//   Sunflower: idle = 3..32, anim_producing = 33..54 (sun spawn, handled by
//     the happy sheet state), anim_blink = 0..2 is likewise empty -> same
//     synthetic blink, using sunflower_eye2.
//   Wallnut: native fidgets: anim_blink_twice 1..20, anim_blink_thrice
//     21..81, anim_blink_twitch 82..86; idle proper = static frame 0. The
//     export hides the body over 1..86; the WALLNUT case below un-hides it
//     (retail draws the fidget as an overlay on the still-idling body,
//     repvz Plant.cpp:2929-3046).
//   PotatoMine (underground bob 0..319, armed light-blink 327..338) and
//   CherryBomb (one-shot) are lively enough as-is: no fidget plan.
// ---------------------------------------------------------------------------

const int BLINK_LEN = 4; // [open, closed, closed, open] ~= 0.33 s at 12 fps
// Sunflower closed-eye strips are authored at frame 60 (inside anim_watering,
// the one window where they are visible with both strips on the head).
const int SUNFLOWER_STRIP_SRC = 60;

// Track index of the sunflower head part (image stem contains "head"), the
// -1 when none. The eye strips in Sunflower.reanim are authored head-mounted.
int sunflowerHeadTrack(const reanim::ReanimDoc& doc, int frame) {
    for (int i = 0; i < (int)doc.tracks.size(); i++) {
        const reanim::Track& tr = doc.tracks[i];
        if (!tr.hasAnyImage || frame >= (int)tr.frames.size()) continue;
        const reanim::Transform& t = tr.frames[frame];
        if (t.f >= 0.0f && t.image >= 0 &&
            reanim::ImageFileStem(doc.images[t.image]).find("head") !=
                std::string::npos)
            return i;
    }
    return -1;
}

struct IdleClip {
    int first, last;
    float weight; // relative, over the clips of one plan
};

struct IdlePlan {
    bool valid = false;
    int idleFirst = 0, idleLast = 0; // main idle loop (reanim frames)
    int idleHold = 0;                // frames one idle pass lasts (>= idle len)
    int slotLen = 0;                 // frames reserved for a fidget after each pass
    float pFidget = 0.0f;            // chance a cycle plays a fidget at all
    IdleClip clips[3];
    int nClips = 0;
};

IdlePlan gIdlePlans[SKIN_COUNT];

uint32_t mixHash(uint32_t h) {
    h ^= h >> 16;
    h *= 0x7feb352dU;
    h ^= h >> 15;
    h *= 0x846ca68bU;
    h ^= h >> 16;
    return h;
}

// Re-anchor a part pose `part` (authored to ride a parent transform `par0`)
// onto the parent at pose `par1`: M' = M(par1) * inv(M(par0)) * M(part),
// decomposed back to x/y/sx/sy/kx/ky (matrix convention as in reanim.hpp).
reanim::Transform reanchorOnParent(const reanim::Transform& part,
                                   const reanim::Transform& par0,
                                   const reanim::Transform& par1) {
    static const float D2R = 3.14159265358979323846f / 180.0f;
    static const float R2D = 180.0f / 3.14159265358979323846f;
    auto lin = [](const reanim::Transform& t, float& A, float& B, float& C,
                     float& D) {
        float kxr = t.kx * D2R, kyr = -t.ky * D2R;
        A = cosf(kxr) * t.sx, B = sinf(kxr) * t.sx;
        C = sinf(kyr) * t.sy, D = cosf(kyr) * t.sy;
    };
    float A0, B0, C0, D0, A1, B1, C1, D1, A2, B2, C2, D2;
    lin(part, A0, B0, C0, D0);
    lin(par0, A1, B1, C1, D1);
    lin(par1, A2, B2, C2, D2);
    float det = A1 * D1 - B1 * C1;
    if (fabsf(det) < 1e-6f) return part;
    // rel = inv(M(par0)) * M(part)
    float iA = D1 / det, iB = -B1 / det, iC = -C1 / det, iD = A1 / det;
    float itx = (C1 * par0.y - D1 * par0.x) / det;
    float ity = (B1 * par0.x - A1 * par0.y) / det;
    float rA = iA * A0 + iC * B0, rB = iB * A0 + iD * B0;
    float rC = iA * C0 + iC * D0, rD = iB * C0 + iD * D0;
    float rtx = iA * part.x + iC * part.y + itx;
    float rty = iB * part.x + iD * part.y + ity;
    // M' = M(par1) * rel
    reanim::Transform out;
    float nA = A2 * rA + C2 * rB, nB = B2 * rA + D2 * rB;
    float nC = A2 * rC + C2 * rD, nD = B2 * rC + D2 * rD;
    out.x = A2 * rtx + C2 * rty + par1.x;
    out.y = B2 * rtx + D2 * rty + par1.y;
    out.sx = hypotf(nA, nB);
    out.sy = hypotf(nC, nD);
    out.kx = atan2f(nB, nA) * R2D;
    out.ky = -atan2f(nC, nD) * R2D;
    out.a = part.a;
    out.f = 0.0f;
    return out;
}

// Appends a synthetic blink clip to the skin's doc: BLINK_LEN frames copying
// the poses of the idle loop's TAIL (srcFirst..srcFirst+BLINK_LEN-1 — the
// frames the fidget slot stands in for, keeping the cycle seamless), with the
// visible eye parts (image stem containing "_eye") swapped to the closed-eye
// image (or hidden when none exists) on the two middle frames. Only this
// module's in-memory ReanimDoc is touched.
//
// Sunflower ("sunflower_eye2") is different: its closed-eye art is a wide
// strip carrying BOTH eyes, authored on two overlay tracks at the watering
// window — the open-eye pivots can't carry it (the per-eye image swap lands
// the strip doubled and ~2x too big, the "nháy mắt lệch hẳn" look). There the
// middle frames instead hide the open-eye tracks and show the strip tracks
// with their authored watering pose re-anchored onto the head track's pose.
// Returns the clip's first frame index, or -1 when not applicable.
int appendBlinkClip(PlantSkin& skin, int srcFirst, const char* closedStem) {
    reanim::ReanimDoc& doc = skin.doc;
    int n = doc.frameCount;
    if (srcFirst < 0 || srcFirst + BLINK_LEN > n) return -1;
    int closedId = -1;
    for (int i = 0; i < (int)doc.images.size(); i++)
        if (reanim::ImageFileStem(doc.images[i]) == closedStem) {
            closedId = i;
            break;
        }
    const bool stripMode = std::string(closedStem) == "sunflower_eye2" &&
                           closedId >= 0 && n > SUNFLOWER_STRIP_SRC;
    if (stripMode && sunflowerHeadTrack(doc, SUNFLOWER_STRIP_SRC) < 0) return -1;
    int headIdx = stripMode ? sunflowerHeadTrack(doc, SUNFLOWER_STRIP_SRC) : -1;
    bool anyEye = false;
    for (auto& tr : doc.tracks) {
        if ((int)tr.frames.size() != n) return -1; // uneven track layout: bail
        bool eye = false, strip = false;
        for (int k = 0; k < BLINK_LEN; k++) {
            reanim::Transform base = tr.frames[srcFirst + k];
            if (tr.hasAnyImage && base.image >= 0) {
                std::string stem = reanim::ImageFileStem(doc.images[base.image]);
                if (stripMode && stem == closedStem)
                    strip = true;
                else if (base.f >= 0.0f && stem.find("_eye") != std::string::npos)
                    eye = true;
            }
        }
        if (eye) anyEye = true;
        if (!stripMode) strip = false;
        if (strip) eye = false;
        for (int k = 0; k < BLINK_LEN; k++) {
            reanim::Transform t = tr.frames[srcFirst + k];
            bool mid = (k == 1 || k == 2);
            if (strip) {
                if (mid) {
                    t = reanchorOnParent(tr.frames[SUNFLOWER_STRIP_SRC],
                                         doc.tracks[headIdx].frames[SUNFLOWER_STRIP_SRC],
                                         doc.tracks[headIdx].frames[srcFirst + k]);
                    t.image = closedId;
                } else {
                    t.f = -1.0f;
                }
            } else if (eye && mid) {
                if (stripMode || closedId < 0)
                    t.f = -1.0f;
                else
                    t.image = closedId;
            }
            tr.frames.push_back(t);
        }
    }
    if (!anyEye) { // no open-eye part in the source frames: undo, no blink
        for (auto& tr : doc.tracks) tr.frames.resize(n);
        return -1;
    }
    doc.frameCount = n + BLINK_LEN;
    return n;
}

bool findSeg(const PlantSkin& skin, const char* name, int& first, int& last) {
    auto it = skin.doc.segments.find(name);
    if (it == skin.doc.segments.end()) return false;
    first = it->second.first;
    last = it->second.last;
    return last >= first;
}

// Defined next to resolveRange below; trims a looping window's loop-seam
// duplicate end (see loopPlayLast there).
int loopPlayLast(const PlantSkin& skin, int first, int last);

void addClip(IdlePlan& plan, const PlantSkin& skin, const char* seg, float w) {
    int a, b;
    if (plan.nClips >= 3 || !findSeg(skin, seg, a, b)) return;
    plan.clips[plan.nClips++] = {a, loopPlayLast(skin, a, b), w};
}

void buildIdlePlan(int type, PlantSkin& skin) {
    IdlePlan& plan = gIdlePlans[type];
    switch (type) {
    case PEASHOOTER_TYPE:
    case SNOWPEA_TYPE:
    case REPEATER_TYPE: {
        if (!findSeg(skin, "anim_full_idle", plan.idleFirst, plan.idleLast)) return;
        plan.idleLast = loopPlayLast(skin, plan.idleFirst, plan.idleLast);
        int blink = appendBlinkClip(skin, plan.idleLast - BLINK_LEN + 1,
                                    type == SNOWPEA_TYPE ? "snowpea_eye1"
                                                         : "peashooter_eye2");
        if (blink < 0) return;
        plan.idleHold = plan.idleLast - plan.idleFirst + 1;
        plan.slotLen = BLINK_LEN;
        plan.pFidget = 0.30f; // ~= one blink per ~6 s of idle
        int blinkLast = loopPlayLast(skin, blink, blink + BLINK_LEN - 1);
        plan.clips[0] = {blink, blinkLast, 1.0f};
        plan.nClips = 1;
        plan.valid = true;
        break;
    }
    case SUNFLOWER_TYPE: {
        plan.idleFirst = 3; // plain-face idle (matches pickSegment's range)
        plan.idleLast = loopPlayLast(skin, plan.idleFirst, 32);
        int blink = appendBlinkClip(skin, plan.idleLast - BLINK_LEN + 1,
                                    "sunflower_eye2");
        if (blink < 0) return;
        plan.idleHold = plan.idleLast - plan.idleFirst + 1;
        plan.slotLen = BLINK_LEN;
        plan.pFidget = 0.30f;
        int blinkLast = loopPlayLast(skin, blink, blink + BLINK_LEN - 1);
        plan.clips[0] = {blink, blinkLast, 1.0f};
        plan.nClips = 1;
        plan.valid = true;
        break;
    }
    case WALLNUT_TYPE: {
        // 2Pak export quirk: anim_face (the body) is f=-1 through the whole
        // fidget window 1..86, where only eye/lid patch layers draw — playing
        // a fidget window standalone shows eyes-only (blink_thrice's gaps
        // shrink to three tiny specks). Retail runs the blink as a SECOND
        // reanim overlaid on the anim_idle body instance (repvz
        // Plant.cpp:2929-3046 AttachBlinkAnim); same look here by holding the
        // body's static frame-0 pose through the window — the idle proper is
        // one frozen frame, so no authored motion is lost.
        int bFirst, bLast, tFirst, tLast;
        if (findSeg(skin, "anim_blink_twice", bFirst, bLast) && bFirst >= 1 &&
            findSeg(skin, "anim_blink_twitch", tFirst, tLast))
            for (auto& tr : skin.doc.tracks)
                if (tr.name == "anim_face" && (int)tr.frames.size() > tLast) {
                    reanim::Transform pose = tr.frames[bFirst - 1]; // idle pose
                    pose.f = 0.0f;
                    for (int f = bFirst; f <= tLast; f++) tr.frames[f] = pose;
                    break;
                }
        if (!findSeg(skin, "anim_idle", plan.idleFirst, plan.idleLast)) return;
        plan.idleLast = loopPlayLast(skin, plan.idleFirst, plan.idleLast);
        plan.idleHold = 24; // sit still ~2 s before a possible fidget
        plan.slotLen = 61;  // fits the longest fidget (blink_thrice)
        plan.pFidget = 0.55f;
        addClip(plan, skin, "anim_blink_twice", 0.45f);
        addClip(plan, skin, "anim_blink_thrice", 0.36f);
        addClip(plan, skin, "anim_blink_twitch", 0.19f);
        plan.valid = plan.nClips > 0;
        break;
    }
    default:
        break;
    }
    // Seam: the plain-idle branch of evalIdleCycle plays the idle loop with
    // period idleLen inside a cycle of idleHold+slotLen ticks. If the cycle is
    // not a whole number of idle passes the playhead restarts mid-pass at
    // every non-fidget cycle border (e.g. peashooter 20-tick cycle vs 16-frame
    // loop: a 4-frame jump each cycle — the visible "khựng"). Round the hold
    // UP so the cycle is always a whole number of idle passes; fidget cycles
    // stay seamless because the blink clip carries the tail poses of the pass
    // it replaces.
    if (plan.valid && plan.slotLen > 0) {
        int idleLen = plan.idleLast - plan.idleFirst + 1;
        if (idleLen > 1) {
            int cycle = plan.idleHold + plan.slotLen;
            int passes = (cycle + idleLen - 1) / idleLen;
            plan.idleHold = passes * idleLen - plan.slotLen;
        }
    }
    if (plan.valid)
        SDL_Log("reanimplant: %s idle %d..%d hold %d + slot %d, %d fidget clip(s)",
                skin.reanimFile, plan.idleFirst, plan.idleLast, plan.idleHold,
                plan.slotLen, plan.nClips);
}

// Pick this cycle's frame from the clock; sets the interpolation wrap range.
float evalIdleCycle(const IdlePlan& plan, float clock, uint32_t seed,
                    int& wrapFirst, int& wrapLast) {
    int idleLen = plan.idleLast - plan.idleFirst + 1;
    int hold = plan.idleHold > 0 ? plan.idleHold : idleLen;
    int cycleLen = hold + plan.slotLen;
    uint32_t cycle = (uint32_t)floorf(clock / (float)cycleLen);
    float phase = clock - (float)cycle * (float)cycleLen;
    if (plan.slotLen > 0 && phase >= (float)hold) {
        float sub = phase - (float)hold;
        uint32_t h = mixHash(seed ^ mixHash(cycle));
        float rAny = (float)(h & 0xFFFFu) / 65536.0f;
        if (rAny < plan.pFidget && plan.nClips > 0) {
            float rPick = (float)((h >> 16) & 0xFFFFu) / 65536.0f;
            float acc = 0.0f;
            for (int i = 0; i < plan.nClips; i++) {
                acc += plan.clips[i].weight;
                if (rPick <= acc || i == plan.nClips - 1) {
                    int clen = plan.clips[i].last - plan.clips[i].first + 1;
                    if (sub < (float)clen) {
                        wrapFirst = plan.clips[i].first;
                        wrapLast = plan.clips[i].last;
                        return (float)plan.clips[i].first + sub;
                    }
                    break; // fidget shorter than the slot: fill with idle
                }
            }
        }
    }
    wrapFirst = plan.idleFirst;
    wrapLast = plan.idleLast;
    return (float)plan.idleFirst + fmodf(phase, (float)idleLen);
}

// --- Repeater variant --------------------------------------------------------
// The Repeater reuses the PeaShooter body reanim; its fiercer eyebrows and big
// back leaf ship as same-size replacements for the two baked-in parts
// (repeater_eyebrows.png 34x14 == peashooter_leaf3.png, repeater_leaf.png 41x54
// == peashooter_leaf2.png). drawPlant swaps the images in place via
// DrawOptions::imageOverrides so they inherit the authored pivots, the z-order
// of the source slots (the leaf track precedes the head track, so the leaf
// sits BEHIND the head and connects to the stem), and the bob/recoil
// keyframes — an after-the-fact overlay anchored to a different track cannot
// reproduce any of that.
bool resolveRange(const PlantSkin& skin, const SegPick& pick, int& first, int& last) {
    if (!pick.mapped) return false;
    if (pick.segment) {
        auto it = skin.doc.segments.find(pick.segment);
        if (it == skin.doc.segments.end()) return false;
        first = it->second.first;
        last = it->second.last;
    } else {
        first = pick.first;
        last = pick.last;
    }
    return last >= first;
}

// Play range end for a LOOPING window. 2Pak reanim exports close every loop
// with a copy of its first pose (the loop seam); feeding that duplicate as a
// full playback frame shows the first pose twice in a row at each wrap — the
// visible "khựng" at the loop point. Loops therefore play [first, seamLast-1]
// and reach the duplicate only as the wrap-blend target (SampleTrack).
// One-shot (clamped) states keep the raw last frame.
int loopPlayLast(const PlantSkin& skin, int first, int last) {
    return reanim::SegmentEndsOnLoopSeam(skin.doc, first, last) ? last - 1 : last;
}

// Absolute reanim frame a sheet state maps to (drawPlant's sync-mode math;
// clock-driven idle states resolve to their plan's main idle range). Shared
// by drawPlant and the debug snapshot trackStatuses(). sframe is the
// FRACTIONAL sheet frame, so consecutive game ticks sample consecutive
// fractional reanim frames and SampleTrack interpolates the pose between
// authored keyframes (integer sframe would step the playhead at sheet fps,
// the old "khựng" staircase).
bool resolveSampleFrame(int plantType, int sheetDir, float sframe, int nSheet,
                        float& absFrame) {
    if (plantType < 0 || plantType >= SKIN_COUNT) return false;
    PlantSkin& skin = gSkins[plantType];
    if (!skin.loaded) return false;
    SegPick pick = pickSegment(plantType, sheetDir);
    int first, last;
    const IdlePlan& plan = gIdlePlans[plantType];
    if (pick.idleCycle && plan.valid) {
        first = plan.idleFirst;
        last = plan.idleLast; // already seam-trimmed by buildIdlePlan
    } else if (!resolveRange(skin, pick, first, last)) {
        return false;
    } else if (!pick.clamp) {
        last = loopPlayLast(skin, first, last); // mirrors drawPlant's loop play range
    }
    int segLen = last - first + 1;
    float local = nSheet > 0 ? (float)sframe * segLen / nSheet : 0.0f;
    if (local > (float)(segLen - 1))
        local = pick.clamp ? (float)(segLen - 1) : fmodf(local, (float)segLen);
    absFrame = (float)first + local;
    return true;
}

// Accessory tracks never drawn for plants: the ornament_* part images are
// fully transparent in this asset set, so hiding them also keeps them out of
// the placement bbox. Extra per-type hides: the 2Pak PeaShooter art bakes the
// repeater look into the shared body — a big back leaf ("leaf",
// peashooter_leaf2.png) plus an eyebrow tuft ("PeaShooter_eyebrow",
// peashooter_leaf3.png). The classic peashooter silhouette has a small back
// leaf but no tuft, so PEASHOOTER_TYPE hides only the eyebrow family by name
// pattern and drawPlant swaps the leaf slot's image for PeaShooter_leaf.png.
// REPEATER_TYPE instead keeps both tracks visible: drawPlant swaps
// their images for repeater_leaf/repeater_eyebrows via imageOverrides.
bool isHidden(int plantType, const std::string& nameLower) {
    if (nameLower.find("ornament") != std::string::npos) return true;
    if (plantType == PEASHOOTER_TYPE)
        return nameLower.find("eyebrow") != std::string::npos;
    if (plantType == KERNELPULT_TYPE) {
        // Loaded-ammo swap (see gKernelPultButterArmed): while a butter lob
        // is armed the butter track carries the basket and the kernal is
        // hidden; at all other times the loaded (or reloading) ammo is the
        // kernal and the butter track stays off.
        if (nameLower == "cornpult_butter") return !gKernelPultButterArmed;
        if (nameLower == "cornpult_kernal") return gKernelPultButterArmed;
    }
    return false;
}

// Calibration hides: the leaf/eyebrow parts are decorations hanging off the
// pea-family body (the peashooter shows its own leaf, the repeater shows its
// swapped set); artH targets the body height, so the placement measure skips
// them for both PEASHOOTER_TYPE and REPEATER_TYPE — leaving them in would
// shrink the whole plant ~13% and recentre it.
bool isMeasureHidden(int plantType, const std::string& nameLower) {
    if (plantType == PEASHOOTER_TYPE || plantType == REPEATER_TYPE)
        return nameLower.find("leaf") != std::string::npos ||
               nameLower.find("eyebrow") != std::string::npos ||
               nameLower.find("ornament") != std::string::npos;
    // Plantern's light7 part is a 6.8x-scaled firefly swarm FX floating above
    // the plant; it stays drawn (it's the lamp glow) but must not size the
    // body placement.
    if (plantType == PLANTERN_TYPE && nameLower == "light7") return true;
    return isHidden(plantType, nameLower);
}

bool calibrate(int plantType, PlantSkin& skin);

// --- Split Pea composite -------------------------------------------------------
// Splitpea.reanim has no full-plant window: the 2Pak timeline staggers the
// three clusters (leaves 7..23, back head 24..53, front head 54..87). Compose
// the plant by drawing the doc three times at the cluster windows, hiding the
// other clusters' parts each pass. Sheet sync: nSheet sheet frames map into
// each cluster window; the attack sheet swaps both heads to their shooting
// windows.
int splitPeaCluster(const std::string& n) {
    if (n.rfind("splitpea_", 0) == 0) return 1; // back head (faces left)
    if (n == "backleaf1" || n == "backleaf2" || n == "stem" ||
        n == "frontleaf1" || n == "frontleaf2")
        return 2; // leaves
    return 0;     // front head
}

float splitPeaLocal(float sframe, int nSheet, int segLen) {
    float local = nSheet > 0 ? (float)sframe * segLen / nSheet : 0.0f;
    if (local > (float)(segLen - 1))
        local = fmodf(local, (float)segLen);
    return local;
}

bool drawSplitPea(PlantSkin& skin, int sheetDir, float sframe, int nSheet,
                  int cellX1, int cellX2, int footY, bool flash,
                  int* partsDrawn, float scaleMul,
                  const PlantPlacement* retailPlacement) {
    if (!skin.calibrated && !calibrate(SPLITPEA_TYPE, skin)) return false;
    float scale = retailPlacement ? retailPlacement->scale
                                  : skin.scale * scaleMul;
    float ox = retailPlacement
                   ? retailPlacement->x
                   : (cellX1 + cellX2) * 0.5f + skin.dx - scale * skin.refCX;
    float oy = retailPlacement
                   ? retailPlacement->y
                   : (float)footY - skin.padBottom + skin.dy - scale * skin.refBottom;
    bool attack = (sheetDir == SPLITPEA_ATTACK_DIRECTORY);

    // (cluster, window first/last) — back head first so the front faces hug it.
    int backFirst = attack ? 41 : 24, backLast = attack ? 53 : 40;
    int frontFirst = attack ? 71 : 54, frontLast = attack ? 87 : 70;
    struct Pass { int cluster, first, last; };
    Pass passes[3] = {
        {1, backFirst, backLast},
        {2, 7, 23},
        {0, frontFirst, frontLast},
    };
    int total = 0;
    for (const Pass& p : passes) {
        // cluster windows loop too: trim a loop-seam duplicate end like drawPlant
        int plast = loopPlayLast(skin, p.first, p.last);
        reanim::DrawOptions opt;
        opt.flash = flash;
        opt.loopWrapFirst = p.first;
        opt.loopWrapLast = plast;
        float frame = (float)p.first +
                      splitPeaLocal(sframe, nSheet, plast - p.first + 1);
        reanim::DrawStats st = gRenderer->Draw(
            skin.doc, frame, ox, oy, scale,
            [&](const std::string& name) {
                std::string n = lowerOf(name);
                if (isHidden(SPLITPEA_TYPE, n)) return true;
                return splitPeaCluster(n) != p.cluster;
            }, opt);
        total += st.partsDrawn;
    }
    if (partsDrawn) *partsDrawn = total;
    return total > 0;
}

// Measure the reference range (reanim units) and derive scale/anchor so the
// reanim art's idle median lands at artH px tall on the sheet's old foot line.
// The fit uses the MEDIAN per-frame metrics (height, center-x, bottom), not
// the union bbox: reference windows are long idle loops and any wind-up /
// flare / fade-out frame pokes the union (e.g. Kernel-pult's raised basket,
// Coffee Bean's fly-off) and would shrink the whole plant to fit a pose that
// is on screen a fraction of the time.
float medianOf(std::vector<float>& v) {
    if (v.empty()) return 0.0f;
    size_t m = v.size() / 2;
    std::nth_element(v.begin(), v.begin() + m, v.end());
    if (v.size() % 2) return v[m];
    float hi = v[m];
    std::nth_element(v.begin(), v.begin() + (m - 1), v.end());
    return (hi + v[m - 1]) * 0.5f;
}

bool calibrate(int plantType, PlantSkin& skin) {    int first = skin.refFirst, last = skin.refLast;
    if (skin.refSegment[0] != '\0') {
        auto it = skin.doc.segments.find(skin.refSegment);
        if (it == skin.doc.segments.end()) return false;
        first = it->second.first;
        last = it->second.last;
    }
    reanim::DrawOptions measure;
    measure.measureOnly = true;
    std::vector<float> hs, cxs, bots;
    if (plantType == SPLITPEA_TYPE) {
        // Split Pea has no single full-plant window; its heads only show inside
        // their own cluster windows. Measure the same three passes drawSplitPea
        // composes (leaves + back head + front head, idle windows), unioned per
        // loop step — otherwise the median reflects the bare stem/leaves and
        // the plant calibrates ~2x too large.
        struct Pass { int cluster, first, last; };
        Pass passes[3] = {{1, 24, 40}, {2, 7, 23}, {0, 54, 70}};
        int maxLen = 0;
        for (Pass& p : passes) {
            p.last = loopPlayLast(skin, p.first, p.last);
            maxLen = (std::max)(maxLen, p.last - p.first + 1);
        }
        for (int step = 0; step < maxLen; step += 2) {
            reanim::DrawStats uni;
            for (const Pass& p : passes) {
                int len = p.last - p.first + 1;
                float f = (float)p.first + (float)(step % len);
                reanim::DrawStats st = gRenderer->Draw(
                    skin.doc, f, 0.0f, 0.0f, 1.0f,
                    [&](const std::string& n) {
                        std::string nl = lowerOf(n);
                        if (isMeasureHidden(plantType, nl)) return true;
                        return splitPeaCluster(nl) != p.cluster;
                    }, measure);
                if (!st.hasBounds) continue;
                if (!uni.hasBounds) {
                    uni = st;
                } else {
                    uni.minX = (std::min)(uni.minX, st.minX);
                    uni.minY = (std::min)(uni.minY, st.minY);
                    uni.maxX = (std::max)(uni.maxX, st.maxX);
                    uni.maxY = (std::max)(uni.maxY, st.maxY);
                }
            }
            if (!uni.hasBounds) continue;
            hs.push_back(uni.maxY - uni.minY);
            cxs.push_back((uni.minX + uni.maxX) * 0.5f);
            bots.push_back(uni.maxY);
        }
        if (!hs.empty()) {
            float h = medianOf(hs);
            if (h <= 1.0f) return false;
            skin.scale = skin.artH / h;
            skin.refCX = medianOf(cxs);
            skin.refBottom = medianOf(bots);
            skin.calibrated = true;
            SDL_Log("reanimplant: %s splitpea composite medianH %.0f -> artH %.0f, scale %.3f",
                    skin.reanimFile, h, skin.artH, skin.scale);
        }
        return skin.calibrated;
    }
    for (int f = first; f <= last; f += 2) {
        reanim::DrawStats st = gRenderer->Draw(
            skin.doc, (float)f, 0.0f, 0.0f, 1.0f,
            [plantType](const std::string& n) {
                return isMeasureHidden(plantType, lowerOf(n));
            }, measure);
        if (!st.hasBounds) continue;
        hs.push_back(st.maxY - st.minY);
        cxs.push_back((st.minX + st.maxX) * 0.5f);
        bots.push_back(st.maxY);
    }
    if (hs.empty()) return false;
    float h = medianOf(hs);
    if (h <= 1.0f) return false;
    skin.scale = skin.artH / h;
    skin.refCX = medianOf(cxs);
    skin.refBottom = medianOf(bots);
    skin.calibrated = true;
    SDL_Log("reanimplant: %s ref %d..%d medianH %.0f -> artH %.0f, scale %.3f",
            skin.reanimFile, first, last, h, skin.artH, skin.scale);
    return true;
}

} // namespace

bool init(ISpriteRenderer& renderer) {
    if (gInitTried) return gReady;
    gInitTried = true;
    static reanim::SdlReanimRenderer sRenderer(renderer);
    gRenderer = &sRenderer;
    if (!gRenderer->BindDirectory(REANIM_DIR)) {
        SDL_Log("reanimplant: no part images in %s (plants keep sprite sheets)", REANIM_DIR);
        return gReady;
    }
    int loadedCount = 0;
    for (int t = 0; t < SKIN_COUNT; t++) {
        PlantSkin& skin = gSkins[t];
        std::string err;
        std::string path = std::string(REANIM_DIR) + "/" + skin.reanimFile;
        if (!reanim::LoadReanimXml(path, skin.doc, err)) {
            SDL_Log("reanimplant: %s (plant %d keeps sprite sheets)", err.c_str(), t);
            continue;
        }
        skin.loaded = true;
        calibrate(t, skin); // log the fitted scale at load time instead of first draw
        buildIdlePlan(t, skin);
        loadedCount++;
    }
    gReady = loadedCount > 0;
    SDL_Log("reanimplant: %d/%d reanim skins loaded", loadedCount, SKIN_COUNT);
    return gReady;
}

bool ready() { return gReady; }

// Pumpkin damage look (original IMAGE_REANIM_PUMPKIN_DAMAGE1/3 swap): the
// Pumpkin class posts its shell hp right before drawing itself.
void setPumpkinHealth(int hp, int hpMax) { gPumpkinHp = hp; gPumpkinHpMax = hpMax; }

// Kernel-pult loaded ammo (original Cornpult_butter/kernal render groups):
// PultBase::display posts whether a butter lob is armed right before drawing.
void setKernelPultButterArmed(bool armed) { gKernelPultButterArmed = armed; }

float frameRate(int plantType) {
    if (plantType < 0 || plantType >= SKIN_COUNT || !gSkins[plantType].loaded)
        return 0.0f;
    return gSkins[plantType].doc.fps;
}

// Static table (scale_audit.md S8.2), valid before init().
float plant_idle_height_px(int plantType) {
    if (plantType < 0 || plantType >= SKIN_COUNT)
        return 0.0f;
    return gSkins[plantType].artH;
}

bool drawPlant(int plantType, int sheetDir, float sframe, int nSheet,
               float reanimClock, int cellX1, int cellX2, int footY,
               int gameHeight, bool flash,
               const std::unordered_set<std::string>* extraHide,
               const std::unordered_set<std::string>* extraShow,
               int* partsDrawn, float* sampledFrame, float flash_fade,
               float scaleMul, const PlantPlacement* retailPlacement) {
    if (!gReady) return false;
    if (plantType < 0 || plantType >= SKIN_COUNT) return false;
    PlantSkin& skin = gSkins[plantType];
    if (!skin.loaded) return false;

    // Split Pea has no full-plant window; compose leaves + both heads.
    if (plantType == SPLITPEA_TYPE) {
        if (sampledFrame) *sampledFrame = -1.0f; // composite: no single frame
        return drawSplitPea(skin, sheetDir, sframe, nSheet, cellX1, cellX2,
                            footY, flash, partsDrawn, scaleMul,
                            retailPlacement);
    }

    SegPick pick = pickSegment(plantType, sheetDir);
    int first, last;
    if (!resolveRange(skin, pick, first, last)) return false;
    // Looping states play over [first, playLast] with the loop-seam duplicate
    // trimmed (loopPlayLast); one-shots keep the raw last frame and clamp.
    int playLast = pick.clamp ? last : loopPlayLast(skin, first, last);
    int segLen = playLast - first + 1;

    // Sync mode: map the sheet frame proportionally into the reanim range so
    // events keyed to sheet frames (pea fire, explosion) stay aligned.
    // Idle states instead run the clock-driven idle/fidget lifecycle.
    float local = 0.0f;
    int wrapFirst = first, wrapLast = playLast;
    const IdlePlan& plan = gIdlePlans[plantType];
    if (pick.idleCycle && plan.valid) {
        uint32_t seed = mixHash((uint32_t)plantType * 0x9e3779b9u ^
                                (uint32_t)cellX1 ^ ((uint32_t)footY << 16));
        // Retail pea-family bodies idle at RandRangeFloat(15, 20) fps
        // (repvz Plant.cpp:206-244) while the clone feeds reanim_clock at
        // doc rate 12; scale the idle clock by a per-plant hashed factor in
        // [15,20]/12. Other plants keep 12 (retail RandRangeFloat(10,15)
        // averages 12.5, indistinguishable from the fed rate).
        float rateMul = 1.0f;
        if (plantType == PEASHOOTER_TYPE || plantType == SNOWPEA_TYPE ||
            plantType == REPEATER_TYPE) {
            uint32_t hr = mixHash(seed ^ 0xA53A9D01u);
            rateMul = (15.0f + 5.0f * (float)(hr & 0xFFFFu) / 65536.0f) / 12.0f;
        }
        local = evalIdleCycle(plan, reanimClock * rateMul, seed, wrapFirst,
                              wrapLast) -
                (float)first;
    } else {
        local = nSheet > 0 ? (float)sframe * segLen / nSheet : 0.0f;
        if (local > (float)(segLen - 1))
            local = pick.clamp ? (float)(segLen - 1) : fmodf(local, (float)segLen);
    }
    if (sampledFrame) *sampledFrame = (float)first + local;

    if (!skin.calibrated && !calibrate(plantType, skin)) return false;
    (void)gameHeight; // placement comes from the sheet-measured targets now
    // scaleMul (F2 layout editor): multiplies the calibrated scale; refCX /
    // refBottom scale along, so the art's bottom-center stays pinned on the
    // (cell center, footY) anchor. In-game drawing passes retailPlacement
    // instead: doc origin at the cell top-left at scale 1.0.
    float scale = retailPlacement ? retailPlacement->scale
                                  : skin.scale * scaleMul;
    float ox = retailPlacement
                   ? retailPlacement->x
                   : (cellX1 + cellX2) * 0.5f + skin.dx - scale * skin.refCX;
    float oy = retailPlacement
                   ? retailPlacement->y
                   : (float)footY - skin.padBottom + skin.dy - scale * skin.refBottom;

    reanim::DrawOptions opt;
    opt.tint = pick.tint;
    opt.flash = flash; // white-silhouette pass ~= the *_blink sheet overlay
    // fade-out over the blink budget (game hook feeds attacked_time /
    // MAX_TIME_BLINK); clamps keep the default strength when unfaded
    opt.flashAlpha = (Uint8)std::clamp(lroundf(opt.flashAlpha * flash_fade), 0l, 255l);
    opt.loopWrapFirst = wrapFirst;
    opt.loopWrapLast = wrapLast;
    std::unordered_map<std::string, std::string> overrides;
    std::unordered_map<std::string, SDL_FPoint> overrideOffsets;
    if (pick.faceOverride)
        overrides["anim_face"] = pick.faceOverride;
    if (plantType == PEASHOOTER_TYPE) {
        // The classic silhouette's back leaf is the small leaf blade
        // (PeaShooter_leaf.png, 21x23). The slot's pivots were authored for
        // the 41x54 peashooter_leaf2 curl the 2Pak body bakes in; as-is they
        // would put the blade ~11px above the head dome (classic sheet:
        // leaf top ~= head top + 5, its back edge tucked 2-3px under the
        // head), so shift it down onto the dome's back edge. The leaf track
        // still precedes the head track, so the blade is occluded where they
        // overlap, and it keeps the authored sway keyframes.
        overrides["leaf"] = "peashooter_leaf";
        overrideOffsets["leaf"] = SDL_FPoint{6.0f, 12.0f};
    }
    if (plantType == REPEATER_TYPE) {
        // Repeater = peashooter body + fiercer set, drawn in place of the two
        // baked-in slots: same pivots/z-order/animation as the 2Pak originals
        // (leaf stays behind the head, hooked to the stem top).
        overrides["leaf"] = "repeater_leaf";
        overrides["peashooter_eyebrow"] = "repeater_eyebrows";
    }
    if (plantType == PUMPKIN_TYPE && gPumpkinHp >= 0 && gPumpkinHpMax > 0) {
        // Shell degrade by hp thirds (original: the pumpkin's image swaps to
        // IMAGE_REANIM_PUMPKIN_DAMAGE1/3; the 2Pak set ships exactly those,
        // matching pivots). >2/3 intact, >1/3 cracked, <=1/3 heavily cracked.
        if (gPumpkinHp * 3 <= gPumpkinHpMax)
            overrides["pumpkin_front"] = "pumpkin_damage3";
        else if (gPumpkinHp * 3 <= 2 * gPumpkinHpMax)
            overrides["pumpkin_front"] = "pumpkin_damage1";
    }
    if (!overrides.empty()) opt.imageOverrides = &overrides;
    if (!overrideOffsets.empty()) opt.imageOverrideOffsets = &overrideOffsets;
    // Debug overrides (anim viewer; reanimz::drawZombie semantics): a show
    // takes a track out of the hide set AND lets it render on f=-1 frames.
    std::unordered_set<std::string> hideL, showL;
    if (extraHide)
        for (const auto& t : *extraHide) hideL.insert(lowerOf(t));
    if (extraShow) {
        for (const auto& t : *extraShow) showL.insert(lowerOf(t));
        if (!showL.empty()) opt.forceShow = &showL;
    }
    reanim::DrawStats st = gRenderer->Draw(
        skin.doc, (float)first + local, ox, oy, scale,
        [&](const std::string& n) {
            std::string nl = lowerOf(n);
            if (showL.count(nl)) return false;
            if (hideL.count(nl)) return true;
            return isHidden(plantType, nl);
        }, opt);
    if (partsDrawn) *partsDrawn = st.partsDrawn;
    return st.partsDrawn > 0;
}

std::vector<std::string> trackNames(int plantType) {
    std::vector<std::string> out;
    if (!gReady || plantType < 0 || plantType >= SKIN_COUNT) return out;
    const PlantSkin& skin = gSkins[plantType];
    if (!skin.loaded) return out;
    for (const auto& tr : skin.doc.tracks) out.push_back(lowerOf(tr.name));
    return out;
}

std::vector<TrackStatus> trackStatuses(int plantType, int sheetDir,
                                       float sframe, int nSheet) {
    std::vector<TrackStatus> out;
    if (!gReady) return out;
    float absFrame;
    if (!resolveSampleFrame(plantType, sheetDir, sframe, nSheet, absFrame))
        return out;
    for (const auto& tr : gSkins[plantType].doc.tracks) {
        reanim::Transform t = reanim::SampleTrack(tr, absFrame);
        std::string n = lowerOf(tr.name);
        out.push_back({n, tr.hasAnyImage, isHidden(plantType, n), t.f < -0.5f, false});
    }
    return out;
}

std::vector<SegmentInfo> segments(int plantType) {
    std::vector<SegmentInfo> out;
    if (!gReady || plantType < 0 || plantType >= SKIN_COUNT) return out;
    const PlantSkin& skin = gSkins[plantType];
    if (!skin.loaded) return out;
    for (const auto& kv : skin.doc.segments)
        out.push_back({kv.first, kv.second.first, kv.second.last});
    std::sort(out.begin(), out.end(),
              [](const SegmentInfo& a, const SegmentInfo& b) { return a.first < b.first; });
    return out;
}

// Debug tooling (anim viewer): looping play length of one labelled segment,
// seam-trimmed like drawPlantSegment's play window.
float segmentLength(int plantType, const char* segment) {
    if (!gReady || plantType < 0 || plantType >= SKIN_COUNT) return 0.0f;
    const PlantSkin& skin = gSkins[plantType];
    if (!skin.loaded) return 0.0f;
    int first, last;
    if (!findSeg(skin, segment, first, last)) return 0.0f;
    return (float)(loopPlayLast(skin, first, last) - first + 1);
}

// Debug-only sibling of drawPlant: plays one labelled segment raw (no idle
// lifecycle, no sheet sync). Placement/visibility math mirrors drawPlant —
// if the drawPlant tail changes, update this too.
bool drawPlantSegment(int plantType, const char* segment, float frame,
                      int cellX1, int cellX2, int footY, bool flash,
                      const std::unordered_set<std::string>* extraHide,
                      const std::unordered_set<std::string>* extraShow,
                      int* partsDrawn) {
    if (!gReady) return false;
    if (plantType < 0 || plantType >= SKIN_COUNT) return false;
    PlantSkin& skin = gSkins[plantType];
    if (!skin.loaded) return false;
    int first, last;
    if (!findSeg(skin, segment, first, last)) return false;
    int playLast = loopPlayLast(skin, first, last);
    int segLen = playLast - first + 1;
    float local = frame < 0.0f ? 0.0f : fmodf(frame, (float)segLen);

    if (!skin.calibrated && !calibrate(plantType, skin)) return false;
    float scale = skin.scale;
    float ox = (cellX1 + cellX2) * 0.5f + skin.dx - scale * skin.refCX;
    float oy = (float)footY - skin.padBottom + skin.dy - scale * skin.refBottom;

    reanim::DrawOptions opt;
    opt.flash = flash;
    opt.loopWrapFirst = first;
    opt.loopWrapLast = playLast;
    std::unordered_map<std::string, std::string> overrides;
    std::unordered_map<std::string, SDL_FPoint> overrideOffsets;
    if (plantType == PEASHOOTER_TYPE) {
        overrides["leaf"] = "peashooter_leaf"; // see drawPlant
        overrideOffsets["leaf"] = SDL_FPoint{6.0f, 12.0f};
    }
    if (plantType == REPEATER_TYPE) {
        // repeater look on the shared peashooter body (see drawPlant)
        overrides["leaf"] = "repeater_leaf";
        overrides["peashooter_eyebrow"] = "repeater_eyebrows";
    }
    if (!overrides.empty()) opt.imageOverrides = &overrides;
    if (!overrideOffsets.empty()) opt.imageOverrideOffsets = &overrideOffsets;
    std::unordered_set<std::string> hideL, showL;
    if (extraHide)
        for (const auto& t : *extraHide) hideL.insert(lowerOf(t));
    if (extraShow) {
        for (const auto& t : *extraShow) showL.insert(lowerOf(t));
        if (!showL.empty()) opt.forceShow = &showL;
    }
    reanim::DrawStats st = gRenderer->Draw(
        skin.doc, (float)first + local, ox, oy, scale,
        [&](const std::string& n) {
            std::string nl = lowerOf(n);
            if (showL.count(nl)) return false;
            if (hideL.count(nl)) return true;
            return isHidden(plantType, nl);
        }, opt);
    if (partsDrawn) *partsDrawn = st.partsDrawn;
    return st.partsDrawn > 0;
}

std::vector<TrackStatus> segmentTrackStatuses(int plantType, const char* segment,
                                              float frame) {
    std::vector<TrackStatus> out;
    if (!gReady || plantType < 0 || plantType >= SKIN_COUNT) return out;
    const PlantSkin& skin = gSkins[plantType];
    if (!skin.loaded) return out;
    int first, last;
    if (!findSeg(skin, segment, first, last)) return out;
    int playLast = loopPlayLast(skin, first, last); // mirrors drawPlantSegment
    float local = frame < 0.0f ? 0.0f : fmodf(frame, (float)(playLast - first + 1));
    float absFrame = (float)first + local;
    for (const auto& tr : skin.doc.tracks) {
        reanim::Transform t = reanim::SampleTrack(tr, absFrame, first, playLast);
        std::string n = lowerOf(tr.name);
        out.push_back({n, tr.hasAnyImage, isHidden(plantType, n), t.f < -0.5f, false});
    }
    return out;
}

} // namespace reanimplant
