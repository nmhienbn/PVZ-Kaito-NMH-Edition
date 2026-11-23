#include "almanac.hpp"
#include "almanac_info.hpp"
#include "elements/Player/Player.hpp"
#include "elements/SeedPacket/SeedPacket.hpp"
#include "elements/boss/boss_driver.hpp"
#include "elements/button/button.hpp"
#include "level/debut_data.hpp"
#include "music/music.hpp"
#include "reanim/reanim.hpp"
#include "reanim/reanim_plant.hpp"
#include "reanim/reanim_sdl.hpp"
#include "reanim/reanim_zombie.hpp"

#include <algorithm>
#include <cmath>
#include <string>
#include <unordered_set>

extern Window win;
extern Player player;
extern bool quit;

// The almanac pages are authored in the original game's 800x600 space; this
// scene keeps them at native scale, centered in the 1050x600 window (+125).
const int ALMANAC_X_OFF = (WINDOW_WIDTH - 800) / 2;

// Idle reanim sheet directory per plant type (the reanim pipeline maps it to
// the plant's idle pose/loop; see pickSegment in reanim_plant.cpp).
const int PLANT_IDLE_DIR[PLANT_COUNT] = {
    PEASHOOTER_SHEET_DIRECTORY, SUNFLOWER_SHEET_DIRECTORY, WALLNUT_1_DIRECTORY,
    SNOWPEA_SHEET_DIRECTORY, POTATOMINE_IDLE_DIRECTORY, CHERRYBOMB_SHEET_DIRECTORY,
    PEASHOOTER_SHEET_DIRECTORY /*repeater*/, CHOMPER_SHEET_DIRECTORY,
    PUFFSHROOM_SHEET_DIRECTORY, SUNSHROOM_SHEET_DIRECTORY, FUMESHROOM_SHEET_DIRECTORY,
    GRAVEBUSTER_IDLE_DIRECTORY, HYPNOSHROOM_SHEET_DIRECTORY, SCAREDYSHROOM_SHEET_DIRECTORY,
    ICESHROOM_SHEET_DIRECTORY, DOOMSHROOM_SHEET_DIRECTORY, LILYPAD_SHEET_DIRECTORY,
    SQUASH_SHEET_DIRECTORY, THREEPEATER_SHEET_DIRECTORY, TANGLEKELP_SHEET_DIRECTORY,
    JALAPENO_SHEET_DIRECTORY, SPIKEWEED_SHEET_DIRECTORY, TORCHWOOD_SHEET_DIRECTORY,
    TALLNUT_SHEET_DIRECTORY, SEASHROOM_SHEET_DIRECTORY, PLANTERN_SHEET_DIRECTORY,
    CACTUS_SHEET_DIRECTORY, BLOVER_SHEET_DIRECTORY, SPLITPEA_SHEET_DIRECTORY,
    STARFRUIT_SHEET_DIRECTORY, PUMPKIN_SHEET_DIRECTORY, MAGNETSHROOM_SHEET_DIRECTORY,
    CABBAGEPULT_DIRECTORY, FLOWERPOT_DIRECTORY, KERNELPULT_DIRECTORY,
    COFFEEBEAN_DIRECTORY, GARLIC_DIRECTORY, UMBRELLALEAF_DIRECTORY,
    MARIGOLD_DIRECTORY, MELONPULT_DIRECTORY};

void almanac_plant_icon_box(int type, int cell_x, int cell_y, int &bx, int &by,
                            int &bw, int &bh)
{
    int margin = type == LILYPAD_TYPE ? 0 : ALMANAC_ICON_BOTTOM_MARGIN;
    bx = cell_x + ALMANAC_ICON_SIDE_INSET;
    by = cell_y + ALMANAC_ICON_TOP_INSET;
    bw = ALMANAC_PLANT_CELL_W - 2 * ALMANAC_ICON_SIDE_INSET;
    bh = ALMANAC_PLANT_CELL_H - ALMANAC_ICON_TOP_INSET - margin;
}

void almanac_draw_plant_icon(int type, int cell_x, int cell_y)
{
    if (type < 0 || type >= PLANT_COUNT)
        return;
    int bx, by, bw, bh;
    almanac_plant_icon_box(type, cell_x, cell_y, bx, by, bw, bh);
    // One shared seed scale for every almanac packet (ALMANAC_SEED_SCALE):
    // the grid reads at a uniform zoom, bottom-anchored inside the box --
    // NOT the bank's world-height sizing.
    draw_plant_icon_scaled(type, bx + bw / 2, by + bh, bw, bh,
                           ALMANAC_SEED_SCALE);
}

// Per-zombie reanim icon geometry. walk_dir/armor_dir pick the anim like the
// in-game draw; the EFFECTIVE bbox is measured at runtime by the placement
// probes (reanimz::zombiePoseGeom / skinPoseGeom): the renderer's opaque
// union over the pose's whole play window, in the retail pivot frame (doc
// point (30,120), skin scale folded -- see reanim_zombie.hpp). Four zombies
// crop the FULL pose render to a PIVOT-SPACE rect instead of fitting the
// whole anim: for those, the crop rect is the effective bbox -- the art
// beyond the rect is scissored off, so the vignette is steady (a pixel crop;
// the pose itself is unchanged). The dancer's disco doc is almanac-local
// (see the namespace below) and keeps hand-measured numbers instead of a
// probe (documented deviation: its draw stays at DISCO_DRAW_SCALE, not the
// retail 0.8 dancer skin scale).
//
// Per-type preview poses (the "recipe"): the plain default previews the walk
// loop; several zombies read wrong that way, so the recipe picks a different
// segment and/or a crop rect:
//   pole vaulter  anim_idle -- standing with the pole in hand (the plain
//                  post-vault "anim_walk" has no pole and read as a generic
//                  runner, i.e. the pole vaulter was effectively missing)
//   dancer        the 2Pak DISCO sheet set (orange quiff, shades, sparkly
//                  suit): its own doc (Zombie_disco.reanim, converted from
//                  2Pak's zombie_jackson.reanim.compiled) -- the in-game
//                  Zombie_dancer.reanim walks the plain backup-dancer body
//   dolphin rider ARMOR phase DOLPHIN_CARRY_STATE -> "anim_walkdolphin": the
//                  pool-entry walk that carries the dolphin ON ITS HEAD
//                  (plain anim_walk is the post-jump wade, no dolphin)
//   balloon       "anim_idle" -- the credited hover with the balloon aloft
//                  (the 2Pak walk segment hid the balloon parts entirely,
//                  showing the popped, ground-walking zombie)
//   catapult      "anim_idle" FULL render, crop rect on the driver's
//                  head+face zone (-24.8,-125.1)..(89.2,-52.2) pivot units -- the
//                  vehicle frame/wheels sit outside the rect and are
//                  scissored off
//   gargantuar    "anim_walk" FULL render, crop rect on head+torso+pack imp
//                  (-159.7,-207.0)..(126.9,-3.3) -- the legs animate below
//                  the rect (the telephone pole behind the shoulder stays
//                  inside, as in the full art)
//   zomboni       "anim_drive" FULL render (the doc already includes the
//                  driver in the cabin -- no tracks hidden), crop rect =
//                  the vehicle silhouette (-116.7,-141.0)..(105.2,36.0); the
//                  brush dips below the wheel line on bounce frames and gets
//                  clipped by the cell
//   zomboss       "anim_head_idle" head-only track subset (kBossBodyCrop,
//                  kept because the head sits in FRONT of the body in this
//                  pose, so a pure pixel crop would keep body junk behind/
//                  through the neck) + the dome driver overlay riding the
//                  boss_head track; the crop rect (547.8,55.5)..(747.8,225.5)
//                  frames the head TOP -- the cockpit dome with the doctor
//                  plus the exposed brain; the jaw clips below and the wide
//                  skull side/antenna struts clip at the glass edges at the
//                  common 0.55 almanac scale
enum AlmanacZombiePose
{
    AZ_POSE_WALK,    // drawZombie(walk_dir, armor_dir) -- the default
    AZ_POSE_SEGMENT, // drawSkinSegment(skin, seg) -- a chosen full segment
    AZ_POSE_DISCO,   // the 2Pak disco-dancer doc (almanac-local, see below)
    AZ_POSE_ZOMBOSS, // boss head track subset + dome driver overlay
};

