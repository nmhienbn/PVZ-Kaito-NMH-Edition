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

// The almanac plant portrait is the original's per-seed mX/mY cell: the plant
// is drawn exactly like an in-game plant whose cell top-left is this point
// (reference AlmanacDialog::SetupPlant, AlmanacDialog.cpp:143-156). All in
// original 800x600 coords; the page adds ALMANAC_X_OFF to x.
void almanac_plant_detail_cell(int type, int &mx, int &my)
{
    mx = 578;
    my = 140;
    switch (type)
    {
    case TALLNUT_TYPE:      my += 18;  break;
    case FLOWERPOT_TYPE:    my -= 20;  break;
    case COFFEEBEAN_TYPE:   my += 20;  break; // original SEED_INSTANT_COFFEE
    case GRAVEBUSTER_TYPE:  my += 55;  break;
    default: break;
    }
}

// Port of PlantDrawHeightOffset with theBoard == nullptr (reference
// Plant.cpp:3537-3669): only the aquatic bob and the type constants apply --
// no flower pot, no roof/pool board queries.
float almanac_plant_height_offset(int type)
{
    float height = 0.0f;
    const bool aquatic = type >= 0 && type < (int)PLANT_IS_AQUATIC.size() &&
                         PLANT_IS_AQUATIC[type];
    if (aquatic)
    {
        const float counter = (float)(SDL_GetTicks() / 10 % 200);
        const float t = counter * (2.0f * 3.14159265f / 200.0f);
        height += sinf(t) * 2.0f;
    }
    switch (type)
    {
    case FLOWERPOT_TYPE:    height += 26.0f; break;
    case LILYPAD_TYPE:      height += 25.0f; break;
    case STARFRUIT_TYPE:    height += 10.0f; break;
    case TANGLEKELP_TYPE:   height += 24.0f; break;
    case SEASHROOM_TYPE:    height += 28.0f; break;
    case COFFEEBEAN_TYPE:   height -= 20.0f; break;
    case CACTUS_TYPE:       return height;
    case PUMPKIN_TYPE:      height += 15.0f; break;
    case PUFFSHROOM_TYPE:   height += 5.0f;  break;
    case SCAREDYSHROOM_TYPE:height -= 14.0f; break;
    case GRAVEBUSTER_TYPE:  height -= 40.0f; break;
    case SPIKEWEED_TYPE:    height += 15.0f; break; // theBoard == nullptr fallback
    default: break;
    }
    return height;
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
// picks the pool water strip; the sy picks put the icon foot line onto the
// matching rows.
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

// Legacy artH-normalised preview (seed chooser / award pedestal and the
// almanac's non-retail fallback path). The almanac detail pages use the
// in-game retail placement instead (see almanac_draw_plant_detail_page).
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

// In-game retail plant draw at an original almanac cell: doc origin at
// (ALMANAC_X_OFF + mx, my + PlantDrawHeightOffset(board == nullptr)), scale
// 1.0 -- the same PlantPlacement path Plants::display_reanim uses.
static void almanac_draw_plant_retail(int type, int mx, int my, float clock)
{
    if (type < 0 || type >= PLANT_COUNT)
        return;
    if (!reanimplant::ready())
    {
        reanimplant::init(win.sprite_renderer());
        if (!reanimplant::ready())
            return;
    }
    int idle = PLANT_IDLE_DIR[type];
    int n_sheet = all_img[idle].n_sheet > 0 ? all_img[idle].n_sheet : 60;
    reanimplant::PlantPlacement placement;
    placement.x = (float)(ALMANAC_X_OFF + mx);
    placement.y = (float)my + almanac_plant_height_offset(type);
    placement.scale = 1.0f;
    reanimplant::drawPlant(type, idle, (float)((int)clock % n_sheet), n_sheet,
                           clock, ALMANAC_X_OFF + mx, ALMANAC_X_OFF + mx + 80,
                           my, 0, false, nullptr, nullptr, nullptr, nullptr,
                           1.0f, 1.0f, &placement);
}

// --- Zombie anim recipes -----------------------------------------------------
// walk_dir/armor_dir pick the same in-game draw state the original almanac
// uses (ZombieInitialize walks the real zombie); a few types whose retail
// vignette reads better in a fixed segment keep that segment as the almanac's
// anim choice (pole vaulter with the pole in hand, balloon hovering, catapult
// driver, gargantuar walk). Geometry is ALWAYS the in-game placement now --
// no per-type framing table/crops.
enum AlmanacZombiePose
{
    AZ_POSE_WALK,    // drawZombie(walk_dir, armor_dir) -- the in-game walk/idle
    AZ_POSE_SEGMENT, // drawSkinSegment(skin, seg) -- a chosen full segment
    AZ_POSE_ZOMBOSS, // boss head track subset + dome driver overlay
};

struct AlmanacZombie
{
    int walk_dir;
    int armor_dir; // CONE1/BUCKET1 directory, a reanimz phase (dolphin), -1 otherwise
    AlmanacZombiePose pose = AZ_POSE_WALK;
    int skin = -1;             // AZ_POSE_SEGMENT: reanimz skin
    const char *seg = nullptr; // AZ_POSE_SEGMENT: segment name
    const std::unordered_set<std::string> *hide = nullptr; // Zomboss: body-out tracks
};

// Track set cropped out of the Zomboss head pose (matched lowercase, same
// semantics as reanimz::drawZombie's extraHide): everything but the head and
// its cockpit dome. The legs/rv/body tracks lie BEHIND the head in
// anim_head_idle, so hiding the body tracks is what makes the head read as a
// head instead of "head in front of the robot".
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

const AlmanacZombie ZOMBIE_RECIPE[COUNT_ZOMBIE_TYPE] = {
    {ZOMBIE_WALK1_DIRECTORY, -1},                                    // normal
    {FLAG_ZOMBIE_WALK1_1_DIRECTORY, -1},                             // flag
    {ZOMBIE_WALK1_DIRECTORY, CONE1_DIRECTORY},                       // conehead
    {ZOMBIE_WALK1_DIRECTORY, BUCKET1_DIRECTORY},                     // buckethead
    {DOOR_ZOMBIE_WALK1_1_DIRECTORY, -1},                             // screen door
    {POLEVAULT_WALK_DIRECTORY, -1, AZ_POSE_SEGMENT,
     reanimz::SKIN_POLEVAULT, "anim_idle"},                          // pole vaulter: standing, pole in hand
    {NEWSPAPER_WALK_DIRECTORY, -1},                                  // newspaper
    {FOOTBALL_RUN_DIRECTORY, -1},                                    // football
    {DANCER_WALK_DIRECTORY, -1},                                     // dancer
    {BACKUP_WALK_DIRECTORY, -1},                                     // backup dancer
    {DUCKYTUBE_WALK_DIRECTORY, -1},                                  // ducky tube
    {SNORKEL_WALK_DIRECTORY, -1},                                    // snorkel
    {ZOMBONI_DRIVE_DIRECTORY, -1},                                   // zomboni
    {BOBSLED_WALK_DIRECTORY, -1},                                    // bobsled team
    {DOLPHINRIDER_WALK_DIRECTORY, reanimz::DOLPHIN_CARRY_STATE},     // dolphin rider: dolphin on head
    {JACKBOX_WALK_DIRECTORY, -1},                                    // jack-in-the-box
    {BALLOON_WALK_DIRECTORY, -1, AZ_POSE_SEGMENT,
     reanimz::SKIN_BALLOON, "anim_idle"},                            // balloon: only idle shows the balloon
    {DIGGER_WALK_DIRECTORY, -1},                                     // digger
    {POGO_WALK_DIRECTORY, -1},                                       // pogo
    {BUNGEE_WALK_DIRECTORY, -1},                                     // bungee
    {LADDER_WALK_DIRECTORY, -1},                                     // ladder
    {CATAPULT_WALK_DIRECTORY, -1, AZ_POSE_SEGMENT,
     reanimz::SKIN_CATAPULT, "anim_idle"},                           // catapult driver
    {GARGANTUAR_WALK_DIRECTORY, -1, AZ_POSE_SEGMENT,
     reanimz::SKIN_GARGANTUAR, "anim_walk"},                         // gargantuar
    {IMP_WALK_DIRECTORY, -1},                                        // imp
    {0, -1, AZ_POSE_ZOMBOSS, -1, nullptr, &kBossBodyCrop},           // zomboss
    {YETI_WALK_DIRECTORY, -1},                                       // zombie yeti
};

static bool almanac_reanimz_ready()
{
    if (!reanimz::ready() && !reanimz::init(win.sprite_renderer()))
        return false;
    return reanimz::ready();
}

// Original detail-page framing nudges (AlmanacDialog.cpp:422-431,
// DrawZombies): an extra translateF applied after the retail doc origin.
// The boss's (-540,-175) entry is replaced by the head-only pose below.
static void almanac_zombie_detail_nudge(int type, int &dx, int &dy)
{
    dx = dy = 0;
    switch (type)
    {
    case ZOMBONI_TYPE:    dx = -30; dy = 5;   break;
    case GARGANTUAR_TYPE: dx = 0;   dy = 40;  break;
    case FOOTBALL_TYPE:   dx = -10; dy = 0;   break;
    case BALLOON_TYPE:    dx = 0;   dy = -20; break;
    case BUNGEE_TYPE:     dx = 15;  dy = 0;   break;
    case CATAPULT_TYPE:   dx = -10; dy = 0;   break;
    default: break;
    }
}

// Detail-page zombie: exactly the in-game draw at the given x_location /
// y_location (the original mPosX/mPosY chain), at `scale_mul` x retail skin
// scale. Zomboss keeps the head-only pose + dome driver overlay the original
// cached frame uses.
//
// y_location is the in-game row pair: the clone feeds cells[row].y1 - 50 while
// the original mPosY = GridToPixelY(row) - 30, so mPosY = y_location + 20
// (drawZombie: oy = y_location + 132 - 120*s == mPosY - 8 at s = 1).
void almanac_draw_zombie_detail(int type, float clock, int x_location,
                                int y_location, float scale_mul)
{
    if (type < 0 || type >= COUNT_ZOMBIE_TYPE || !almanac_reanimz_ready())
        return;
    const AlmanacZombie &r = ZOMBIE_RECIPE[type];
    if (r.pose != AZ_POSE_ZOMBOSS)
    {
        int ndx, ndy;
        almanac_zombie_detail_nudge(type, ndx, ndy);
        x_location += ndx;
        y_location += ndy;
    }
    reanimz::setZombieScaleMul(scale_mul);
    switch (r.pose)
    {
    case AZ_POSE_SEGMENT:
        reanimz::drawSkinSegment(r.skin, r.seg, clock, x_location, y_location,
                                 false, false, 0, 0, scale_mul, true, nullptr);
        break;
    case AZ_POSE_ZOMBOSS:
    {
        int ddx, ddy;
        reanimz::bossDocCompensation(scale_mul, ddx, ddy);
        reanimz::drawSkinSegment(reanimz::SKIN_ZOMBOSS, "anim_head_idle", clock,
                                 x_location, y_location, false, false, ddx, ddy,
                                 scale_mul, true, &kBossBodyCrop);
        bossdriver::draw("anim_head_idle", clock, true, x_location, y_location,
                         scale_mul, false, false);
        break;
    }
    default:
        reanimz::drawZombie(r.walk_dir, r.armor_dir, clock, x_location,
                            y_location, false, false);
        break;
    }
    reanimz::setZombieScaleMul(1.0f);
}

// Loop length of the zombie's detail pose in 12fps clock units.
float almanac_zombie_pose_loop_length(int type)
{
    if (type < 0 || type >= COUNT_ZOMBIE_TYPE || !almanac_reanimz_ready())
        return 0.0f;
    const AlmanacZombie &r = ZOMBIE_RECIPE[type];
    switch (r.pose)
    {
    case AZ_POSE_SEGMENT:
        return reanimz::skinSegmentLength(r.skin, r.seg);
    case AZ_POSE_ZOMBOSS:
        return reanimz::skinSegmentLength(reanimz::SKIN_ZOMBOSS, "anim_head_idle");
    default:
        return reanimz::segmentLength(r.walk_dir);
    }
}

// Per-type translateF table of the original grid thumbnails (AlmanacDialog.cpp
// :390-412). Boss uses its own -540,-175 special case (handled separately).
static void almanac_zombie_thumb_offset(int type, int &tx, int &ty)
{
    tx = ty = 0;
    switch (type)
    {
    case POLEVAULT_TYPE:   tx = 2;   ty = -3;  break;
    case FLAG_TYPE:        tx = 2;   ty = 10;  break;
    case CONE_TYPE:        tx = 0;   ty = 12;  break;
    case BUCKET_TYPE:      tx = 0;   ty = 9;   break;
    case FOOTBALL_TYPE:    tx = -15; ty = -1;  break;
    case ZOMBONI_TYPE:     tx = 0;   ty = 3;   break;
    case DOLPHINRIDER_TYPE:tx = -2;  ty = -10; break;
    case POGO_TYPE:        tx = 0;   ty = -3;  break;
    case GARGANTUAR_TYPE:  tx = 15;  ty = 17;  break;
    case IMP_TYPE:         tx = -8;  ty = -7;  break;
    case BUNGEE_TYPE:      tx = -4;  ty = 3;   break;
    case DANCER_TYPE:      tx = 0;   ty = 15;  break;
    case BACKUP_TYPE:      tx = -4;  ty = 20;  break;
    case SNORKEL_TYPE:     tx = -10; ty = 0;   break;
    case YETI_TYPE:        tx = 0;   ty = 4;   break;
    case CATAPULT_TYPE:    tx = -24; ty = -1;  break;
    case BOBSLED_TYPE:     tx = 0;   ty = -8;  break;
    case LADDER_TYPE:      tx = 0;   ty = -3;  break;
    default: break;
    }
}

// Grid thumbnail of one zombie, original geometry: the in-game zombie at
// scale 0.5 with its retail doc origin at (cell_x+21+tx, cell_y+14+ty)
// (cache image draws the reanim at (40,40), 0.5 scale -> +20; the original
// translates by (cell+1, cell-6)). Pose = first frame of anim_idle when the
// doc has it, else walk frame 0.
static void almanac_draw_zombie_thumb(int type, int cell_x, int cell_y)
{
    if (type < 0 || type >= COUNT_ZOMBIE_TYPE || !almanac_reanimz_ready())
        return;
    const AlmanacZombie &r = ZOMBIE_RECIPE[type];
    int tx = 0, ty = 0;
    almanac_zombie_thumb_offset(type, tx, ty);

    if (r.pose == AZ_POSE_ZOMBOSS)
    {
        // The original cached boss frame is its own translateF(-540,-175)
        // special case. Keep the head+dome pose and frame it in the glass at
        // the thumbnail scale (probe-based, no crop table).
        reanimz::PoseGeom g;
        if (!reanimz::skinPoseGeomFrame(reanimz::SKIN_ZOMBOSS, "anim_head_idle",
                                        0.0f, g, &kBossBodyCrop))
            return;
        const float k = ALMANAC_ZOMBIE_THUMB_SCALE;
        int ddx, ddy;
        reanimz::bossDocCompensation(k, ddx, ddy);
        int xl = (int)lroundf(cell_x + 38 - (g.pivotX + ddx) - k * g.C);
        int yl = (int)lroundf(cell_y + 74 - (g.pivotY + ddy) - k * g.B);
        almanac_draw_zombie_detail(type, 0.0f, xl, yl, k);
        return;
    }

    const float k = ALMANAC_ZOMBIE_THUMB_SCALE;
    const float s = reanimz::skinScaleForType(type) * k;
    float offx = 0, offy = 0;
    reanimz::zombieImageOffsetForType(type, offx, offy);
    const float target_x = (float)(cell_x + 1 + 20 + tx);
    const float target_y = (float)(cell_y - 6 + 20 + ty);
    // ox = x + 45 + offX - 30*s, oy = y + 132 + offY - 120*s: solve for the
    // in-game x_location/y_location that lands the doc origin on the target.
    int xl = (int)lroundf(target_x - 45.0f - offx + 30.0f * s);
    int yl = (int)lroundf(target_y - 132.0f - offy + 120.0f * s);
    reanimz::setZombieScaleMul(k);
    reanimz::drawZombieSegment(r.walk_dir, r.armor_dir, "anim_idle", 0.0f,
                               xl, yl);
    reanimz::setZombieScaleMul(1.0f);
}

// --- Zombies grid geometry (original GetZombiePosition) ----------------------
void almanac_zombie_cell(int type, int scroll_row, int &x, int &y)
{
    if (type == ZOMBOSS_TYPE)
    {
        x = ALMANAC_X_OFF + 192;
        y = 486 - scroll_row * ALMANAC_ZOMBIE_CELL_DY;
        return;
    }
    x = ALMANAC_ZOMBIE_GRID_X + (type % ALMANAC_ZOMBIE_COLS) * ALMANAC_ZOMBIE_CELL_DX;
    y = ALMANAC_ZOMBIE_GRID_Y +
        (type / ALMANAC_ZOMBIE_COLS - scroll_row) * ALMANAC_ZOMBIE_CELL_DY;
}

int almanac_zombie_max_scroll_row()
{
    int rows = (COUNT_ZOMBIE_TYPE + ALMANAC_ZOMBIE_COLS - 1) / ALMANAC_ZOMBIE_COLS;
    return std::max(0, rows - ALMANAC_ZOMBIE_VIS_ROWS);
}

bool almanac_zombie_row_visible(int type, int scroll_row)
{
    int row = type == ZOMBOSS_TYPE ? 5 : type / ALMANAC_ZOMBIE_COLS;
    return row >= scroll_row && row < scroll_row + ALMANAC_ZOMBIE_VIS_ROWS;
}

// View rect of the grid (cell area + scrollbar strip), for wheel hit-testing.
Button almanac_zombies_view_rect()
{
    int right = ALMANAC_ZOMBIE_GRID_X +
                (ALMANAC_ZOMBIE_COLS - 1) * ALMANAC_ZOMBIE_CELL_DX +
                ALMANAC_ZOMBIE_CELL_W;
    int bottom = ALMANAC_ZOMBIE_GRID_Y +
                 (ALMANAC_ZOMBIE_VIS_ROWS - 1) * ALMANAC_ZOMBIE_CELL_DY +
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

    // Animated mascots: sunflower at the original index plant cell (167,255)
    // and the normal zombie at mPosX/mPosY (535,215) -- both through the same
    // in-game draw paths as the detail pages (mPosY = y_location + 20).
    almanac_draw_plant_retail(SUNFLOWER_TYPE, 167, 255, s.clock);
    almanac_draw_zombie_detail(NORMAL_TYPE, s.clock, ALMANAC_X_OFF + 535,
                               215 - 20, 1.0f);

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

// The in-game seed-bank packet: packet art + world-height portrait + sun tag +
// bottom border + cost, at the bank's normal size. A high sun count keeps the
// almanac from drawing the unaffordable dimming overlay.
void almanac_draw_plant_packet(int type, int x, int y)
{
    if (type < 0 || type >= PLANT_COUNT)
        return;
    SeedPacket packet(type);
    packet.display(x, y, 999999);
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
        // Original GetSeedPosition layout, but the clone's own bank packet
        // (same art/size the in-game seed bank draws).
        almanac_draw_plant_packet(type, x, y);
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
        // Original SetupPlant cell (578,140 + per-seed tweaks), retail draw.
        int px, py;
        almanac_plant_detail_cell(t, px, py);
        almanac_draw_plant_retail(t, px, py, s.clock);
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
        // The thumbnail is the in-game zombie at scale 0.5, doc origin placed
        // at (cell+21+tx, cell+14+ty) and scissored to the window glass
        // (original AlmanacDialog grid geometry).
        SDL_Rect glass{x + 2, y + 2, ALMANAC_ZOMBIE_CELL_W - 4,
                       ALMANAC_ZOMBIE_CELL_H - 4};
        win.sprite_renderer().setClipRect(&glass);
        almanac_draw_zombie_thumb(type, x, y);
        win.sprite_renderer().setClipRect(nullptr);
        // Glass frame drawn over the zombie (window2 is the native 85x85
        // frame at the same top-left, overlapping the neighbors as authored).
        win.draw_png(ALMANAC_ZOMBIE_WINDOW2_DIRECTORY, x, y, 85, 85);
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
        // In-game zombie at the original detail placement mPosX/mPosY =
        // (559,175), i.e. x_location = 559+OX, y_location = 175-20 (the
        // in-game mPosY = y_location + 20 chain).
        SDL_Rect patch{ALMANAC_X_OFF + 518, 110, 200, 200};
        win.sprite_renderer().setClipRect(&patch);
        almanac_draw_zombie_detail(t, s.clock, ALMANAC_X_OFF + 559, 175 - 20,
                                   1.0f);
        win.sprite_renderer().setClipRect(nullptr);
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

// Headless page render for the scene dump: draw one page into the current
// renderer without running the scene loop. page 0 = index, 1 = plants,
// 2 = zombies; entry selects the detail subject.
void almanac_render_page(int page, int entry, float clock)
{
    AlmanacState s;
    s.clock = clock;
    if (page == 1)
    {
        s.page = ALMANAC_PAGE_PLANTS;
        s.selected_plant = (entry >= 0 && entry < PLANT_COUNT) ? entry : 0;
    }
    else if (page == 2)
    {
        s.page = ALMANAC_PAGE_ZOMBIES;
        s.selected_zombie = (entry >= 0 && entry < COUNT_ZOMBIE_TYPE) ? entry : 0;
    }
    else
    {
        s.page = ALMANAC_PAGE_INDEX;
    }
    if (s.page == ALMANAC_PAGE_INDEX)
        almanac_draw_index(s);
    else if (s.page == ALMANAC_PAGE_PLANTS)
        almanac_draw_plants(s);
    else
        almanac_draw_zombies(s);
}