struct AlmanacZombie
{
    int walk_dir;
    int armor_dir; // CONE1/BUCKET1 directory, a reanimz phase (dolphin), -1 otherwise
    AlmanacZombiePose pose = AZ_POSE_WALK;
    int skin = -1;                                  // AZ_POSE_SEGMENT: reanimz skin
    const char *seg = nullptr;                      // AZ_POSE_SEGMENT: segment name
    const std::unordered_set<std::string> *hide = nullptr; // Zomboss: body-out tracks
    // AZ_POSE_DISCO only: hand-fitted placement of the almanac-local disco
    // doc (raw draw at (x, y), DISCO_DRAW_SCALE folded in) -- bbox_w/h the
    // effective bbox, cx_off/foot_off its center/bottom in that draw's own
    // scale-1 pose space.
    float bbox_w = 0, bbox_h = 0, cx_off = 0, foot_off = 0;
    // Pose-space crop rect (PIVOT space: doc point (30,120) origin, skin
    // scale folded -- the same frame the probes report): crop_on=true
    // renders the FULL pose and scissor-clips to this rect; the rect is
    // then also the effective bbox the layout fits/centers.
    bool crop_on = false;
    float crop_l = 0, crop_t = 0, crop_r = 0, crop_b = 0;
};

// Track set cropped out of the Zomboss head pose (matched lowercase, same
// semantics as reanimz::drawZombie's extraHide): everything but the head and
// its cockpit dome. The legs/rv/body tracks lie BEHIND the head in
// anim_head_idle, so a pixel crop alone would leave body slivers around the
// neck; hiding the body tracks is the difference between "head with cockpit"
// and "head in front of the robot". (catapult/gargantuar no longer hide
// tracks -- their vignettes are pure pixel crops of the full pose.)
static const std::unordered_set<std::string> kBossBodyCrop = {
    "boss_innerarm_bits", "boss_innerarm_lower", "boss_innerarm_finger4",
    "boss_innerarm_finger2", "boss_innerarm_finger1", "boss_innerarm_finger3",
    "boss_innerarm_hand", "boss_rv_wheel1", "boss_rv_wheel2", "boss_rv",
    "boss_innerarm_thumb2", "boss_innerarm_thumb1", "boss_innerarm_upper",
    "boss_innerleg_foot", "boss_innerleg_bits", "boss_innerleg_lower",
    "boss_innerleg_upper", "boss_body2", "boss_outerleg_foot",
    "boss_outerleg_bits", "boss_outerleg_lower", "boss_outerleg_upper",
    "boss_body1", "boss_neck", "boss_outerarm_thumb2", "boss_outerarm_hand",
    "boss-outerarm_finger4", "boss-outerarm_knuckle4", "boss-outerarm_finger3",
    "boss-outerarm_knuckle3", "boss-outerarm_finger2", "boss-outerarm_knuckle2",
    "boss-outerarm_finger1", "boss-outerarm_knuckle1", "boss_outerarm_thumb1",
    "boss_outerarm_bits", "boss_outerarm_lower", "boss_outerarm_upper",
};

// The recipe: pose selection only (the geometry itself is probed at runtime
// -- see almanac_pose_geom below). The disco entry keeps its hand-fitted
// bbox/placement numbers; the four crop entries carry their PIVOT-space
// crop rects (same frame as the probe's geometry).
const AlmanacZombie ZOMBIE_RECIPE[COUNT_ZOMBIE_TYPE] = {
    {ZOMBIE_WALK1_DIRECTORY, -1},                                    // normal
    {FLAG_ZOMBIE_WALK1_1_DIRECTORY, -1},                             // flag
    {ZOMBIE_WALK1_DIRECTORY, CONE1_DIRECTORY},                       // conehead
    {ZOMBIE_WALK1_DIRECTORY, BUCKET1_DIRECTORY},                     // buckethead
    {DOOR_ZOMBIE_WALK1_1_DIRECTORY, -1},                             // screen door
    {POLEVAULT_WALK_DIRECTORY, -1, AZ_POSE_SEGMENT,
     reanimz::SKIN_POLEVAULT, "anim_idle"},                          // pole vaulter: standing, pole in hand
    {NEWSPAPER_WALK_DIRECTORY, -1},                                 // newspaper
    {FOOTBALL_RUN_DIRECTORY, -1},                                   // football
    {DANCER_WALK_DIRECTORY, -1, AZ_POSE_DISCO, -1, nullptr, nullptr,
     72, 114, 27.5f, 92.0f},                                        // dancer (hand-fitted)
    {BACKUP_WALK_DIRECTORY, -1},                                     // backup dancer
    {DUCKYTUBE_WALK_DIRECTORY, -1},                                 // ducky tube
    {SNORKEL_WALK_DIRECTORY, -1},                                   // snorkel
    {ZOMBONI_DRIVE_DIRECTORY, -1, AZ_POSE_WALK, -1, nullptr, nullptr,
     0, 0, 0, 0, true, -116.7f, -141.0f, 105.2f, 36.0f},            // full vehicle + driver crop
    {BOBSLED_WALK_DIRECTORY, -1},                                   // bobsled team
    {DOLPHINRIDER_WALK_DIRECTORY, reanimz::DOLPHIN_CARRY_STATE},    // dolphin rider: anim_walkdolphin, dolphin on head
    {JACKBOX_WALK_DIRECTORY, -1},                                   // jack-in-the-box
    {BALLOON_WALK_DIRECTORY, -1, AZ_POSE_SEGMENT,
     reanimz::SKIN_BALLOON, "anim_idle"},                            // balloon: only idle shows the balloon
    {DIGGER_WALK_DIRECTORY, -1},                                     // digger
    {POGO_WALK_DIRECTORY, -1},                                       // pogo
    {BUNGEE_WALK_DIRECTORY, -1},                                     // bungee
    {LADDER_WALK_DIRECTORY, -1},                                     // ladder
    {CATAPULT_WALK_DIRECTORY, -1, AZ_POSE_SEGMENT,
     reanimz::SKIN_CATAPULT, "anim_idle", nullptr, 0, 0, 0, 0, true,
     -24.8f, -125.1f, 89.2f, -52.2f},                               // full render, driver head+torso crop
    {GARGANTUAR_WALK_DIRECTORY, -1, AZ_POSE_SEGMENT,
     reanimz::SKIN_GARGANTUAR, "anim_walk", nullptr, 0, 0, 0, 0, true,
     -159.7f, -207.0f, 126.9f, -3.3f},                              // full render, head+torso crop
    {IMP_WALK_DIRECTORY, -1},                                        // imp
    {0, -1, AZ_POSE_ZOMBOSS, -1, nullptr, &kBossBodyCrop, 0, 0, 0, 0,
     true, 547.8f, 55.5f, 747.8f, 225.5f},                          // head top: dome driver + brain, jaw clipped
    {YETI_WALK_DIRECTORY, -1},                                      // zombie yeti
};

// Effective pose geometry of an almanac entry, in the draw's own frame:
//   pivotX/pivotY: where the pose's doc-pivot point lands for a draw at
//     (x_location, y_location), mul 1, no deltas -- (45+offX, 132+offY) for
//     the skins (the art grows about it), + the boss doc compensation for
//     Zomboss (its drawSkinSegment call passes that pair as ddx/ddy).
//   C/B/W/H: effective bbox center-x / bottom / size, pivot space at mul 1
//     (the probe's opaque-union box, overridden by the crop rect when the
//     entry crops -- the rect IS the effective bbox then).
// A caller wanting the bbox centered on (ax, ay) at scale m draws at
//   x_location = ax - pivotX - m*C,  y_location = ay - pivotY - m*B.
// Lazily inits reanimz (almanac_zombie_place runs before the icon draw's
// own init dance); ok=false when the assets are unavailable or the pose is
// the hand-fitted disco (caller uses the recipe's numbers).
struct AlmanacPoseGeom
{
    float pivotX = 0, pivotY = 0;
    float C = 0, B = 0, W = 0, H = 0;
    bool ok = false;
};

static bool almanac_pose_geom(int type, float scale, AlmanacPoseGeom &out)
{
    out.ok = false;
    if (type < 0 || type >= COUNT_ZOMBIE_TYPE)
        return false;
    const AlmanacZombie &r = ZOMBIE_RECIPE[type];
    if (r.pose == AZ_POSE_DISCO)
        return false; // hand-fitted placement (the recipe numbers)
    if (!reanimz::ready() && !reanimz::init(win.sprite_renderer()))
        return false;
    reanimz::PoseGeom g;
    bool have;
    if (r.pose == AZ_POSE_ZOMBOSS)
        have = reanimz::skinPoseGeom(reanimz::SKIN_ZOMBOSS, "anim_head_idle", g,
                                     &kBossBodyCrop);
    else if (r.pose == AZ_POSE_SEGMENT)
        have = reanimz::skinPoseGeom(r.skin, r.seg, g, r.hide);
    else
        have = reanimz::zombiePoseGeom(r.walk_dir, r.armor_dir, g);
    if (!have)
        return false;
    out.pivotX = g.pivotX;
    out.pivotY = g.pivotY;
    if (r.crop_on)
    {
        // the crop rect is the effective bbox (pivot space, like C/B)
        out.C = (r.crop_l + r.crop_r) * 0.5f;
        out.B = r.crop_b;
        out.W = r.crop_r - r.crop_l;
        out.H = r.crop_b - r.crop_t;
    }
    else
    {
        out.C = g.C;
        out.B = g.B;
        out.W = g.W;
        out.H = g.H;
    }
    if (r.pose == AZ_POSE_ZOMBOSS)
    {
        // the draw passes the boss doc compensation as ddx/ddy: fold it into
        // the pivot so placement and clip rect track the real draw (the pair
        // is scale-dependent, hence the `scale` parameter).
        int ddx, ddy;
        reanimz::bossDocCompensation(scale, ddx, ddy);
        out.pivotX += (float)ddx;
        out.pivotY += (float)ddy;
    }
    out.ok = true;
    return true;
}

// Place a measured bbox at a spot, bottom-center, at ONE common draw scale
// (ALMANAC_ZOMBIE_DRAW_SCALE) -- no per-zombie fit: every zombie in the
// almanac reads at its natural relative size, and an art that would spill
// its container is scissored at draw time instead. `scale` lets non-thumbnail
// callers override (off-screen calibration probes, the index-page mascot).
AlmanacZombieLayout almanac_zombie_place(int type, float target_cx,
                                         float target_bottom,
                                         float scale = ALMANAC_ZOMBIE_DRAW_SCALE)
{
    const AlmanacZombie &r = ZOMBIE_RECIPE[type];
    AlmanacZombieLayout l;
    l.scale = scale;
    l.anchor_x = target_cx;
    l.anchor_y = target_bottom;
    float w = 0, h = 0;
    if (r.pose == AZ_POSE_DISCO)
    {
        w = r.bbox_w * scale; // hand-fitted numbers (see the recipe)
        h = r.bbox_h * scale;
    }
    else
    {
        AlmanacPoseGeom g;
        if (almanac_pose_geom(type, scale, g))
        {
            w = g.W * scale;
            h = g.H * scale;
        }
    }
    l.bbox_l = target_cx - w / 2.0f;
    l.bbox_r = target_cx + w / 2.0f;
    l.bbox_b = target_bottom;
    l.bbox_t = target_bottom - h;
    return l;
}

const Button ALMANAC_CLOSE(ALMANAC_X_OFF + 676, ALMANAC_X_OFF + 676 + 89, 567, 593);
const Button ALMANAC_BACK_INDEX(ALMANAC_X_OFF + 32, ALMANAC_X_OFF + 32 + 164, 567, 593);
const Button ALMANAC_VIEW_PLANTS(ALMANAC_X_OFF + 130, ALMANAC_X_OFF + 130 + 156, 345, 387);
const Button ALMANAC_VIEW_ZOMBIES(ALMANAC_X_OFF + 487, ALMANAC_X_OFF + 487 + 210, 345, 393);

const RGB ALMANAC_TITLE_GRAY(220, 220, 220);
const RGB ALMANAC_TITLE_GOLD(213, 159, 43);
const RGB ALMANAC_TITLE_GREEN(0, 196, 0);
const RGB ALMANAC_TEXT_BLUE(40, 50, 90);
const RGB ALMANAC_BUTTON_LABEL(42, 42, 90);
const RGB ALMANAC_ZOMBIE_NAME(190, 255, 235);

enum AlmanacPage
{
    ALMANAC_PAGE_INDEX,
    ALMANAC_PAGE_PLANTS,
    ALMANAC_PAGE_ZOMBIES
};

struct AlmanacState
{
    int page = ALMANAC_PAGE_INDEX;
    int selected_plant = -1;
    int selected_zombie = -1;
    int scroll_row = 0;  // plants grid top row
    int zscroll_row = 0; // zombies grid top row
    bool scroll_drag = false;
    int scroll_grab_dy = 0; // cursor offset inside the thumb at drag start
    float clock = 0.0f;     // reanim time, ~12 fps units
    bool done = false;
};

bool is_plant_unlocked(int type)
{
    return level_unlock_new_plant[type] <= player.unlocked_level;
}

void show_text_middle(const string &text, int center_x, int y, const RGB &color,
                      string font, int size, int outline = 0, const RGB &outline_color = BLACK)
{
    int w = 0, h = 0;
    TTF_SizeText(win.get_font(font, size), text.c_str(), &w, &h);
    if (outline > 0)
        win.show_text_outlined(text, center_x - w / 2, y, color, font, size, outline, outline_color);
    else
        win.show_text(text, center_x - w / 2, y, color, font, size);
}

// Small word-wrap helper for the almanac flavor text. line_height <= 0 uses
// the font's own line skip (matches the original's GetLineSpacing flow).
void show_text_wrapped(const string &text, int x, int y, int width,
                       const RGB &color, string font, int size, int line_height = 0)
{
    string line, word;
    TTF_Font *f = win.get_font(font, size);
    if (line_height <= 0)
        line_height = TTF_FontLineSkip(f) + 2;
    auto flush = [&]()
    {
        if (line.empty())
            return;
        win.show_text(line, x, y, color, font, size);
        y += line_height;
        line.clear();
    };
    for (size_t i = 0; i <= text.size(); i++)
    {
        char c = i < text.size() ? text[i] : ' ';
        if (c == '\n')
        {
            flush();
            continue;
        }
        if (c == ' ')
        {
            if (!word.empty())
            {
                string trial = line.empty() ? word : line + " " + word;
                int tw = 0, th = 0;
                TTF_SizeText(f, trial.c_str(), &tw, &th);
                if (tw > width)
                {
                    flush();
                    line = word;
                }
                else
                    line = trial;
                word.clear();
            }
            continue;
        }
        word += c;
    }
    flush();
}

string almanac_recharge_text(int loading_time)
{
    if (loading_time < 1500)
        return "fast";
    if (loading_time < 2500)
        return "slow";
    return "very slow";
}

// Stock Almanac_Ground* patch for types with NO adventure debut (debut 0,
// e.g. the yeti) -- the pre-debut-data ground choice kept verbatim.
int almanac_ground_for_plant(int type)
{
    switch (type)
    {
    case LILYPAD_TYPE:
    case TANGLEKELP_TYPE:
        return ALMANAC_GROUND_POOL_DIRECTORY;
    case SEASHROOM_TYPE:
        return ALMANAC_GROUND_NIGHT_POOL_DIRECTORY;
    case FLOWERPOT_TYPE:
        return ALMANAC_GROUND_ROOF_DIRECTORY;
    case PUFFSHROOM_TYPE:
    case SUNSHROOM_TYPE:
    case FUMESHROOM_TYPE:
    case GRAVEBUSTER_TYPE:
    case HYPNOSHROOM_TYPE:
    case SCAREDYSHROOM_TYPE:
    case ICESHROOM_TYPE:
    case DOOMSHROOM_TYPE:
    case PLANTERN_TYPE:
    case MAGNETSHROOM_TYPE:
        return ALMANAC_GROUND_NIGHT_DIRECTORY;
    default:
        return ALMANAC_GROUND_DAY_DIRECTORY;
    }
}

// 200x200 crop of the debut level's backdrop (bg index = dir -
// BACKGROUND_LV1_DIRECTORY; lanes in image coords: flat yards 5x100 from y80
// -- the 1lane art sods only row 2 = 280..380, 3lane rows 1..3; pool/fog land
// rows 80..250 + 420..590, water rows 250..420; roof rows 70..495). `water`
// picks the pool water strip; the sy picks put the icon foot line (~196
// zombies, ~156 plants inside the 200px patch) onto the matching rows.
AlmanacBackdrop almanac_debut_backdrop(int debut_level, bool water)
{
    AlmanacBackdrop b;
    if (debut_level <= 0)
        return b;
    int dir = level_background_dir(debut_level);
    if (dir < 0)
        return b;
    b.dir = dir;
    b.custom = true;
    b.sx = 520; // lawn columns center (lawn x 260..980 -> 620)
    switch (dir - BACKGROUND_LV1_DIRECTORY)
    {
    case 4:
    case 5: // pool/fog
        b.sy = water ? 150 : 40; // feet on water rows resp. top land rows
        break;
    case 6:
    case 7: // roof (day/night): flat columns right of the slope crest
        b.sx = 720;
        b.sy = 180;
        break;
    default: // flat yards (1lane/3lane/full, day/night): mid lanes in view
        b.sy = 180;
        break;
    }
    return b;
}

AlmanacBackdrop almanac_plant_backdrop(int type)
{
    bool aquatic = type >= 0 && type < (int)PLANT_IS_AQUATIC.size() &&
                   PLANT_IS_AQUATIC[type];
    AlmanacBackdrop b = almanac_debut_backdrop(plant_debut_level(type), aquatic);
    if (!b.custom)
        b.dir = almanac_ground_for_plant(type);
    return b;
}

AlmanacBackdrop almanac_zombie_backdrop(int type)
{
    // Water-only zombies (ducky tube family, snorkel, dolphin rider) stand on
    // the water strip of their debut backdrop; pool/fog zombies that also
    // walk on land get the land strip.
    AlmanacBackdrop b = almanac_debut_backdrop(zombie_debut_level(type),
                                               zombie_water_only(type));
    if (!b.custom)
        b.dir = type == ZOMBONI_TYPE || type == BOBSLED_TYPE
                    ? ALMANAC_GROUND_ICE_DIRECTORY
                    : ALMANAC_GROUND_DAY_DIRECTORY;
    return b;
}

bool draw_plant_preview(int type, float clock, int center_x, int foot_y)
{
    if (type < 0 || type >= PLANT_COUNT)
        return false;
    if (!reanimplant::ready())
    {
        reanimplant::init(win.sprite_renderer());
        if (!reanimplant::ready())
            return false;
    }
    int idle = PLANT_IDLE_DIR[type];
    int n_sheet = all_img[idle].n_sheet > 0 ? all_img[idle].n_sheet : 60;
    return reanimplant::drawPlant(type, idle, (int)clock, n_sheet, clock,
                                  center_x - 40, center_x + 40, foot_y, 0, false);
}

// --- disco dancer: the real GOTY disco art ------------------------------------
// The in-game dancer doc (Zombie_dancer.reanim, a 2Pak conversion) is built
// on backup-dancer body parts -- no quiff, no glasses. The actual disco art
// only exists in 2Pak's unused zombie_jackson.reanim.compiled, converted to
// resources/reanim/Zombie_disco.reanim. Only the almanac draws it, so the doc
// lives here instead of in reanim_zombie. Draw scale matches the in-game
// dancer's on-screen height: its anim_walk bbox is 132 units tall x DANCER
// skin scale 0.860 = 113.5 px; across the loop the disco doc renders a union
// bbox ~155 units tall (measured on this production path via
// demo/almanac_calib_dump.cpp) -> x0.732.
namespace
{
reanim::ReanimDoc gDiscoDoc;
reanim::SdlReanimRenderer *gDiscoRenderer = nullptr;
bool gDiscoTried = false, gDiscoReady = false;
int gDiscoWalkFirst = 0, gDiscoWalkLast = 0; // seam-trimmed play window
const float DISCO_DRAW_SCALE = 113.5f / 155.0f;

void almanac_disco_init()
{
    if (gDiscoTried)
        return;
    gDiscoTried = true;
    std::string err;
    if (!reanim::LoadReanimXml("./resources/reanim/Zombie_disco.reanim",
                               gDiscoDoc, err))
    {
        SDL_Log("almanac: %s", err.c_str());
        return;
    }
    auto it = gDiscoDoc.segments.find("anim_walk");
    if (it == gDiscoDoc.segments.end())
        return;
    static reanim::SdlReanimRenderer sRenderer(win.sprite_renderer());
    gDiscoRenderer = &sRenderer;
    if (!gDiscoRenderer->BindDirectory("./resources/reanim"))
        return;
    int first = it->second.first, last = it->second.last;
    if (reanim::SegmentEndsOnLoopSeam(gDiscoDoc, first, last))
        last -= 1; // closed loop: the duplicate seam pose only wraps into
    gDiscoWalkFirst = first;
    gDiscoWalkLast = last;
    gDiscoReady = true;
}

// Doc-frame length of the disco walk loop, in 12fps clock units.
float almanac_disco_loop_length()
{
    almanac_disco_init();
    if (!gDiscoReady)
        return 0.0f;
    return (gDiscoWalkLast - gDiscoWalkFirst + 1) * 12.0f / gDiscoDoc.fps;
}

void almanac_draw_disco(float clock, int x_location, int y_location, float scale)
{
    almanac_disco_init();
    if (!gDiscoReady)
        return;
    float len = (float)(gDiscoWalkLast - gDiscoWalkFirst + 1);
    float local = clock < 0 ? 0 : clock;
    local *= gDiscoDoc.fps / 12.0f; // clock runs in 12fps units
    local = fmodf(local, len);
    reanim::DrawOptions opt;
    opt.loopWrapFirst = gDiscoWalkFirst;
    opt.loopWrapLast = gDiscoWalkLast;
    gDiscoRenderer->Draw(gDiscoDoc, gDiscoWalkFirst + local, (float)x_location,
                         (float)y_location, DISCO_DRAW_SCALE * scale, nullptr,
                         opt);
}
} // namespace

// Pose-space crop rect of a zombie's almanac pose; false when the type draws
// its full pose (the union bbox is the effective bbox then).
bool almanac_zombie_crop(int type, float &l, float &t, float &r, float &b)
{
    if (type < 0 || type >= COUNT_ZOMBIE_TYPE || !ZOMBIE_RECIPE[type].crop_on)
        return false;
    const AlmanacZombie &z = ZOMBIE_RECIPE[type];
    l = z.crop_l;
    t = z.crop_t;
    r = z.crop_r;
    b = z.crop_b;
    return true;
}

// The crop rect mapped into window coords for a resolved layout: the rect
// is PIVOT-space, so a pose-space point P renders at (draw pivot + P*scale)
// -- the probe pivot (the draw's scale-invariant anchor), boss-compensated
// for the Zomboss entry (see almanac_pose_geom).
bool almanac_zombie_clip_rect(int type, const AlmanacZombieLayout &l,
                              SDL_Rect &clip)
{
    float cl = 0, ct = 0, cr = 0, cb = 0;
    if (!almanac_zombie_crop(type, cl, ct, cr, cb))
        return false;
    int x_location = 0, y_location = 0;
    almanac_zombie_draw_args(type, l, x_location, y_location);
    AlmanacPoseGeom g;
    if (!almanac_pose_geom(type, l.scale, g))
        return false;
    float px = (float)x_location + g.pivotX;
    float py = (float)y_location + g.pivotY;
    clip.x = (int)floorf(px + cl * l.scale);
    clip.y = (int)floorf(py + ct * l.scale);
    clip.w = (int)ceilf(px + cr * l.scale) - clip.x;
    clip.h = (int)ceilf(py + cb * l.scale) - clip.y;
    return true;
}

// Draw the zombie's almanac pose at a resolved layout (bottom-center anchored:
// bbox centered on layout.anchor_x, art bottom on layout.anchor_y -- for the
// cropped types, the CROP RECT centered/bottom). The pose is the walk reanim
// by default; ZOMBIE_RECIPE can swap in a chosen segment (pole vaulter with
// his pole, the balloon's idle-only balloon), the disco doc, or the Zomboss
// head + dome driver overlay. Everything is scissored: cropped types to their
// mapped crop rect, then to `contain` (the grid cell / preview patch) when
// given -- so no art can spill its container regardless of the loop frame.
void almanac_draw_zombie_icon(int type, float clock, const AlmanacZombieLayout &l,
                              const SDL_Rect *contain)
{
    if (type < 0 || type >= COUNT_ZOMBIE_TYPE)
        return;
    if (!reanimz::ready())
    {
        reanimz::init(win.sprite_renderer());
        if (!reanimz::ready())
            return;
    }
    const AlmanacZombie &r = ZOMBIE_RECIPE[type];
    int x_location = 0, y_location = 0;
    almanac_zombie_draw_args(type, l, x_location, y_location);

    SDL_Rect clip;
    bool have_clip = false;
    if (almanac_zombie_clip_rect(type, l, clip))
        have_clip = true;
    if (contain)
    {
        if (!have_clip)
        {
            clip = *contain;
            have_clip = true;
        }
        else
        {
            int x1 = std::max(clip.x, contain->x), y1 = std::max(clip.y, contain->y);
            int x2 = std::min(clip.x + clip.w, contain->x + contain->w);
            int y2 = std::min(clip.y + clip.h, contain->y + contain->h);
            clip = SDL_Rect{x1, y1, std::max(0, x2 - x1), std::max(0, y2 - y1)};
            have_clip = clip.w > 0 && clip.h > 0;
        }
    }
    if (have_clip)
        win.sprite_batch().setClipRect(&clip);

    reanimz::setZombieTimeMul(1.0f);
    switch (r.pose)
    {
    case AZ_POSE_SEGMENT:
        reanimz::setZombieScaleMul(1.0f);
        // Full segment render -- the crop-rect scissor (not a track hide)
        // frames the vignette for catapult/gargantuar.
        reanimz::drawSkinSegment(r.skin, r.seg, clock, x_location, y_location,
                                 false, false, 0, 0, l.scale, true, nullptr);
        break;
    case AZ_POSE_ZOMBOSS:
        // Head-attack loop: head track subset (no body in front of/behind the
        // dome) + the doctor overlay riding the boss_head track. The body
        // draw passes the boss doc compensation as ddx/ddy (the retail pivot
        // centers the doc's empty space -- see reanimz::bossDocCompensation);
        // the driver anchors itself the same way, so the pair is shared.
        reanimz::setZombieScaleMul(1.0f);
        {
            int ddx, ddy;
            reanimz::bossDocCompensation(l.scale, ddx, ddy);
            reanimz::drawSkinSegment(reanimz::SKIN_ZOMBOSS, "anim_head_idle", clock,
                                     x_location, y_location, false, false,
                                     ddx, ddy, l.scale, true, &kBossBodyCrop);
        }
        bossdriver::draw("anim_head_idle", clock, true, x_location, y_location,
                         l.scale, false, false);
        break;
    case AZ_POSE_DISCO:
        almanac_draw_disco(clock, x_location, y_location, l.scale);
        break;
    default:
        reanimz::setZombieScaleMul(l.scale);
        reanimz::drawZombie(r.walk_dir, r.armor_dir, clock, x_location, y_location,
                            false, false);
        reanimz::setZombieScaleMul(1.0f);
        break;
    }
    if (have_clip)
        win.sprite_batch().setClipRect(nullptr);
    reanimz::setZombieTimeMul(1.0f);
}

// Loop length of the zombie's almanac pose in 12fps clock units; the layout
// check samples one full loop with it.
float almanac_zombie_pose_loop_length(int type)
{
    if (type < 0 || type >= COUNT_ZOMBIE_TYPE)
        return 0.0f;
    if (!reanimz::ready())
    {
        reanimz::init(win.sprite_renderer());
        if (!reanimz::ready())
            return 0.0f;
    }
    const AlmanacZombie &r = ZOMBIE_RECIPE[type];
    switch (r.pose)
    {
    case AZ_POSE_SEGMENT:
        return reanimz::skinSegmentLength(r.skin, r.seg);
    case AZ_POSE_ZOMBOSS:
        return reanimz::skinSegmentLength(reanimz::SKIN_ZOMBOSS, "anim_head_idle");
    case AZ_POSE_DISCO:
        return almanac_disco_loop_length();
    default:
        return reanimz::segmentLength(r.walk_dir);
    }
}

AlmanacZombieLayout almanac_zombie_grid_layout(int type, int cell_x, int cell_y)
{
    // Window image is 100x100 at (cell_x, cell_y) (the original 76x80 art
    // scaled x100/76); the zombie stands on the glass bottom (y+93; same 74/80
    // ratio upscaled), centered on the window (x+50), at the ONE common draw
    // scale ALMANAC_ZOMBIE_DRAW_SCALE (sized so a normal zombie reads ~75px
    // tall like the old 76x80 windows; bigger arts scissor into the glass).
    return almanac_zombie_place(type, cell_x + 50, cell_y + 93);
}

AlmanacZombieLayout almanac_zombie_preview_layout(int type)
{
    // Preview patch: original card window spans (525..705, 103..323); the
    // zombie stands bottom-center of the ground strip, at the SAME common
    // draw scale as the grid -- the vignette keeps the zombies' natural
    // relative sizes instead of up-scaling each to the patch.
    return almanac_zombie_place(type, ALMANAC_X_OFF + 615, 306.0f);
}

void almanac_zombie_draw_args(int type, const AlmanacZombieLayout &l,
                              int &x_location, int &y_location)
{
    const AlmanacZombie &r = ZOMBIE_RECIPE[type];
    if (r.pose == AZ_POSE_DISCO)
    {
        // Hand-fitted numbers: the disco doc draws raw at (x, y) (its own
        // namespace below, DISCO_DRAW_SCALE folded into the offsets), so the
        // recipe's center/foot offsets map straight onto the anchor.
        x_location = (int)(l.anchor_x - r.cx_off * l.scale + 0.5f);
        y_location = (int)(l.anchor_y - r.foot_off * l.scale + 0.5f);
        return;
    }
    // Probe placement (see almanac_pose_geom): the pose's bbox center lands
    // on anchor_x and its bottom on anchor_y -- x_location/y_location feed
    // drawZombie / drawSkinSegment directly (the retail pivot anchors the
    // art, so the same pair works for walk, segment and boss poses; the
    // boss doc compensation is already folded into the probed pivot).
    AlmanacPoseGeom g;
    if (!almanac_pose_geom(type, l.scale, g))
    {
        x_location = 0;
        y_location = 0;
        return;
    }
    x_location = (int)(l.anchor_x - g.pivotX - g.C * l.scale + 0.5f);
    y_location = (int)(l.anchor_y - g.pivotY - g.B * l.scale + 0.5f);
}

void draw_almanac_button(const Button &b, int label_dir, int highlight_dir,
                         const string &label)
{
    int _x = 0, _y = 0;
    SDL_GetMouseState(&_x, &_y);
    b.display(b.is_mouse_in(_x, _y) ? highlight_dir : label_dir);
    // Original dialog GameButtons use FONT_BRIANNETOD12 for their labels.
    int w = 0, h = 0;
    TTF_SizeText(win.get_font(BRIANNE_TTF, 12), label.c_str(), &w, &h);
    win.show_text(label, (b.x1 + b.x2 - w) / 2, (b.y1 + b.y2 - h) / 2,
                  ALMANAC_BUTTON_LABEL, BRIANNE_TTF, 12);
}

// ---------------------------------------------------------------------------
// Index page: sunflower + normal zombie teasing the two collections, with the
// VIEW PLANTS / VIEW ZOMBIES buttons of the original layout.
// ---------------------------------------------------------------------------
void almanac_draw_index(const AlmanacState &s)
{
    win.clear_renderer();
    win.draw_png(ALMANAC_INDEX_BACK_DIRECTORY, ALMANAC_X_OFF, 0, 800, 600);
    // Original index title (repvz Almanac.cpp): [SUBURBAN_ALMANAC_INDEX],
    // FONT_HOUSEOFTERROR28, (220,220,220) - the image font's black outline.
    show_text_middle("SUBURBAN ALMANAC", ALMANAC_X_OFF + 400, 34,
                     ALMANAC_TITLE_GRAY, HOUSEOFTERROR_TTF, 28, 2, BLACK);

    // Animated mascots on the two book piles: sunflower on the left pile,
    // the normal zombie standing on the right one (original index spans
    // y~216..345; art bottom-center anchored).
    draw_plant_preview(SUNFLOWER_TYPE, s.clock, ALMANAC_X_OFF + 167 + 30, 340);
    {
        // Index page mascot: NOT a thumbnail -- the normal zombie teases the
        // zombie page at full in-game scale (the one place the common almanac
        // scale does not apply).
        AlmanacZombieLayout l = almanac_zombie_place(NORMAL_TYPE, ALMANAC_X_OFF + 582,
                                                     343.0f, 1.0f);
        almanac_draw_zombie_icon(NORMAL_TYPE, s.clock, l);
    }

    for (const auto &bt : {make_pair(ALMANAC_VIEW_PLANTS, string("VIEW PLANTS")),
                           make_pair(ALMANAC_VIEW_ZOMBIES, string("VIEW ZOMBIES"))})
    {
        const Button &b = bt.first;
        b.display(SEEDCHOOSER_BUTTON_DIRECTORY);
        int _x = 0, _y = 0;
        SDL_GetMouseState(&_x, &_y);
        if (b.is_mouse_in(_x, _y))
            b.display(SEEDCHOOSER_BUTTON_GLOW_DIRECTORY);
        int w = 0, h = 0;
        TTF_SizeText(win.get_font(DWARVESC_TTF, 24), bt.second.c_str(), &w, &h);
        win.show_text(bt.second, (b.x1 + b.x2 - w) / 2, (b.y1 + b.y2 - h) / 2 - 2,
                      WHITE, DWARVESC_TTF, 24);
    }
    draw_almanac_button(ALMANAC_CLOSE, ALMANAC_CLOSE_BUTTON_DIRECTORY,
                        ALMANAC_CLOSE_BUTTON_HIGHLIGHT_DIRECTORY, "CLOSE");
}

// ---------------------------------------------------------------------------
// Plants page: grid of unlocked seed packets at the pre-game chooser size
// (105x65, 4 columns x 7 visible rows left of the detail card, the rest
// scrolled by wheel / slim scrollbar) + the selected plant's detail card
// (debut-level backdrop patch, animated reanim preview, name/desc/cost).
// ---------------------------------------------------------------------------
int almanac_plant_max_scroll_row()
{
    int rows = (PLANT_COUNT + ALMANAC_PLANT_COLS - 1) / ALMANAC_PLANT_COLS;
    return std::max(0, rows - ALMANAC_PLANT_VIS_ROWS);
}

bool almanac_plant_row_visible(int type, int scroll_row)
{
    int row = type / ALMANAC_PLANT_COLS;
    return row >= scroll_row && row < scroll_row + ALMANAC_PLANT_VIS_ROWS;
}

void almanac_plant_cell(int type, int scroll_row, int &x, int &y)
{
    x = ALMANAC_PLANT_GRID_X + (type % ALMANAC_PLANT_COLS) * ALMANAC_PLANT_CELL_DX;
    y = ALMANAC_PLANT_GRID_Y + (type / ALMANAC_PLANT_COLS - scroll_row) * ALMANAC_PLANT_CELL_DY;
}

// View rect of the grid (cell area + scrollbar strip), for wheel hit-testing.
Button almanac_plants_view_rect()
{
    int right = ALMANAC_PLANT_GRID_X + (ALMANAC_PLANT_COLS - 1) * ALMANAC_PLANT_CELL_DX +
                ALMANAC_PLANT_CELL_W;
    int bottom = ALMANAC_PLANT_GRID_Y + (ALMANAC_PLANT_VIS_ROWS - 1) * ALMANAC_PLANT_CELL_DY +
                 ALMANAC_PLANT_CELL_H;
    // View spans the cells plus the scrollbar strip; the track hugs the
    // cells' right edge with a 5px gap (card face starts at 584, clear).
    return Button(ALMANAC_PLANT_GRID_X, right + 5 + ALMANAC_SCROLL_TRACK_W,
                  ALMANAC_PLANT_GRID_Y, bottom);
}

bool almanac_plants_scrollbar(int scroll_row, SDL_Rect &track, SDL_Rect &thumb)
{
    int max_row = almanac_plant_max_scroll_row();
    if (max_row <= 0)
        return false;
    Button view = almanac_plants_view_rect();
    track.x = view.x2 - ALMANAC_SCROLL_TRACK_W;
    track.y = view.y1;
    track.w = ALMANAC_SCROLL_TRACK_W;
    track.h = view.y2 - view.y1;
    int travel = track.h - ALMANAC_SCROLL_THUMB;
    thumb.x = track.x + (track.w - ALMANAC_SCROLL_THUMB) / 2;
    thumb.y = track.y + travel * scroll_row / max_row;
    thumb.w = ALMANAC_SCROLL_THUMB;
    thumb.h = ALMANAC_SCROLL_THUMB;
    return true;
}

int almanac_scroll_row_at(int thumb_top)
{
    SDL_Rect track, thumb;
    if (!almanac_plants_scrollbar(0, track, thumb))
        return 0;
    int max_row = almanac_plant_max_scroll_row();
    int row = (int)lroundf((thumb_top - track.y) * max_row /
                           (float)(track.h - thumb.h));
    return std::min(max_row, std::max(0, row));
}

// Slim vertical bar from the 2Pak options art, shared by both almanac grids
// (definition in the zombies-page block below): the 135x10 slider groove
// rotated 90 into the track + the slider knob's opaque core as the thumb.
void almanac_draw_slim_scrollbar(const SDL_Rect &track, const SDL_Rect &thumb);

void almanac_draw_plants_scrollbar(int scroll_row)
{
    SDL_Rect track, thumb;
    if (!almanac_plants_scrollbar(scroll_row, track, thumb))
        return;
    almanac_draw_slim_scrollbar(track, thumb);
}

void almanac_draw_plants(const AlmanacState &s)
{
    win.clear_renderer();
    win.draw_png(ALMANAC_PLANTBACK_DIRECTORY, ALMANAC_X_OFF, 0, 800, 600);
    // Original plants-page title (repvz Almanac.cpp): [SUBURBAN_ALMANAC_PLANTS],
    // FONT_HOUSEOFTERROR20, (213,159,43) over the baked black outline.
    show_text_middle("SUBURBAN ALMANAC - PLANTS", ALMANAC_X_OFF + 400, 34,
                     ALMANAC_TITLE_GOLD, HOUSEOFTERROR_TTF, 20, 2, BLACK);

    int mx = 0, my = 0;
    SDL_GetMouseState(&mx, &my);
    for (int type = 0; type < PLANT_COUNT; type++)
    {
        if (!is_plant_unlocked(type))
            continue; // original shows nothing for locked entries
        if (!almanac_plant_row_visible(type, s.scroll_row))
            continue; // scrolled out of the view
        int x, y;
        almanac_plant_cell(type, s.scroll_row, x, y);
        // Same packet look as the pre-game chooser (its draw_packet_scaled):
        // the plant's debut-world packet background (plant_packet_bg_dir) +
        // the shared-scale portrait + the bottom border strip.
        win.draw_png(plant_packet_bg_dir(type), x, y, ALMANAC_PLANT_CELL_W,
                     ALMANAC_PLANT_CELL_H);
        almanac_draw_plant_icon(type, x, y);
        win.draw_png_height_scaled(SEED_PACKET_BOT_BORDER_DIRECTORY, x, y + 60,
                                   ALMANAC_PLANT_CELL_W);
        if (type == s.selected_plant ||
            Button(x, x + ALMANAC_PLANT_CELL_W, y, y + ALMANAC_PLANT_CELL_H).is_mouse_in(mx, my))
            win.draw_png(BUTTON_BLINK_DIRECTORY, x, y, ALMANAC_PLANT_CELL_W,
                         ALMANAC_PLANT_CELL_H);
    }
    almanac_draw_plants_scrollbar(s.scroll_row);

    if (s.selected_plant >= 0)
    {
        int t = s.selected_plant;
        // Debut-level backdrop (crop of the 1400x600 level art; a 200x200
        // crop of the stock 200x200 patch equals the old full draw).
        AlmanacBackdrop pb = almanac_plant_backdrop(t);
        win.draw_png(pb.dir, pb.sx, pb.sy, 200, 200, ALMANAC_X_OFF + 521, 107,
                     200, 200);
        int foot_bias = 0;
        if (t == GRAVEBUSTER_TYPE)
            foot_bias = 45;
        else if (t == TALLNUT_TYPE)
            foot_bias = 15;
        else if (t == FLOWERPOT_TYPE)
            foot_bias = -20;
        else if (t == COFFEEBEAN_TYPE)
            foot_bias = 10;
        draw_plant_preview(t, s.clock, ALMANAC_X_OFF + 621, 263 + foot_bias);
        win.draw_png(ALMANAC_PLANT_CARD_DIRECTORY, ALMANAC_X_OFF + 459, 86, 341, 494);
        show_text_middle(almanac_plant_name(t), ALMANAC_X_OFF + 617, 274,
                         YELLOW_A, DWARVESC_TTF, 18);
        // Original body font: FONT_BRIANNETOD12 inside Rect(485,309,258,210).
        show_text_wrapped(almanac_plant_desc(t), ALMANAC_X_OFF + 485, 309, 258,
                          ALMANAC_TEXT_BLUE, BRIANNE_TTF, 12);
        win.show_text("COST: " + to_string(PLANT_SUN_COST[t]), ALMANAC_X_OFF + 485,
                      520, WHITE, BRIANNE_TTF, 12);
        {
            string rech = "RECHARGE: " + almanac_recharge_text(PLANT_LOADING_TIME[t]);
            int rw = 0, rh = 0;
            TTF_SizeText(win.get_font(BRIANNE_TTF, 12), rech.c_str(), &rw, &rh);
            win.show_text(rech, ALMANAC_X_OFF + 600 + 139 - rw, 520,
                          ALMANAC_TEXT_BLUE, BRIANNE_TTF, 12);
        }
    }

    draw_almanac_button(ALMANAC_BACK_INDEX, ALMANAC_INDEX_BUTTON_DIRECTORY,
                        ALMANAC_INDEX_BUTTON_HIGHLIGHT_DIRECTORY, "ALMANAC INDEX");
    draw_almanac_button(ALMANAC_CLOSE, ALMANAC_CLOSE_BUTTON_DIRECTORY,
                        ALMANAC_CLOSE_BUTTON_HIGHLIGHT_DIRECTORY, "CLOSE");
}

// ---------------------------------------------------------------------------
// Zombies page: 4-column grid of BIG zombie windows (100x100, the original
// 76x80 ZombieWindow art scaled up, cells touch) inside the ZombieBack panel
// + detail card. 26 zombies = 7 rows, 4 visible; the rest scroll in by whole
// rows via the wheel / slim scrollbar -- the same idiom as the plants page.
// All 26 entries share the plain grid (zomboss included: its head vignette
// fits a normal wide cell), which puts the YETI on the last row WITH the
// zomboss instead of dangling in its own row next to a wide boss slot.
// ---------------------------------------------------------------------------
void almanac_zombie_cell(int type, int scroll_row, int &x, int &y)
{
    x = ALMANAC_ZOMBIE_GRID_X + (type % ALMANAC_ZOMBIE_COLS) * ALMANAC_ZOMBIE_CELL_DX;
    y = ALMANAC_ZOMBIE_GRID_Y + (type / ALMANAC_ZOMBIE_COLS - scroll_row) * ALMANAC_ZOMBIE_CELL_DY;
}

int almanac_zombie_max_scroll_row()
{
    int rows = (COUNT_ZOMBIE_TYPE + ALMANAC_ZOMBIE_COLS - 1) / ALMANAC_ZOMBIE_COLS;
    return std::max(0, rows - ALMANAC_ZOMBIE_VIS_ROWS);
}

bool almanac_zombie_row_visible(int type, int scroll_row)
{
    int row = type / ALMANAC_ZOMBIE_COLS;
    return row >= scroll_row && row < scroll_row + ALMANAC_ZOMBIE_VIS_ROWS;
}

// View rect of the grid (cell area + scrollbar strip), for wheel hit-testing.
Button almanac_zombies_view_rect()
{
    int right = ALMANAC_ZOMBIE_GRID_X + (ALMANAC_ZOMBIE_COLS - 1) * ALMANAC_ZOMBIE_CELL_DX +
                ALMANAC_ZOMBIE_CELL_W;
    int bottom = ALMANAC_ZOMBIE_GRID_Y + (ALMANAC_ZOMBIE_VIS_ROWS - 1) * ALMANAC_ZOMBIE_CELL_DY +
                 ALMANAC_ZOMBIE_CELL_H;
    return Button(ALMANAC_ZOMBIE_GRID_X, right + 5 + ALMANAC_SCROLL_TRACK_W,
                  ALMANAC_ZOMBIE_GRID_Y, bottom);
}

bool almanac_zombies_scrollbar(int scroll_row, SDL_Rect &track, SDL_Rect &thumb)
{
    int max_row = almanac_zombie_max_scroll_row();
    if (max_row <= 0)
        return false;
    Button view = almanac_zombies_view_rect();
    track.x = view.x2 - ALMANAC_SCROLL_TRACK_W;
    track.y = view.y1;
    track.w = ALMANAC_SCROLL_TRACK_W;
    track.h = view.y2 - view.y1;
    int travel = track.h - ALMANAC_SCROLL_THUMB;
    thumb.x = track.x + (track.w - ALMANAC_SCROLL_THUMB) / 2;
    thumb.y = track.y + travel * scroll_row / max_row;
    thumb.w = ALMANAC_SCROLL_THUMB;
    thumb.h = ALMANAC_SCROLL_THUMB;
    return true;
}

int almanac_zombies_scroll_row_at(int thumb_top)
{
    SDL_Rect track, thumb;
    if (!almanac_zombies_scrollbar(0, track, thumb))
        return 0;
    int max_row = almanac_zombie_max_scroll_row();
    int row = (int)lroundf((thumb_top - track.y) * max_row /
                           (float)(track.h - thumb.h));
    return std::min(max_row, std::max(0, row));
}

// Slim vertical bar from the 2Pak options art: the 135x10 slider groove
// rotated 90 into the track (its shading then runs across the bar) and the
// slider knob's opaque core (24x24 at (0,2)) cropped square as the thumb.
// Shared by both almanac grids' scrollbars.
void almanac_draw_slim_scrollbar(const SDL_Rect &track, const SDL_Rect &thumb)
{
    int cx = track.x + track.w / 2, cy = track.y + track.h / 2;
    win.draw_png(ALMANAC_SLIDERSLOT_DIRECTORY, cx - track.h / 2, cy - track.w / 2,
                 track.h, track.w, 90);
    win.draw_png(SLIDER_DIRECTORY, 0, 2, 24, 24, thumb.x, thumb.y, thumb.w, thumb.h);
}

void almanac_draw_zombies(const AlmanacState &s)
{
    win.clear_renderer();
    win.draw_png(ALMANAC_ZOMBIEBACK_DIRECTORY, ALMANAC_X_OFF, 0, 800, 600);
    // Original zombies-page title (repvz Almanac.cpp): [SUBURBAN_ALMANAC_ZOMBIES],
    // FONT_DWARVENTODCRAFT24, (0,196,0) - green fill, black outline.
    show_text_middle("SUBURBAN ALMANAC - ZOMBIES", ALMANAC_X_OFF + 400, 34,
                     ALMANAC_TITLE_GREEN, DWARVENTODCRAFT_TTF, 24, 2, BLACK);

    int mx = 0, my = 0;
    SDL_GetMouseState(&mx, &my);
    // Original (un-modded clone keeps) all zombies visible: there is no
    // encounter tracking in the clone's player data. Hover brightens.
    for (int type = 0; type < COUNT_ZOMBIE_TYPE; type++)
    {
        if (!almanac_zombie_row_visible(type, s.zscroll_row))
            continue; // scrolled out of the view
        int x, y;
        almanac_zombie_cell(type, s.zscroll_row, x, y);
        win.draw_png(ALMANAC_ZOMBIE_WINDOW_DIRECTORY, x, y, ALMANAC_ZOMBIE_CELL_W,
                     ALMANAC_ZOMBIE_CELL_H);
        // The vignette is scissored to the window glass: full-pose crops for
        // the four big arts, plain cell clip for everyone else (the art is
        // already bbox-fitted at the shared scale; the clip only guards anim
        // overshoot like the zomboni brush bounce).
        SDL_Rect contain{x + 2, y + 2, ALMANAC_ZOMBIE_CELL_W - 4,
                         ALMANAC_ZOMBIE_CELL_H - 4};
        almanac_draw_zombie_icon(type, s.clock + type * 7.0f,
                                 almanac_zombie_grid_layout(type, x, y), &contain);
        // Glass frame drawn over the zombie (window2 85x85 around the 76x80
        // window, same top-left, scaled by the same 100/76 factor: it
        // overlaps the neighbors a touch, as authored).
        win.draw_png(ALMANAC_ZOMBIE_WINDOW2_DIRECTORY, x, y, 112, 106);
        if (Button(x, x + ALMANAC_ZOMBIE_CELL_W, y, y + ALMANAC_ZOMBIE_CELL_H)
                .is_mouse_in(mx, my))
            win.draw_png(BUTTON_BLINK_DIRECTORY, x, y, ALMANAC_ZOMBIE_CELL_W,
                         ALMANAC_ZOMBIE_CELL_H);
    }
    {
        SDL_Rect track, thumb;
        if (almanac_zombies_scrollbar(s.zscroll_row, track, thumb))
            almanac_draw_slim_scrollbar(track, thumb);
    }

    if (s.selected_zombie >= 0)
    {
        int t = s.selected_zombie;
        AlmanacBackdrop zb = almanac_zombie_backdrop(t);
        win.draw_png(zb.dir, zb.sx, zb.sy, 200, 200, ALMANAC_X_OFF + 518, 110,
                     200, 200);
        SDL_Rect patch{ALMANAC_X_OFF + 518, 110, 200, 200};
        almanac_draw_zombie_icon(t, s.clock, almanac_zombie_preview_layout(t),
                                 &patch);
        win.draw_png(ALMANAC_ZOMBIE_CARD_DIRECTORY, ALMANAC_X_OFF + 455, 78, 324, 497);
        show_text_middle(almanac_zombie_name(t), ALMANAC_X_OFF + 613, 348,
                         ALMANAC_ZOMBIE_NAME, DWARVESC_TTF, 18);
        // Original body font FONT_BRIANNETOD12 inside Rect(485,377,257,160).
        show_text_wrapped(almanac_zombie_desc(t), ALMANAC_X_OFF + 485, 377, 258,
                          ALMANAC_TEXT_BLUE, BRIANNE_TTF, 12);
    }

    draw_almanac_button(ALMANAC_BACK_INDEX, ALMANAC_INDEX_BUTTON_DIRECTORY,
                        ALMANAC_INDEX_BUTTON_HIGHLIGHT_DIRECTORY, "ALMANAC INDEX");
    draw_almanac_button(ALMANAC_CLOSE, ALMANAC_CLOSE_BUTTON_DIRECTORY,
                        ALMANAC_CLOSE_BUTTON_HIGHLIGHT_DIRECTORY, "CLOSE");
}

void almanac_click(AlmanacState &s, int mouse_x, int mouse_y)
{
    if (ALMANAC_CLOSE.is_mouse_in(mouse_x, mouse_y))
    {
        play_sound_effect(BUTTON_CLICK_MUSIC_DIRECTORY);
        s.done = true;
        return;
    }
    if (s.page == ALMANAC_PAGE_INDEX)
    {
        if (ALMANAC_VIEW_PLANTS.is_mouse_in(mouse_x, mouse_y))
        {
            play_sound_effect(BUTTON_CLICK_MUSIC_DIRECTORY);
            s.page = ALMANAC_PAGE_PLANTS;
            s.selected_plant = -1;
            s.scroll_row = 0;
            return;
        }
        if (ALMANAC_VIEW_ZOMBIES.is_mouse_in(mouse_x, mouse_y))
        {
            play_sound_effect(BUTTON_CLICK_MUSIC_DIRECTORY);
            s.page = ALMANAC_PAGE_ZOMBIES;
            s.selected_zombie = -1;
            s.zscroll_row = 0;
            return;
        }
        return;
    }
    if (ALMANAC_BACK_INDEX.is_mouse_in(mouse_x, mouse_y))
    {
        play_sound_effect(BUTTON_CLICK_MUSIC_DIRECTORY);
        s.page = ALMANAC_PAGE_INDEX;
        s.scroll_drag = false;
        return;
    }
    if (s.page == ALMANAC_PAGE_PLANTS)
    {
        SDL_Rect track, thumb;
        if (almanac_plants_scrollbar(s.scroll_row, track, thumb))
        {
            Button thumb_b(thumb.x - 3, thumb.x + thumb.w + 3,
                           thumb.y - 3, thumb.y + thumb.h + 3);
            if (thumb_b.is_mouse_in(mouse_x, mouse_y))
            {
                s.scroll_drag = true;
                s.scroll_grab_dy = mouse_y - thumb.y;
                return;
            }
            Button track_b(track.x - 2, track.x + track.w + 2, track.y,
                           track.y + track.h);
            if (track_b.is_mouse_in(mouse_x, mouse_y))
            {
                // Center the thumb under the cursor and drag on from there.
                s.scroll_grab_dy = ALMANAC_SCROLL_THUMB / 2;
                s.scroll_row = almanac_scroll_row_at(mouse_y - s.scroll_grab_dy);
                s.scroll_drag = true;
                return;
            }
        }
        for (int type = 0; type < PLANT_COUNT; type++)
        {
            if (!is_plant_unlocked(type))
                continue;
            if (!almanac_plant_row_visible(type, s.scroll_row))
                continue;
            int x, y;
            almanac_plant_cell(type, s.scroll_row, x, y);
            if (Button(x, x + ALMANAC_PLANT_CELL_W, y, y + ALMANAC_PLANT_CELL_H)
                    .is_mouse_in(mouse_x, mouse_y))
            {
                play_sound_effect(BUTTON_CLICK_MUSIC_DIRECTORY);
                s.selected_plant = type;
                return;
            }
        }
    }
    else
    {
        SDL_Rect track, thumb;
        if (almanac_zombies_scrollbar(s.zscroll_row, track, thumb))
        {
            Button thumb_b(thumb.x - 3, thumb.x + thumb.w + 3,
                           thumb.y - 3, thumb.y + thumb.h + 3);
            if (thumb_b.is_mouse_in(mouse_x, mouse_y))
            {
                s.scroll_drag = true;
                s.scroll_grab_dy = mouse_y - thumb.y;
                return;
            }
            Button track_b(track.x - 2, track.x + track.w + 2, track.y,
                           track.y + track.h);
            if (track_b.is_mouse_in(mouse_x, mouse_y))
            {
                // Center the thumb under the cursor and drag on from there.
                s.scroll_grab_dy = ALMANAC_SCROLL_THUMB / 2;
                s.zscroll_row = almanac_zombies_scroll_row_at(mouse_y - s.scroll_grab_dy);
                s.scroll_drag = true;
                return;
            }
        }
        for (int type = 0; type < COUNT_ZOMBIE_TYPE; type++)
        {
            if (!almanac_zombie_row_visible(type, s.zscroll_row))
                continue;
            int x, y;
            almanac_zombie_cell(type, s.zscroll_row, x, y);
            if (Button(x, x + ALMANAC_ZOMBIE_CELL_W, y, y + ALMANAC_ZOMBIE_CELL_H)
                    .is_mouse_in(mouse_x, mouse_y))
            {
                play_sound_effect(BUTTON_CLICK_MUSIC_DIRECTORY);
                s.selected_zombie = type;
                return;
            }
        }
    }
}

/*
Suburban Almanac scene (blocking, like display_starting_screen): index page
with plants/zombies entries, per-entry detail card with the live reanim.
*/
void run_almanac()
{
    AlmanacState s;
    while (!quit && !s.done)
    {
        if (s.page == ALMANAC_PAGE_INDEX)
            almanac_draw_index(s);
        else if (s.page == ALMANAC_PAGE_PLANTS)
            almanac_draw_plants(s);
        else
            almanac_draw_zombies(s);
        win.update_screen();
        HANDLE_SDL_EVENT(
            QUIT();
            LCLICK({
                almanac_click(s, mouse_x, mouse_y);
            });
            LRELEASE({
                s.scroll_drag = false;
                (void)mouse_x;
                (void)mouse_y;
            });
            if (e.type == SDL_MOUSEWHEEL)
            {
                // One cell row per notch while the cursor is over the grid --
                // both grids scroll the same way.
                int wx = 0;
                int wy = 0;
                SDL_GetMouseState(&wx, &wy);
                if (s.page == ALMANAC_PAGE_PLANTS &&
                    almanac_plants_view_rect().is_mouse_in(wx, wy))
                {
                    int max_row = almanac_plant_max_scroll_row();
                    s.scroll_row -= e.wheel.y;
                    if (s.scroll_row < 0)
                        s.scroll_row = 0;
                    if (s.scroll_row > max_row)
                        s.scroll_row = max_row;
                }
                else if (s.page == ALMANAC_PAGE_ZOMBIES &&
                         almanac_zombies_view_rect().is_mouse_in(wx, wy))
                {
                    int max_row = almanac_zombie_max_scroll_row();
                    s.zscroll_row -= e.wheel.y;
                    if (s.zscroll_row < 0)
                        s.zscroll_row = 0;
                    if (s.zscroll_row > max_row)
                        s.zscroll_row = max_row;
                }
            }
            if (e.type == SDL_MOUSEMOTION && s.scroll_drag)
            {
                // The drag starts from the active page's own scrollbar.
                if (s.page == ALMANAC_PAGE_PLANTS)
                    s.scroll_row = almanac_scroll_row_at(e.motion.y - s.scroll_grab_dy);
                else if (s.page == ALMANAC_PAGE_ZOMBIES)
                    s.zscroll_row =
                        almanac_zombies_scroll_row_at(e.motion.y - s.scroll_grab_dy);
            }
        );
        s.clock += 12.0f / FPS;
    }
    win.fade_out();
    win.clear_renderer();
}
