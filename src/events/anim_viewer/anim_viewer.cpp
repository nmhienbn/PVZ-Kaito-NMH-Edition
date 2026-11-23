#include "anim_viewer.hpp"

#include "draw/plant_icon_dir.hpp"
#include "draw/rsdl.hpp"
#include "elements/Map/Map.hpp"
#include "elements/SeedPacket/SeedPacket.hpp"
#include "elements/plants/plants.hpp"
#include "elements/zombies/zombie_struct.hpp"
#include "events/almanac/almanac_info.hpp"
#include "level/debut_data.hpp"
#include "nlohmann/json.hpp"
#include "reanim/reanim_plant.hpp"
#include "reanim/reanim_sdl.hpp"
#include "reanim/reanim_zombie.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace std;

extern bool quit;
extern Map cells;
extern Window win;

namespace
{

enum PreviewMode
{
    P_REANIM_LOOP, // reanimz::drawZombie loop           (sheet: same dir frame loop)
    P_REANIM_DEATH,// reanimz::drawDeathPart            (sheet: die + flying head)
    P_REANIM_DROP, // reanimz::drawDropPart             (sheet: baked drop sheet, if any)
    P_REANIM_IDLE, // reanimz::drawSkinSegment sub-window of the base doc's idle
                   //   seg (segFirst..segLast; sheet: the old credit sheet)
    P_PLANT,       // plant sheet state; reanimplant sync view when the skin loaded
    P_SKIN_LABEL   // raw reanim label: zombie = reanimz::drawSkinSegment
                   //   (skin field, SKIN_FLAGPOLE = the flag-pole doc),
                   //   plant = reanimplant::drawPlantSegment (ptype field)
};

struct AnimState
{
    string label;
    PreviewMode mode;
    int dir;         // ImageType directory (zombie body loop / plant sheet)
    int armor;       // drawZombie armor_dir (CONE1..3/BUCKET1..3_DIRECTORY or -1)
    int drop_img;    // reanimz::DROP_*_IMG / DEATH_PART_IMG for P_REANIM_DROP
    int sheet_dir;   // P_REANIM_DROP sheet fallback (-1 = none)
    bool cold, attacked;
    const char *cmd_dir; // P_PLANT: asset json to size with (nullptr = no json)
    // P_SKIN_LABEL fields
    int skin;        // zombie: reanimz skin id (SKIN_PLAIN / SKIN_FLAGPOLE / 1..20)
    int ptype;       // plant: PlantType (-1 on zombie states)
    string seg;      // reanim segment label
    int segFirst, segLast; // absolute frame range (for the "frame i/N" readout)
};

// Parts panel data, one instance per subject (built lazily from the
// subject's own reanim doc, so a skin zombie lists its skin's tracks and a
// plant lists its plant skin's tracks).
struct TrackGroup
{
    string name;
    vector<string> tracks;
};

// Flattened panel rows: one header per group, then its tracks.
struct PanelItem
{
    bool header;
    string text;  // display text (track rows: name without the zombie_ prefix)
    string track; // full lowercase track name ("" for headers)
    string group; // owning group (headers: the group itself)
};

struct CharDef
{
    string name;
    bool is_zombie;
    int type; // ZombieType (zombies) / PlantType (plants)
    int skin; // zombies: reanimz skin for label playback + the parts doc
    vector<AnimState> states;
    // per-subject parts panel (sticky until ALL/RESET), built on first view
    bool panelBuilt = false;
    vector<TrackGroup> groups;
    vector<PanelItem> items;
    vector<string> allTracks;
    unordered_set<string> userShow, userHide;
    bool showAll = false;
    int scroll = 0;
};

AnimState zLoop(const char *label, int dir, int armor = -1, bool cold = false, bool attacked = false)
{
    return AnimState{string("game: ") + label, P_REANIM_LOOP, dir, armor, 0, -1, cold, attacked, nullptr, 0, -1, "", 0, 0};
}

AnimState zDeath(const char *label)
{
    return AnimState{string("game: ") + label, P_REANIM_DEATH, ZOMBIE_DIE1_DIRECTORY, -1, 0, -1, false, false, nullptr, 0, -1, "", 0, 0};
}

AnimState zDrop(const char *label, int drop_img, int sheet_dir)
{
    return AnimState{string("game: ") + label, P_REANIM_DROP, 0, -1, drop_img, sheet_dir, false, false, nullptr, 0, -1, "", 0, 0};
}

// Natural idle of the base zombie doc (resources/reanim/Zombie.reanim, track
// anim_idle = frames 0..30, the slow standing sway the retail credit roll
// plays): shown as two looping halves (the retail credit-idle split, mirrored
// by reanim_zombie.cpp's credit mapping). `sheet_dir` keeps the old F3 credit
// sheet as the no-reanim fallback / T-compare side.
AnimState zIdle(const char *label, int first, int last, int sheet_dir)
{
    return AnimState{string("game: ") + label, P_REANIM_IDLE, sheet_dir, -1, 0, sheet_dir,
                     false, false, nullptr, reanimz::SKIN_PLAIN, -1, "anim_idle", first, last};
}

AnimState pSheet(const char *label, int dir, const char *cmd_dir, int ptype)
{
    return AnimState{string("game: ") + label, P_PLANT, dir, -1, 0, -1, false, false, cmd_dir, 0, ptype, "", 0, 0};
}

// Raw reanim label states, for the zombie docs (skin, SKIN_PLAIN or the
// flagpole doc) and the plant docs.
AnimState zLabel(int skin, const string &seg, int first, int last)
{
    return AnimState{seg, P_SKIN_LABEL, 0, -1, 0, -1, false, false, nullptr, skin, -1, seg, first, last};
}

AnimState pLabel(int ptype, const string &seg, int first, int last)
{
    return AnimState{seg, P_SKIN_LABEL, 0, -1, 0, -1, false, false, nullptr, 0, ptype, seg, first, last};
}

// walk1/walk2/eat sheets for a door degrade stage; armor sheets are separate
// only for cone/bucket, door/flag bake their stage into the body sheets.
const int FLAG_WALK1[3] = {FLAG_ZOMBIE_WALK1_1_DIRECTORY, FLAG_ZOMBIE_WALK1_2_DIRECTORY, FLAG_ZOMBIE_WALK1_3_DIRECTORY};
const int FLAG_WALK2[3] = {FLAG_ZOMBIE_WALK2_1_DIRECTORY, FLAG_ZOMBIE_WALK2_2_DIRECTORY, FLAG_ZOMBIE_WALK2_3_DIRECTORY};
const int FLAG_EAT[3] = {FLAG_ZOMBIE_EATING_1_DIRECTORY, FLAG_ZOMBIE_EATING_2_DIRECTORY, FLAG_ZOMBIE_EATING_3_DIRECTORY};
const int DOOR_WALK1[3] = {DOOR_ZOMBIE_WALK1_1_DIRECTORY, DOOR_ZOMBIE_WALK1_2_DIRECTORY, DOOR_ZOMBIE_WALK1_3_DIRECTORY};
const int DOOR_WALK2[3] = {DOOR_ZOMBIE_WALK2_1_DIRECTORY, DOOR_ZOMBIE_WALK2_2_DIRECTORY, DOOR_ZOMBIE_WALK2_3_DIRECTORY};
const int DOOR_EAT[3] = {DOOR_ZOMBIE_EATING_1_DIRECTORY, DOOR_ZOMBIE_EATING_2_DIRECTORY, DOOR_ZOMBIE_EATING_3_DIRECTORY};
const int ARMOR_CONE[3] = {CONE1_DIRECTORY, CONE2_DIRECTORY, CONE3_DIRECTORY};
const int ARMOR_BUCKET[3] = {BUCKET1_DIRECTORY, BUCKET2_DIRECTORY, BUCKET3_DIRECTORY};

void pushArmoredStates(vector<AnimState> &v, const char *armorName, const int armorDir[3])
{
    // Cone/bucket bodies are the plain zombie sheets; the stage rides in the
    // armor_dir parameter (sheet side draws the armor overlay, like gameplay).
    char buf[40];
    for (int s = 0; s < 3; s++)
    {
        snprintf(buf, sizeof buf, "walk1 %s %d", armorName, s + 1);
        v.push_back(zLoop(buf, ZOMBIE_WALK1_DIRECTORY, armorDir[s]));
        snprintf(buf, sizeof buf, "walk2 %s %d", armorName, s + 1);
        v.push_back(zLoop(buf, ZOMBIE_WALK2_DIRECTORY, armorDir[s]));
        snprintf(buf, sizeof buf, "eat %s %d", armorName, s + 1);
        v.push_back(zLoop(buf, ZOMBIE_EATING_DIRECTORY, armorDir[s]));
    }
}

void pushStageStates(vector<AnimState> &v, const char *kindName, const int walk1[3], const int walk2[3], const int eat[3])
{
    // Flag/door stages are baked into separate body sheets.
    char buf[40];
    for (int s = 0; s < 3; s++)
    {
        snprintf(buf, sizeof buf, "walk1 %s %d", kindName, s + 1);
        v.push_back(zLoop(buf, walk1[s]));
        snprintf(buf, sizeof buf, "walk2 %s %d", kindName, s + 1);
        v.push_back(zLoop(buf, walk2[s]));
        snprintf(buf, sizeof buf, "eat %s %d", kindName, s + 1);
        v.push_back(zLoop(buf, eat[s]));
    }
}

// Zombie type -> reanimz skin the label states and parts panel use (plain
// doc = normal zombo + all equipment variants; ducky-tube too).
int skinOfZombie(int zt)
{
    switch (zt)
    {
    case POLEVAULT_TYPE: return reanimz::SKIN_POLEVAULT;
    case NEWSPAPER_TYPE: return reanimz::SKIN_NEWSPAPER;
    case FOOTBALL_TYPE: return reanimz::SKIN_FOOTBALL;
    case DANCER_TYPE:
    case BACKUP_TYPE: return reanimz::SKIN_DANCER;
    case SNORKEL_TYPE: return reanimz::SKIN_SNORKEL;
    case ZOMBONI_TYPE: return reanimz::SKIN_ZOMBONI;
    case BOBSLED_TYPE: return reanimz::SKIN_BOBSLED;
    case DOLPHINRIDER_TYPE: return reanimz::SKIN_DOLPHINRIDER;
    case JACKBOX_TYPE: return reanimz::SKIN_JACKBOX;
    case BALLOON_TYPE: return reanimz::SKIN_BALLOON;
    case DIGGER_TYPE: return reanimz::SKIN_DIGGER;
    case POGO_TYPE: return reanimz::SKIN_POGO;
    case BUNGEE_TYPE: return reanimz::SKIN_BUNGEE;
    case LADDER_TYPE: return reanimz::SKIN_LADDER;
    case CATAPULT_TYPE: return reanimz::SKIN_CATAPULT;
    case GARGANTUAR_TYPE: return reanimz::SKIN_GARGANTUAR;
    case IMP_TYPE: return reanimz::SKIN_IMP;
    case ZOMBOSS_TYPE: return reanimz::SKIN_ZOMBOSS;
    case YETI_TYPE: return reanimz::SKIN_YETI;
    default: return reanimz::SKIN_PLAIN;
    }
}

vector<CharDef> buildChars()
{
    vector<CharDef> chars;
    for (int zt = 0; zt < (int)COUNT_ZOMBIE_TYPE; zt++)
    {
        CharDef c;
        c.name = almanac_zombie_name(zt);
        c.is_zombie = true;
        c.type = zt;
        c.skin = skinOfZombie(zt);

        // Game states (the exact drawZombie paths) for the classic roster,
        // FIRST: they make the default preview state the walking zombie. The
        // raw label states below begin at the base doc's anim_idle (the
        // standing sway) -- pushing labels first showed every zombie frozen
        // mid-sway on open instead of walking.
        switch (zt)
        {
        case NORMAL_TYPE:
            c.states.push_back(zLoop("walk1", ZOMBIE_WALK1_DIRECTORY));
            c.states.push_back(zLoop("walk2", ZOMBIE_WALK2_DIRECTORY));
            c.states.push_back(zLoop("eat", ZOMBIE_EATING_DIRECTORY));
            c.states.push_back(zLoop("armless walk1", ARMLESS_ZOMBIE_WALK1_DIRECTORY));
            c.states.push_back(zLoop("armless walk2", ARMLESS_ZOMBIE_WALK2_DIRECTORY));
            c.states.push_back(zLoop("armless eat", ARMLESS_ZOMBIE_EATING_DIRECTORY));
            c.states.push_back(zLoop("walk1 COLD", ZOMBIE_WALK1_DIRECTORY, -1, true));
            c.states.push_back(zLoop("walk1 ATTACKED", ZOMBIE_WALK1_DIRECTORY, -1, false, true));
            c.states.push_back(zDeath("die + head pop"));
            c.states.push_back(zDrop("drop head", reanimz::DEATH_PART_IMG, ZOMBIE_HEAD_DIRECTORY));
            c.states.push_back(zDrop("drop arm", reanimz::DROP_ARM_IMG, ZOMBIE_ARM_DIRECTORY));
            // Natural standing pose from the reanim XML (base doc anim_idle,
            // 0..30) -- replaces the old static "credit1/credit2" sheet
            // states, which showed the stiff F3 credit art.
            c.states.push_back(zIdle("idle1", 0, 15, ZOMBIE_CREDIT1_DIRECTORY));
            c.states.push_back(zIdle("idle2", 16, 30, ZOMBIE_CREDIT2_DIRECTORY));
            break;
        case FLAG_TYPE:
            pushStageStates(c.states, "flag", FLAG_WALK1, FLAG_WALK2, FLAG_EAT);
            c.states.push_back(zDrop("drop flag", reanimz::DROP_FLAG_IMG, -1));
            break;
        case CONE_TYPE:
            pushArmoredStates(c.states, "cone", ARMOR_CONE);
            c.states.push_back(zDrop("drop cone", reanimz::DROP_CONE_IMG, CONE_DROP_DIRECTORY));
            break;
        case BUCKET_TYPE:
            pushArmoredStates(c.states, "bucket", ARMOR_BUCKET);
            c.states.push_back(zDrop("drop bucket", reanimz::DROP_BUCKET_IMG, BUCKET_DROP_DIRECTORY));
            break;
        case DOOR_TYPE:
            pushStageStates(c.states, "door", DOOR_WALK1, DOOR_WALK2, DOOR_EAT);
            c.states.push_back(zDrop("drop door", reanimz::DROP_DOOR_IMG, -1));
            break;
        case DUCKYTUBE_TYPE:
            c.states.push_back(zLoop("walk (tube)", DUCKYTUBE_WALK_DIRECTORY));
            c.states.push_back(zLoop("eat (tube)", DUCKYTUBE_EAT_DIRECTORY));
            break;
        default:
            break;
        }

        // Raw reanim label states: every anim_* segment of the subject's doc,
        // anim_walk leading when the doc has one so label-only skins still
        // default to the walking pose instead of anim_idle (a standing sway).
        vector<AnimState> labels;
        for (const auto &si : reanimz::skinSegments(c.skin))
            labels.push_back(zLabel(c.skin, si.name, si.first, si.last));
        for (size_t i = 0; i < labels.size(); i++)
            if (labels[i].seg == "anim_walk")
            {
                c.states.push_back(labels[i]);
                labels.erase(labels.begin() + i);
                break;
            }
        for (auto &l : labels)
            c.states.push_back(std::move(l));
        // The Flag zombie's pole: Zombie_flagpole.reanim carries no anim_*
        // labels (pure rest-pose timeline), so there are no pole label
        // states; the pole's tracks are exposed in the PARTS panel (group
        // "flag (pole/hand)", from reanimz::trackNames()) and its hand-
        // attached animation shows in the "game:" states above.
        chars.push_back(std::move(c));
    }
    for (int pt = 0; pt < PLANT_COUNT; pt++)
    {
        CharDef c;
        c.name = almanac_plant_name(pt);
        c.is_zombie = false;
        c.type = pt;
        c.skin = -1;

        // Raw reanim label states: every segment of the plant skin's doc.
        for (const auto &si : reanimplant::segments(pt))
            c.states.push_back(pLabel(pt, si.name, si.first, si.last));

        // Game states (sheet dirs, incl. the sync view) for the classic
        // plants that ship sprite-sheet assets + json size info.
        switch (pt)
        {
        case PEASHOOTER_TYPE:
            c.states.push_back(pSheet("idle", PEASHOOTER_SHEET_DIRECTORY, "peashooter", pt));
            c.states.push_back(pSheet("attack", PEASHOOTER_ATTACK_DIRECTORY, "peashooter", pt));
            break;
        case REPEATER_TYPE:
            c.states.push_back(pSheet("idle", PEASHOOTER_SHEET_DIRECTORY, "repeater", pt));
            c.states.push_back(pSheet("attack", PEASHOOTER_ATTACK_DIRECTORY, "repeater", pt));
            break;
        case SUNFLOWER_TYPE:
            c.states.push_back(pSheet("idle", SUNFLOWER_SHEET_DIRECTORY, "sunflower", pt));
            c.states.push_back(pSheet("happy", SUNFLOWER_HAPPY_DIRECTORY, "sunflower", pt));
            break;
        case WALLNUT_TYPE:
            c.states.push_back(pSheet("stage 1 (full)", WALLNUT_1_DIRECTORY, "wallnut", pt));
            c.states.push_back(pSheet("stage 2", WALLNUT_2_DIRECTORY, "wallnut", pt));
            c.states.push_back(pSheet("stage 3", WALLNUT_3_DIRECTORY, "wallnut", pt));
            c.states.push_back(pSheet("stage 4", WALLNUT_4_DIRECTORY, "wallnut", pt));
            c.states.push_back(pSheet("stage 5", WALLNUT_5_DIRECTORY, "wallnut", pt));
            break;
        case SNOWPEA_TYPE:
            c.states.push_back(pSheet("idle", SNOWPEA_SHEET_DIRECTORY, "snowpea", pt));
            c.states.push_back(pSheet("attack", SNOWPEA_ATTACK_DIRECTORY, "snowpea", pt));
            break;
        case POTATOMINE_TYPE:
            c.states.push_back(pSheet("unarmed", POTATOMINE_UNARMED_DIRECTORY, "potatomine", pt));
            c.states.push_back(pSheet("recover", POTATOMINE_RECOVER_DIRECTORY, "potatomine", pt));
            c.states.push_back(pSheet("idle (armed)", POTATOMINE_IDLE_DIRECTORY, "potatomine", pt));
            c.states.push_back(pSheet("attack", POTATOMINE_ATTACK_DIRECTORY, "potatomine_explosion", pt));
            c.states.push_back(pSheet("explosion", POTATOMINE_EXPLOSION_DIRECTORY, "potatomine_explosion", pt));
            break;
        case CHERRYBOMB_TYPE:
            c.states.push_back(pSheet("growing", CHERRYBOMB_SHEET_DIRECTORY, "cherrybomb", pt));
            c.states.push_back(pSheet("explosion", CHERRYBOMB_EXPLOSION_DIRECTORY, "cherrybomb_explosion", pt));
            break;
        default:
            break;
        }
        chars.push_back(std::move(c));
    }
    return chars;
}

// Same per-plant json the gameplay asset structs are built from.
PlantAssets loadPlantAsset(const char *name)
{
    ifstream in("./resources/images/json/" + string(name) + ".json");
    const nlohmann::json j = nlohmann::json::parse(in);
    return PlantAssets(j["frame"], j["img_width"], j["img_height"],
                       j["game_width"], j["game_height"], j["x_alias"], j["y_alias"]);
}

string baseName(const string &path)
{
    return path.substr(path.find_last_of('/') + 1);
}

// Sheet clock: ticks per displayed frame + total frames, per state.
// (P_SKIN_LABEL states don't use the sheet clock; their frame window is the
// segment range, kept in st.segFirst/segLast.)
void sheetInfo(const AnimState &st, int &clk, int &frames)
{
    switch (st.mode)
    {
    case P_PLANT:
        clk = loadPlantAsset(st.cmd_dir).frame;
        frames = all_img[st.dir].n_sheet > 0 ? all_img[st.dir].n_sheet : 1;
        return;
    case P_REANIM_DEATH:
        clk = ZOMBIE_DIE_FRAME;
        frames = all_img[ZOMBIE_DIE1_DIRECTORY].n_sheet;
        return;
    case P_REANIM_DROP:
        clk = HEAD_ZOMBIE_FRAME;
        frames = st.sheet_dir >= 0 ? all_img[st.sheet_dir].n_sheet : 1;
        return;
    default:
        clk = ZOMBIE_FRAME;
        frames = all_img[st.dir].n_sheet;
        return;
    }
}

// Sheet armor overlay constants (ArmoredZombie::render_zombie / ArmorSpec rows).
bool sheetArmorParams(int armor, int &w, int &h, int &bx, int &by)
{
    switch (armor)
    {
    case CONE1_DIRECTORY:
    case CONE2_DIRECTORY:
    case CONE3_DIRECTORY:
        w = 88; h = 68; bx = 70; by = -12;
        return true;
    case BUCKET1_DIRECTORY:
    case BUCKET2_DIRECTORY:
    case BUCKET3_DIRECTORY:
        w = 86; h = 78; bx = 65; by = -5;
        return true;
    default:
        return false;
    }
}

// cone_of/bucket_of: armor sheet frame offset per body directory.
int armorFrameBias(int armor, int bodyDir)
{
    (void)armor; // both armor kinds share the same offset table
    switch (bodyDir)
    {
    case ZOMBIE_CREDIT1_DIRECTORY: return 0;
    case ZOMBIE_CREDIT2_DIRECTORY: return 22;
    case ZOMBIE_WALK1_DIRECTORY: return 44;
    case ZOMBIE_WALK2_DIRECTORY: return 91;
    case ZOMBIE_EATING_DIRECTORY: return 138;
    default: return 44;
    }
}

// --- preview geometry: the state preview stands in the almanac picker card's
// patch (the exported rects mirror almanac.cpp's card); the lawn-cell slot is
// centered in the patch (halved left/right in compare mode), feet on the
// patch foot line -- the almanac anchors zombie icons ~196 / plant icons
// ~156 inside the 200px patch.
int patchFoot(bool zombie)
{
    return zombie ? VIEWER_ZPATCH.y + 196 : VIEWER_PPATCH.y + 156;
}

int previewCx(bool zombie, int slot, bool compare)
{
    const SDL_Rect &p = zombie ? VIEWER_ZPATCH : VIEWER_PPATCH;
    return p.x + (compare ? p.w / 4 + slot * (p.w / 2) : p.w / 2);
}

// Game-size lawn cell (80x100) in the patch slot; the zombie/plant preview
// paths draw relative to it exactly like the in-game render does.
Block previewCell(bool zombie, int slot, bool compare)
{
    int cx = previewCx(zombie, slot, compare);
    int foot = patchFoot(zombie);
    return Block{cx - 40, cx + 40, foot - 100, foot};
}

void drawZombieShadow(const Block &cell)
{
    win.draw_png(SHADOW_DIRECTORY, cell.x1 + 85, cell.y2 - 40, 96, 40);
}
void drawPlantShadow(const Block &cell)
{
    win.draw_png(SHADOW_DIRECTORY, cell.x1 - 5, cell.y2 - 40, 86, 36);
}

// Animated grid-card recipe per type: everyone stands the walk loop of its
// reanim skin (Zomboss his doc's anim_idle -- drawZombie has no directory for
// him). bbox_w/bbox_h/cx_off/foot_off describe the pose's union bounds at
// scale 1, exactly like the almanac's measured ZOMBIE_RECIPE (almanac.cpp
// mirrors the same poses; values copied at build time, walk-pose variants
// where the almanac crops/poses instead are measured from this clone's
// reanimz init probe). cx_off = bbox center-x in pose space (drawZombie box
// anchor (61,141) is the origin), foot_off = bbox bottom.
struct ZCard
{
    int walk_dir;
    int armor_dir; // CONE1_DIRECTORY / BUCKET1_DIRECTORY, -1 otherwise
    float bbox_w, bbox_h, cx_off, foot_off;
    int skin;         // >= 0: draw that skin's `seg` instead of the walk dir
    const char *seg;
};
const ZCard ZCARD[COUNT_ZOMBIE_TYPE] = {
    {ZOMBIE_WALK1_DIRECTORY, -1, 109, 137, -1.0f, -7.0f, -1, nullptr},            // normal
    {FLAG_ZOMBIE_WALK1_1_DIRECTORY, -1, 161, 179, -27.0f, -7.0f, -1, nullptr},    // flag
    {ZOMBIE_WALK1_DIRECTORY, CONE1_DIRECTORY, 110, 170, -1.5f, -7.0f, -1, nullptr},  // conehead
    {ZOMBIE_WALK1_DIRECTORY, BUCKET1_DIRECTORY, 122, 162, -7.5f, -7.0f, -1, nullptr}, // buckethead
    {DOOR_ZOMBIE_WALK1_1_DIRECTORY, -1, 128, 147, -10.5f, 3.0f, -1, nullptr},     // screen door
    {POLEVAULT_WALK_DIRECTORY, -1, 110, 140, 0.0f, -7.0f, -1, nullptr},           // pole vaulter
    {NEWSPAPER_WALK_DIRECTORY, -1, 132, 140, -8.5f, -8.0f, -1, nullptr},          // newspaper
    {FOOTBALL_RUN_DIRECTORY, -1, 162, 157, -2.5f, -13.0f, -1, nullptr},           // football
    {DANCER_WALK_DIRECTORY, -1, 76, 114, 0.0f, -7.0f, -1, nullptr},               // dancer
    {BACKUP_WALK_DIRECTORY, -1, 79, 137, -2.0f, -7.0f, -1, nullptr},              // backup dancer
    {DUCKYTUBE_WALK_DIRECTORY, -1, 109, 137, -1.0f, -7.0f, -1, nullptr},          // ducky tube
    {SNORKEL_WALK_DIRECTORY, -1, 100, 119, -39.5f, 35.0f, -1, nullptr},           // snorkel
    {ZOMBONI_DRIVE_DIRECTORY, -1, 214, 168, -3.5f, -6.0f, -1, nullptr},           // zomboni
    {BOBSLED_WALK_DIRECTORY, -1, 126, 144, -1.5f, -6.0f, -1, nullptr},            // bobsled team
    {DOLPHINRIDER_WALK_DIRECTORY, -1, 104, 128, 0.0f, -10.0f, -1, nullptr},       // dolphin rider (wading)
    {JACKBOX_WALK_DIRECTORY, -1, 106, 143, -2.5f, -7.0f, -1, nullptr},            // jack-in-the-box
    {BALLOON_WALK_DIRECTORY, -1, 99, 130, 0.0f, -5.0f, -1, nullptr},              // balloon
    {DIGGER_WALK_DIRECTORY, -1, 108, 142, -0.5f, -9.0f, -1, nullptr},             // digger
    {POGO_WALK_DIRECTORY, -1, 69, 130, 1.0f, 9.0f, -1, nullptr},                  // pogo
    {BUNGEE_WALK_DIRECTORY, -1, 109, 106, -1.0f, -6.0f, -1, nullptr},             // bungee
    {LADDER_WALK_DIRECTORY, -1, 141, 135, 2.0f, -6.0f, -1, nullptr},              // ladder
    {CATAPULT_WALK_DIRECTORY, -1, 247, 159, 0.0f, -24.0f, -1, nullptr},           // catapult (whole vehicle)
    {GARGANTUAR_WALK_DIRECTORY, -1, 212, 237, 0.0f, 15.0f, -1, nullptr},          // gargantuar (full body)
    {IMP_WALK_DIRECTORY, -1, 79, 93, -3.0f, -3.0f, -1, nullptr},                  // imp
    {0, -1, 324, 679, 170.0f, 0.0f, reanimz::SKIN_ZOMBOSS, "anim_idle"},          // zomboss: full body, own doc
    {YETI_WALK_DIRECTORY, -1, 160, 176, -3.5f, -9.0f, -1, nullptr},               // zombie yeti
};
// ONE common art scale for the card roster: a normal zombie reads 90px tall
// in the 100px window (~90% of the cell). Arts whose measured bounds exceed
// the ART_FIT box shrink to it (the big machines/boss), everything else is
// uniform -- the natural relative sizes stay readable.
const float VIEWER_ZICON_SCALE = 0.66f;
const int VIEWER_ZART_FIT = 92;    // card art box (px) inside the 100x100 window
const int VIEWER_ZFOOT_INSET = 8;  // art bottom lifts off the cell bottom

// Card-sized reanim portrait for one grid cell, bottom-center anchored
// (almanac placement math: bbox centered on the cell, bbox bottom on the foot
// line). The caller scissored us to the cell inset, so nothing spills.
void drawGridZombie(int type, float clock, int x, int y)
{
    if (!reanimz::ready())
        return;
    const ZCard &r = ZCARD[type];
    float s = VIEWER_ZICON_SCALE;
    if (VIEWER_ZART_FIT / r.bbox_w < s)
        s = VIEWER_ZART_FIT / r.bbox_w;
    if (VIEWER_ZART_FIT / r.bbox_h < s)
        s = VIEWER_ZART_FIT / r.bbox_h;
    float anchor_x = x + VIEWER_ZCELL_W / 2.0f;
    float anchor_y = y + VIEWER_ZCELL_H - VIEWER_ZFOOT_INSET;
    int xl = (int)(anchor_x - r.cx_off * s + 0.5f);
    int yl = (int)(anchor_y - r.foot_off * s + 0.5f);
    if (r.skin >= 0)
    {
        reanimz::drawSkinSegment(r.skin, r.seg, clock + type * 7.0f, xl, yl,
                                 false, false, 0, 0, s);
        return;
    }
    reanimz::setZombieScaleMul(s);
    reanimz::setZombieTimeMul(1.0f);
    reanimz::drawZombie(r.walk_dir, r.armor_dir, clock + type * 7.0f,
                        xl - 61, yl - 141, false, false);
    reanimz::setZombieScaleMul(1.0f);
    reanimz::setZombieTimeMul(1.0f);
}

// Old sheet path, replicated from Zombie::render_zombie / ArmoredZombie's
// helm overlay (cone/bucket) / ZombiePart::display.
void drawZombieSheet(const AnimState &st, long tick, int x, int y)
{
    if (st.mode == P_REANIM_DEATH)
    {
        int sframe = (int)(tick / ZOMBIE_DIE_FRAME) % all_img[ZOMBIE_DIE1_DIRECTORY].n_sheet;
        win.draw_nth_frame(ZOMBIE_DIE1_DIRECTORY, sframe, ZOMBIE_WIDTH, ZOMBIE_HEIGHT, x, y, ZOMBIE_WIDTH, ZOMBIE_HEIGHT);
        sframe = (int)(tick / HEAD_ZOMBIE_FRAME) % all_img[ZOMBIE_HEAD_DIRECTORY].n_sheet;
        win.draw_nth_frame(ZOMBIE_HEAD_DIRECTORY, sframe, HEAD_ZOMBIE_WIDTH, HEAD_ZOMBIE_HEIGHT,
                           x + 80, y, HEAD_ZOMBIE_WIDTH, HEAD_ZOMBIE_HEIGHT);
        return;
    }
    if (st.mode == P_REANIM_DROP)
    {
        if (st.sheet_dir < 0)
            return;
        int sframe = (int)(tick / HEAD_ZOMBIE_FRAME) % all_img[st.sheet_dir].n_sheet;
        // armor parts are drawn at x+80 with head size in gameplay
        int dx = st.sheet_dir == ZOMBIE_ARM_DIRECTORY ? x : x + 80;
        int w = st.sheet_dir == ZOMBIE_ARM_DIRECTORY ? ZOMBIE_WIDTH : HEAD_ZOMBIE_WIDTH;
        int h = st.sheet_dir == ZOMBIE_ARM_DIRECTORY ? ZOMBIE_HEIGHT : HEAD_ZOMBIE_HEIGHT;
        win.draw_nth_frame(st.sheet_dir, sframe, w, h, dx, y, w, h);
        return;
    }
    int sframe = ((int)tick / ZOMBIE_FRAME) % all_img[st.dir].n_sheet;
    win.draw_nth_frame(st.dir, sframe, ZOMBIE_WIDTH, ZOMBIE_HEIGHT, x, y, ZOMBIE_WIDTH, ZOMBIE_HEIGHT);
    // armor overlay (cone/bucket), as ArmoredZombie::render_zombie does it
    int aw = 0, ah = 0, abx = 0, aby = 0;
    if (sheetArmorParams(st.armor, aw, ah, abx, aby))
    {
        win.draw_nth_frame(st.armor, sframe + armorFrameBias(st.armor, st.dir), aw, ah,
                           x + abx, y + aby, aw, ah);
    }
    int blink = blink_of(st.dir);
    if (blink >= 0)
    {
        if (st.cold)
        {
            win.set_texture_color(blink, 0, 75, 255);
            win.set_texture_alpha(blink, 120);
            win.draw_nth_frame(blink, sframe, ZOMBIE_WIDTH, ZOMBIE_HEIGHT, x, y, ZOMBIE_WIDTH, ZOMBIE_HEIGHT);
            win.set_texture_color(blink, 255, 255, 255);
            win.set_texture_alpha(blink, 70);
        }
        else if (st.attacked)
        {
            win.draw_nth_frame(blink, sframe, ZOMBIE_WIDTH, ZOMBIE_HEIGHT, x, y, ZOMBIE_WIDTH, ZOMBIE_HEIGHT);
        }
    }
}

// Real reanim path; returns false if reanim assets are unavailable.
bool drawZombieReanim(const AnimState &st, float t, int x, int y,
                      const unordered_set<string> &extraHide,
                      const unordered_set<string> &extraShow, int &partsDrawn,
                      reanim::DrawStats *stats = nullptr)
{
    partsDrawn = 0;
    if (!reanimz::ready())
        return false;
    if (st.mode == P_REANIM_DEATH)
    {
        reanimz::drawDeathPart(t, x, y, st.cold);
        return true;
    }
    if (st.mode == P_REANIM_DROP)
    {
        reanimz::drawDropPart(st.drop_img, t, x, y, st.cold);
        return true;
    }
    if (st.mode == P_REANIM_IDLE)
    {
        float window = (float)(st.segLast - st.segFirst + 1);
        float local = st.segFirst + (window > 0.0f ? fmodf(t, window) : 0.0f);
        return reanimz::drawSkinSegment(st.skin, st.seg.c_str(), local, x, y,
                                        st.cold, st.attacked, 0, 0, 1.0f, true,
                                        &extraHide, &extraShow, &partsDrawn, stats);
    }
    return reanimz::drawZombie(st.dir, st.armor, t, x, y, st.cold, st.attacked,
                               &extraHide, &extraShow, &partsDrawn, stats);
}

// Loop period of one preview state in reanim frames.
float reanimPeriod(const AnimState &st)
{
    if (st.mode == P_REANIM_DEATH)
        return 60.0f; // 48 full corpse + 12 fade = 300 game ticks
    if (st.mode == P_REANIM_DROP)
    {
        int linger = reanimz::dropPartLinger(st.drop_img);
        return linger > 0 ? linger * 0.2f : 60.0f;
    }
    if (st.mode == P_REANIM_IDLE)
        return (float)(st.segLast - st.segFirst + 1);
    float len = reanimz::segmentLength(st.dir);
    return len > 0 ? len : 60.0f;
}

// Loop period of a raw label state in reanim frames: the looping play window
// with the loop-seam duplicate frame trimmed (matching drawSkinSegment /
// drawPlantSegment playback), so arrow-stepping and the frame readout wrap on
// the same range the drawing fmod uses. Raw range as fallback when the doc
// is unavailable.
float labelPeriod(const AnimState &st)
{
    float len = st.ptype >= 0 ? reanimplant::segmentLength(st.ptype, st.seg.c_str())
                              : reanimz::skinSegmentLength(st.skin, st.seg.c_str());
    return len > 0 ? len : (float)(st.segLast - st.segFirst + 1);
}

// --- per-track parts panel (one per subject, zombies and plants alike) ------

string groupOf(bool zombie, const string &t)
{
    if (zombie)
    {
        if (t.find("flag") != string::npos) return "flag (pole/hand)";
        if (t.find("cone") != string::npos) return "cone";
        if (t.find("bucket") != string::npos) return "bucket";
        if (t.find("screendoor") != string::npos) return "door";
        if (t.find("outerarm") != string::npos) return "outer arm";
        if (t.find("innerarm") != string::npos) return "inner arm";
        if (t.find("leg") != string::npos) return "legs";
        if (t.find("head") != string::npos || t.find("hair") != string::npos ||
            t.find("jaw") != string::npos || t.find("mustache") != string::npos)
            return "head/face";
        if (t.find("tongue") != string::npos) return "tongue";
        if (t.find("duckytube") != string::npos || t.find("whitewater") != string::npos)
            return "pool gear";
        return "body/other";
    }
    if (t.rfind("anim_", 0) == 0) return "markers (anim_*)";
    if (t.find("ornament") != string::npos) return "ornament (default hidden)";
    if (t.find("eye") != string::npos || t.find("mouth") != string::npos ||
        t.find("eyebrow") != string::npos || t.find("face") != string::npos)
        return "face";
    if (t.find("stem") != string::npos || t.find("stalk") != string::npos)
        return "stem";
    if (t.find("leaf") != string::npos || t.find("rock") != string::npos)
        return "base/ground";
    return "body/other";
}

// Build the subject's panel rows from its own doc's track list. Row order
// follows the doc, grouped by part family; overrides stick per subject until
// ALL/RESET.
void buildCharPanel(CharDef &ch)
{
    if (ch.panelBuilt)
        return;
    ch.panelBuilt = true;
    vector<string> tracks;
    if (ch.is_zombie)
        // plain-doc zombies: base doc + attached flagpole doc (flag zombie
        // shows the pole tracks; other plain zombies get them grayed out)
        tracks = ch.skin <= 0 ? reanimz::trackNames() : reanimz::skinTrackNames(ch.skin);
    else
        tracks = reanimplant::trackNames(ch.type);
    for (const string &t : tracks)
    {
        string g = groupOf(ch.is_zombie, t);
        size_t i = 0;
        for (; i < ch.groups.size(); i++)
            if (ch.groups[i].name == g)
                break;
        if (i == ch.groups.size())
            ch.groups.push_back({g, {}});
        ch.groups[i].tracks.push_back(t);
    }
    for (const auto &g : ch.groups)
    {
        ch.items.push_back({true, g.name + " (" + to_string(g.tracks.size()) + ")", "", g.name});
        for (const auto &t : g.tracks)
        {
            string d = t;
            if (d.rfind("zombie_", 0) == 0)
                d = d.substr(7);
            ch.items.push_back({false, d, t, g.name});
            ch.allTracks.push_back(t);
        }
    }
}

vector<string> &groupTracks(CharDef &ch, const string &g)
{
    for (auto &gr : ch.groups)
        if (gr.name == g)
            return gr.tracks;
    static vector<string> empty;
    return empty;
}

// Effective visibility of a track given the module's state rules + the
// viewer's user overrides. Matches reanimz::drawZombie + the renderer.
bool trackWillDraw(const reanimz::TrackStatus &ts, bool showAll,
                   const unordered_set<string> &userShow,
                   const unordered_set<string> &userHide)
{
    if (!ts.hasImage)
        return false;
    bool hidden, forced = ts.forceShown;
    if (showAll)
    {
        hidden = false;
        forced = true;
    }
    else
    {
        hidden = ts.codeHidden || userHide.count(ts.name) > 0;
        if (userShow.count(ts.name))
        {
            hidden = false;
            forced = true;
        }
    }
    return !hidden && (!ts.dataHidden || forced);
}

// One click on a track row: hide a drawn track / force-show a hidden one;
// clicking an overridden row removes the override (back to state default).
void toggleTrack(const string &track, const reanimz::TrackStatus &ts,
                 unordered_set<string> &userShow, unordered_set<string> &userHide)
{
    if (userShow.count(track))
        userShow.erase(track);
    else if (userHide.count(track))
        userHide.erase(track);
    else if (trackWillDraw(ts, false, userShow, userHide))
        userHide.insert(track);
    else
        userShow.insert(track);
}

// --- per-track parts panel (plant reanim path) ------------------------------

// Plants use the same per-subject panel (buildCharPanel / userShow /
// userHide / showAll / scroll live on CharDef); only the status structs
// differ between modules.

// Plant statuses reuse the zombie-panel helpers (trackWillDraw/trackWillDraw
// colors) as-is; the two TrackStatus structs carry the same fields.
reanimz::TrackStatus adaptTrackStatus(const reanimplant::TrackStatus &s)
{
    return reanimz::TrackStatus{s.name, s.hasImage, s.codeHidden, s.dataHidden, s.forceShown};
}

// The plant preview's helper overlays (the OPTIONS rows of the plant page).
struct PlantOpts
{
    bool shadow = true;
    bool cellBox = true;
    bool hitbox = true;
    bool blink = false;
};

const char *const PLANT_OPT_NAMES[4] = {"shadow", "cell box", "hitbox (red)", "blink / flash"};

// Plants::display draws the reanim skin when loaded (synced at gameplay
// speed), falling back to the sheet frames otherwise -- but with one twist:
// the sync is re-checked per state, so a skin missing (or short on) an idle
// segment still falls back to the sheet for THAT state. Returns true when
// the reanim path drew.
bool drawPlantReanim(const AnimState &st, long tick, const Block &cell, bool flash,
                     const unordered_set<string> &extraHide,
                     const unordered_set<string> &extraShow, int &partsDrawn)
{
    if (!reanimplant::ready())
        return false;
    if (st.ptype < 0)
        return false;
    PlantAssets assets = loadPlantAsset(st.cmd_dir);
    int sframe = ((int)tick / assets.frame) % all_img[st.dir].n_sheet;
    int footY = cell.y1 + assets.y_alias + assets.game_height;
    return reanimplant::drawPlant(st.ptype, st.dir, sframe, all_img[st.dir].n_sheet,
                                  (float)tick * reanimplant::frameRate(st.ptype) /
                                      (float)FPS, // drives idle/fidget cycles
                                  cell.x1, cell.x2, footY, assets.game_height, flash,
                                  &extraHide, &extraShow, &partsDrawn);
}

// Raw reanim label playback:
//   zombies -- drawSkinSegment drives any skin (incl. SKIN_PLAIN with the
//   game's base hide rules, and SKIN_FLAGPOLE at its rest pose); stats carry
//   the drawn-part count + screen bbox for the info line.
bool drawZombieLabel(const AnimState &st, float t, int x, int y,
                     const unordered_set<string> &extraHide,
                     const unordered_set<string> &extraShow,
                     int &partsDrawn, reanim::DrawStats &stats)
{
    partsDrawn = 0;
    if (!reanimz::ready())
        return false;
    // Zomboss is authored 2x-scale for the boss fight; shrink him to fit the
    // same 166x144 inspection box (almanac uses an even smaller factor).
    float scaleMul = st.skin == reanimz::SKIN_ZOMBOSS ? 0.45f : 1.0f;
    return reanimz::drawSkinSegment(st.skin, st.seg.c_str(), t, x, y,
                                    st.cold, st.attacked, 0, 0, scaleMul, true,
                                    &extraHide, &extraShow, &partsDrawn, &stats);
}

//   plants -- drawPlantSegment, feet a bit above the cell bottom (no sheet
//   json needed; the raw label keeps the doc's calibration).
bool drawPlantLabel(const AnimState &st, float frame, const Block &cell, bool flash,
                    const unordered_set<string> &extraHide,
                    const unordered_set<string> &extraShow, int &partsDrawn)
{
    partsDrawn = 0;
    if (!reanimplant::ready() || st.ptype < 0)
        return false;
    int footY = cell.y2 - 14;
    return reanimplant::drawPlantSegment(st.ptype, st.seg.c_str(), frame,
                                         cell.x1, cell.x2, footY, flash,
                                         &extraHide, &extraShow, &partsDrawn);
}

// Same as Plants::display: reanim skin first, sprite-sheet fallback, plus
// toggleable helpers. The blink toggle feeds `flash` on the reanim path and
// the *_blink overlay on the sheet path.
void drawPlantState(const AnimState &st, long tick, const Block &cell, const PlantOpts &opts,
                    const unordered_set<string> &extraHide,
                    const unordered_set<string> &extraShow,
                    bool &usedReanim, int &partsDrawn)
{
    PlantAssets assets = loadPlantAsset(st.cmd_dir);
    if (opts.cellBox)
        win.draw_rect(cell.x1, cell.y1, cell.x2 - cell.x1, cell.y2 - cell.y1, WHITE);
    if (opts.shadow)
        drawPlantShadow(cell);
    int sframe = ((int)tick / assets.frame) % all_img[st.dir].n_sheet;
    int x = cell.x1 + assets.x_alias;
    int y = cell.y1 + assets.y_alias;
    usedReanim = drawPlantReanim(st, tick, cell, opts.blink, extraHide, extraShow, partsDrawn);
    if (!usedReanim)
    {
        win.draw_nth_frame(st.dir, sframe, assets.img_width, assets.img_height,
                           x, y, assets.game_width, assets.game_height);
        if (opts.blink)
        {
            int blink = blink_of(st.dir);
            if (blink >= 0)
                win.draw_nth_frame(blink, sframe, assets.img_width, assets.img_height,
                                   x, y, assets.game_width, assets.game_height);
        }
    }
    // game box (hitbox) is identical on both paths
    if (opts.hitbox)
        win.draw_rect(x, y, assets.game_width, assets.game_height, RGB(255, 120, 120));
}

// --- viewer UI (Suburban-Almanac presentation, 3 pages) ----------------------
// INDEX -- the bookshelf with the VIEW PLANTS / VIEW ZOMBIES SeedChooser
// buttons (the almanac's index page); PLANTS -- the almanac plant page:
// chooser-size seed packets in the PlantBack panel's grid zone + the picker
// card; ZOMBIES -- the almanac zombie page: the big 100x100 glass windows in
// the ZombieBack panel + the picker card. The viewer's working controls live
// ON the picker card: the state preview in the almanac patch zone, the
// subject name + state nav, then one scrollable list box that swaps STATES
// (click a state to play it) for PARTS (the per-track table) via the PARTS
// button; plants keep their 4 OPTIONS toggles above the list. Everything
// below the card top derives from the exported page consts (anim_viewer.hpp).
const RGB VIEW_TITLE_GRAY(220, 220, 220); // ALMANAC_TITLE_GRAY
const RGB VIEW_HEAD_GOLD(213, 159, 43);   // ALMANAC_TITLE_GOLD
const RGB VIEW_HEAD_GREEN(0, 196, 0);     // ALMANAC_TITLE_GREEN
const RGB VIEW_BTN_LABEL(42, 42, 90);     // ALMANAC_BUTTON_LABEL
const RGB VIEW_TEXT_NAVY(40, 50, 90);     // ALMANAC_TEXT_BLUE (card body text)
const RGB VIEW_PARCH(232, 226, 208);      // parchment body text on dark plates
const RGB VIEW_PARCH_HOV(255, 235, 180);
const RGB VIEW_INFO(255, 240, 170);

struct Btn
{
    int x, y, w, h;
};

Btn btnOf(const SDL_Rect &r) { return Btn{r.x, r.y, r.w, r.h}; }

// Page buttons (mirror the almanac scene's rects; exported for the check).
const Btn BTN_CLOSE = btnOf(VIEWER_BTN_CLOSE);
const Btn BTN_BACK_INDEX = btnOf(VIEWER_BTN_INDEX);
const Btn BTN_VIEW_PLANTS = btnOf(VIEWER_BTN_VIEW_PLANTS);
const Btn BTN_VIEW_ZOMBIES = btnOf(VIEWER_BTN_VIEW_ZOMBIES);

// Picker-card working zone (the almanac card art rects: zombie (580,78,
// 324x497), plant (584,86,341x494); the content envelope is common).
const int CARD_X = 600, CARD_R = 898, CARD_CLIP_X = 580, CARD_CLIP_TOP = 70,
          CARD_CLIP_BOT = 568;

// Page-local verticals (mirroring the almanac card's text flow: name under
// the patch, the viewer's state nav below it, the list box to the card
// bottom; the plants page keeps its 4 OPTIONS toggle rows between the nav
// and the list header).
int pageNameY(bool zombie) { return zombie ? 320 : 286; }
int pageNavY(bool zombie) { return zombie ? 346 : 312; }
int pageOptsY(bool zombie) { return zombie ? 0 : 338; } // plants only
int pageListHdrY(bool zombie) { return zombie ? 372 : 404; }
int pageListY(bool zombie) { return zombie ? 398 : 428; }
const int LIST_BOT = 540;
const int LIST_LINE = 19;
int pageListRows(bool zombie) { return (LIST_BOT - pageListY(zombie)) / LIST_LINE; }
const int STATUS_Y = 544;

// Small card controls.
Btn navPrevBtn(bool zombie) { return Btn{606, pageNavY(zombie) - 3, 26, 22}; }
Btn navNextBtn(bool zombie) { return Btn{856, pageNavY(zombie) - 3, 26, 22}; }
Btn partsModeBtn(bool zombie) { return Btn{808, pageListHdrY(zombie) - 3, 84, 22}; }
Btn partsAllBtn(bool zombie) { return Btn{600, pageListHdrY(zombie) - 3, 44, 22}; }
Btn partsResetBtn(bool zombie) { return Btn{648, pageListHdrY(zombie) - 3, 54, 22}; }
Btn partsUpBtn(bool zombie) { return Btn{712, pageListHdrY(zombie) - 3, 18, 22}; }
Btn partsDownBtn(bool zombie) { return Btn{734, pageListHdrY(zombie) - 3, 18, 22}; }
Btn frameBtn() { return Btn{786, 540, 104, 22}; }

// playback speed options (multiply the 60 tps / 0.2 reanim-frame-per-tick base)
const float SPEEDS[3] = {1.0f, 0.5f, 2.0f};
const char *const SPEED_NAMES[3] = {"1x", "0.5x", "2x"};

// Page title centered in the 800px page (the almanac's recipes: HOUSEOFTERROR
// 28 on the index page, DWARVESC 24 on the subject pages).
void drawPageTitle(const string &text, const RGB &color, string font, int size)
{
    int w = 0, h = 0;
    TTF_SizeText(win.get_font(font, size), text.c_str(), &w, &h);
    win.show_text(text, (WINDOW_WIDTH - w) / 2, 34, color, font, size);
}

void drawSectionHeader(const string &text, int x, int y, const RGB &color, int size = 18)
{
    win.show_text_shadowed(text, x, y, color, DWARVESC_TTF, size, 1);
}

// Small dark plate + brass border (list box, tooltips) -- legible over the
// page/card art without hiding the almanac dressing.
void fillPlate(int x, int y, int w, int h, int alpha)
{
    win.sprite_batch().fillRect((float)x, (float)y, (float)w, (float)h, 12, 18, 10, alpha);
    win.draw_rect(x, y, w, h, RGB(166, 136, 70));
}

bool hitBtn(const Btn &b, int mx, int my)
{
    return mx >= b.x && mx < b.x + b.w && my >= b.y && my < b.y + b.h;
}

// Small control button: SeedChooser button art + glow on hover, Brianne 16
// label; lit state turns the label gold.
void drawBtn(const Btn &b, const string &text, bool lit, int mx, int my)
{
    bool hov = hitBtn(b, mx, my);
    win.draw_png(SEEDCHOOSER_BUTTON_DIRECTORY, b.x, b.y, b.w, b.h);
    if (hov)
        win.draw_png(SEEDCHOOSER_BUTTON_GLOW_DIRECTORY, b.x, b.y, b.w, b.h);
    int w = 0, h = 0;
    TTF_SizeText(win.get_font(BRIANNE_TTF, 16), text.c_str(), &w, &h);
    win.show_text(text, b.x + (b.w - w) / 2, b.y + (b.h - h) / 2,
                  lit ? VIEW_HEAD_GOLD : WHITE, BRIANNE_TTF, 16);
}

// The almanac's INDEX/CLOSE button recipe (almanac.cpp draw_almanac_button):
// almanac art + BRIANNE 12 navy label.
void drawAlmanacBtn(const Btn &b, int dir, int hiDir, const string &label, int mx, int my)
{
    bool hov = hitBtn(b, mx, my);
    win.draw_png(hov ? hiDir : dir, b.x, b.y, b.w, b.h);
    int w = 0, h = 0;
    TTF_SizeText(win.get_font(BRIANNE_TTF, 12), label.c_str(), &w, &h);
    win.show_text(label, b.x + (b.w - w) / 2, b.y + (b.h - h) / 2, VIEW_BTN_LABEL,
                  BRIANNE_TTF, 12);
}
void drawIndexBtn(const Btn &b, int mx, int my)
{
    drawAlmanacBtn(b, ALMANAC_INDEX_BUTTON_DIRECTORY, ALMANAC_INDEX_BUTTON_HIGHLIGHT_DIRECTORY,
                   "ALMANAC INDEX", mx, my);
}
void drawCloseBtn(const Btn &b, int mx, int my)
{
    drawAlmanacBtn(b, ALMANAC_CLOSE_BUTTON_DIRECTORY, ALMANAC_CLOSE_BUTTON_HIGHLIGHT_DIRECTORY,
                   "CLOSE", mx, my);
}

// The index page's big VIEW PLANTS / VIEW ZOMBIES (the almanac's recipe:
// SeedChooser button art + glow on hover, DWARVESC 24 label).
void drawViewBtn(const Btn &b, const string &label, int mx, int my)
{
    win.draw_png(SEEDCHOOSER_BUTTON_DIRECTORY, b.x, b.y, b.w, b.h);
    if (hitBtn(b, mx, my))
        win.draw_png(SEEDCHOOSER_BUTTON_GLOW_DIRECTORY, b.x, b.y, b.w, b.h);
    int w = 0, h = 0;
    TTF_SizeText(win.get_font(DWARVESC_TTF, 24), label.c_str(), &w, &h);
    win.show_text(label, b.x + (b.w - w) / 2, b.y + (b.h - h) / 2 - 2, WHITE,
                  DWARVESC_TTF, 24);
}

// Slim scrollbar, the almanac idiom (almanac.cpp): the 135x10 slider groove
// rotated 90 into the track + the slider knob's opaque core as the thumb.
void drawSlimScrollbar(const SDL_Rect &track, const SDL_Rect &thumb)
{
    int cx = track.x + track.w / 2, cy = track.y + track.h / 2;
    win.draw_png(ALMANAC_SLIDERSLOT_DIRECTORY, cx - track.h / 2, cy - track.w / 2,
                 track.h, track.w, 90);
    win.draw_png(SLIDER_DIRECTORY, 0, 2, 24, 24, thumb.x, thumb.y, thumb.w, thumb.h);
}
void drawGridScrollbar(bool zombie, int scroll_row)
{
    SDL_Rect track, thumb;
    if (!anim_viewer_grid_scrollbar(zombie, scroll_row, track, thumb))
        return;
    drawSlimScrollbar(track, thumb);
}

// Grid view rect (cell area + the scrollbar strip), the wheel hit zone.
Btn gridViewBtn(bool zombie)
{
    int dx = zombie ? VIEWER_ZCELL_DX : VIEWER_PCELL_DX;
    int dy = zombie ? VIEWER_ZCELL_DY : VIEWER_PCELL_DY;
    int vis = zombie ? VIEWER_ZVIS_ROWS : VIEWER_PVIS_ROWS;
    int gx = zombie ? VIEWER_ZGRID_X : VIEWER_PGRID_X;
    int gy = zombie ? VIEWER_ZGRID_Y : VIEWER_PGRID_Y;
    int right = gx + (VIEWER_GRID_COLS - 1) * dx + (zombie ? VIEWER_ZCELL_W : VIEWER_PCELL_W);
    int bottom = gy + (vis - 1) * dy + (zombie ? VIEWER_ZCELL_H : VIEWER_PCELL_H);
    return Btn{gx, gy, right - gx + 5 + VIEWER_SCROLL_TRACK_W, bottom - gy};
}

int tabCount(bool zombie) { return zombie ? (int)COUNT_ZOMBIE_TYPE : PLANT_COUNT; }
int tabVisRows(bool zombie) { return zombie ? VIEWER_ZVIS_ROWS : VIEWER_PVIS_ROWS; }

// Index (within the page) of the visible grid cell under the mouse, or -1.
// Scrolled-out rows are skipped (their rects land outside the grid zone).
int hitGridCell(bool zombie, int scroll_row, int mx, int my)
{
    for (int i = 0, n = tabCount(zombie); i < n; i++)
    {
        int row = i / VIEWER_GRID_COLS;
        if (row < scroll_row || row >= scroll_row + tabVisRows(zombie))
            continue;
        int x, y, w, h;
        anim_viewer_grid_cell(zombie, i, scroll_row, x, y, w, h);
        if (mx >= x && mx < x + w && my >= y && my < y + h)
            return i;
    }
    return -1;
}

// One grid cell -- the almanac page's own look. Zombies: the big glass
// window + the live reanim walk portrait at the one page scale (scissored to
// the glass) + the window2 frame; plants: the debut-world packet bg + the
// shared-scale portrait + the packet bottom border. Names live in the picker
// card (hover shows them under the cursor), same as the almanac.
void drawGridCell(bool zombie, int idx, int scroll_row, const CharDef &ch, bool selected,
                  bool hovered, float clock)
{
    int x, y, w, h;
    anim_viewer_grid_cell(zombie, idx, scroll_row, x, y, w, h);
    if (!zombie)
    {
        win.draw_png(plant_packet_bg_dir(ch.type), x, y, w, h);
        int margin = ch.type == LILYPAD_TYPE ? 0 : 6;
        int bx = x + 4, by = y + 3, bw = w - 8, bh = h - 3 - margin;
        draw_plant_icon_scaled(ch.type, bx + bw / 2, by + bh, bw, bh, 1.0f);
        win.draw_png_height_scaled(SEED_PACKET_BOT_BORDER_DIRECTORY, x, y + 60, w);
        if (selected || hovered)
            win.draw_png(BUTTON_BLINK_DIRECTORY, x, y, w, h);
        return;
    }
    win.draw_png(ALMANAC_ZOMBIE_WINDOW_DIRECTORY, x, y, w, h);
    SDL_Rect contain{x + 2, y + 2, w - 4, h - 4};
    win.sprite_batch().setClipRect(&contain);
    drawGridZombie(ch.type, clock, x, y);
    win.sprite_batch().setClipRect(nullptr);
    // glass frame over the zombie (overlaps the neighbors a touch, as authored)
    win.draw_png(ALMANAC_ZOMBIE_WINDOW2_DIRECTORY, x, y, 112, 106);
    if (selected || hovered)
        win.draw_png(BUTTON_BLINK_DIRECTORY, x, y, w, h);
}

// --- picker card: states / parts list box ---------------------------------

// The scrollable list plate on the card (STATES default; PARTS via the
// header toggle).
void drawListPlate(bool zombie)
{
    fillPlate(CARD_X - 8, pageListHdrY(zombie) - 8, CARD_R - CARD_X + 16,
              LIST_BOT - pageListHdrY(zombie) + 26, 150);
}

// STATES rows: one per preview state of the subject; the current state is
// gold, hover lights. The rows follow the selection (UP/DOWN, wheel, < >).
void drawStatesList(CharDef &ch, int selState, bool zombie, int mx, int my)
{
    int y0 = pageListY(zombie), rows = pageListRows(zombie);
    int n = (int)ch.states.size();
    int scroll = 0;
    if (selState >= rows)
        scroll = selState - rows + 1;
    for (int i = scroll, r = 0; i < n && r < rows; i++, r++)
    {
        int ty = y0 + r * LIST_LINE;
        bool hov = mx >= CARD_X && mx < CARD_R && my >= ty && my < ty + LIST_LINE;
        RGB c = i == selState ? RGB(255, 224, 120) : hov ? VIEW_PARCH_HOV : VIEW_PARCH;
        string s = to_string(i + 1) + "/" + to_string(n) + "  " + ch.states[i].label;
        if ((int)s.size() > 44)
            s = s.substr(0, 42) + "..";
        win.show_text_shadowed(s, CARD_X + 4, ty + 2, c, BRIANNE_TTF, 14, 1);
    }
}

// Index of the states-list row under the mouse ({row}, or -1). The rows
// follow the selection, so the click maps back through the same scroll.
int statesRowAt(CharDef &ch, int selState, bool zombie, int mx, int my)
{
    int y0 = pageListY(zombie), rows = pageListRows(zombie);
    if (mx < CARD_X || mx >= CARD_R || my < y0)
        return -1;
    int r = (my - y0) / LIST_LINE;
    int scroll = 0;
    if (selState >= rows)
        scroll = selState - rows + 1;
    int i = scroll + r;
    return i >= 0 && i < (int)ch.states.size() && r < rows ? i : -1;
}

// PARTS rows of the current subject's doc. Colors:
//   white  = track actually drawn at the preview frame (f >= 0 or force-shown)
//   gray   = has an image but hidden (state rules / your click / f=-1 in data)
//   dark   = track carries no image at all (anim_* marker windows etc.)
// " *" suffix = you overrode the default (hide or force-show), RESET clears.
// Returns the hovered track name ("" = none) for the status-line tooltip.
string drawPartsList(CharDef &ch, bool zombie,
                     const unordered_map<string, reanimz::TrackStatus> &tmap,
                     bool hasStats, int mx, int my)
{
    drawBtn(partsAllBtn(zombie), "ALL", ch.showAll, mx, my);
    drawBtn(partsResetBtn(zombie), "RESET",
            !ch.userShow.empty() || !ch.userHide.empty() || ch.showAll, mx, my);
    drawBtn(partsUpBtn(zombie), "^", false, mx, my);
    drawBtn(partsDownBtn(zombie), "v", false, mx, my);

    int y0 = pageListY(zombie), rows = pageListRows(zombie);
    string hovered;
    for (int row = ch.scroll; row < (int)ch.items.size() && row < ch.scroll + rows; row++)
    {
        const PanelItem &it = ch.items[row];
        int y = y0 + (row - ch.scroll) * LIST_LINE;
        bool hov = my >= y && my < y + LIST_LINE && mx >= CARD_X && mx < CARD_R;
        if (it.header)
        {
            int shown = 0;
            for (const string &m : groupTracks(ch, it.group))
            {
                auto si = tmap.find(m);
                if (si != tmap.end() && trackWillDraw(si->second, ch.showAll, ch.userShow, ch.userHide))
                    shown++;
            }
            string s = (shown > 0 ? "[x] " : "[ ] ") + it.text;
            win.show_text_shadowed(s, CARD_X, y + 2, hov ? RGB(255, 230, 130) : RGB(255, 200, 40),
                                   BRIANNE_TTF, 13, 1);
            if (hov)
                hovered = "group:" + it.group;
            continue;
        }
        auto si = tmap.find(it.track);
        string s;
        RGB c(200, 200, 200);
        if (!hasStats || si == tmap.end())
        {
            s = "  - " + it.text;
            c = hov ? RGB(200, 230, 255) : RGB(200, 200, 200);
        }
        else
        {
            const reanimz::TrackStatus &ts = si->second;
            bool drawn = trackWillDraw(ts, ch.showAll, ch.userShow, ch.userHide);
            s = string(drawn ? "[x] " : "[ ] ") + it.text;
            if (ch.userShow.count(it.track) || ch.userHide.count(it.track))
                s += " *";
            if (!ts.hasImage)
                c = RGB(100, 100, 100);
            else if (drawn)
                c = hov ? RGB(200, 230, 255) : RGB(235, 235, 235);
            else
                c = hov ? RGB(190, 190, 200) : RGB(150, 150, 150);
        }
        win.show_text_shadowed(s, CARD_X + 8, y + 2, c, BRIANNE_TTF, 13, 1);
        if (hov)
            hovered = it.track;
    }
    (void)zombie;
    return hovered;
}

// Plant OPTIONS block (4 toggle rows above the list box). Returns nothing;
// hitRows handled by optRowAt below.
void drawPlantOpts(const PlantOpts &opts, bool zombie, int mx, int my)
{
    drawSectionHeader("OPTIONS", CARD_X, pageOptsY(zombie), VIEW_HEAD_GOLD, 16);
    const bool on[4] = {opts.shadow, opts.cellBox, opts.hitbox, opts.blink};
    for (int i = 0; i < 4; i++)
    {
        int y = pageOptsY(zombie) + 20 + i * 16;
        bool hov = mx >= CARD_X && mx < CARD_R && my >= y && my < y + 16;
        string s = string("[") + (on[i] ? "x" : " ") + "] " + PLANT_OPT_NAMES[i];
        win.show_text_shadowed(s, CARD_X + 4, y, hov ? VIEW_PARCH_HOV : VIEW_PARCH,
                               BRIANNE_TTF, 13, 1);
    }
}

// OPTIONS row under the mouse (0..3), or -1.
int optRowAt(bool zombie, int mx, int my)
{
    if (zombie)
        return -1;
    for (int i = 0; i < 4; i++)
    {
        int y = pageOptsY(zombie) + 20 + i * 16;
        if (mx >= CARD_X && mx < CARD_R && my >= y && my < y + 16)
            return i;
    }
    return -1;
}

} // namespace

// --- exported layout/inspection helpers (anim_viewer.hpp) ---------------------

void anim_viewer_grid_cell(bool zombie, int idx, int scroll_row, int &x, int &y, int &w, int &h)
{
    w = zombie ? VIEWER_ZCELL_W : VIEWER_PCELL_W;
    h = zombie ? VIEWER_ZCELL_H : VIEWER_PCELL_H;
    x = (zombie ? VIEWER_ZGRID_X : VIEWER_PGRID_X) +
        (idx % VIEWER_GRID_COLS) * (zombie ? VIEWER_ZCELL_DX : VIEWER_PCELL_DX);
    y = (zombie ? VIEWER_ZGRID_Y : VIEWER_PGRID_Y) +
        (idx / VIEWER_GRID_COLS - scroll_row) * (zombie ? VIEWER_ZCELL_DY : VIEWER_PCELL_DY);
}

int anim_viewer_grid_max_scroll_row(bool zombie)
{
    int n = zombie ? (int)COUNT_ZOMBIE_TYPE : (int)PLANT_COUNT;
    int rows = (n + VIEWER_GRID_COLS - 1) / VIEWER_GRID_COLS;
    int vis = zombie ? VIEWER_ZVIS_ROWS : VIEWER_PVIS_ROWS;
    return rows > vis ? rows - vis : 0;
}

bool anim_viewer_grid_scrollbar(bool zombie, int scroll_row, SDL_Rect &track, SDL_Rect &thumb)
{
    int maxRow = anim_viewer_grid_max_scroll_row(zombie);
    if (maxRow <= 0)
        return false;
    int vis = zombie ? VIEWER_ZVIS_ROWS : VIEWER_PVIS_ROWS;
    int gx = zombie ? VIEWER_ZGRID_X : VIEWER_PGRID_X;
    int gy = zombie ? VIEWER_ZGRID_Y : VIEWER_PGRID_Y;
    int dx = zombie ? VIEWER_ZCELL_DX : VIEWER_PCELL_DX;
    int dy = zombie ? VIEWER_ZCELL_DY : VIEWER_PCELL_DY;
    int cw = zombie ? VIEWER_ZCELL_W : VIEWER_PCELL_W;
    int chh = zombie ? VIEWER_ZCELL_H : VIEWER_PCELL_H;
    int top = gy;
    int bottom = gy + (vis - 1) * dy + chh; // visible rows exactly
    int trackX = gx + (VIEWER_GRID_COLS - 1) * dx + cw + 5; // 5px off the cells
    if (scroll_row < 0)
        scroll_row = 0;
    if (scroll_row > maxRow)
        scroll_row = maxRow;
    track = SDL_Rect{trackX, top, VIEWER_SCROLL_TRACK_W, bottom - top};
    int travel = track.h - VIEWER_SCROLL_THUMB;
    thumb = SDL_Rect{track.x + (VIEWER_SCROLL_TRACK_W - VIEWER_SCROLL_THUMB) / 2,
                     top + travel * scroll_row / maxRow,
                     VIEWER_SCROLL_THUMB, VIEWER_SCROLL_THUMB};
    return true;
}

int anim_viewer_grid_scroll_row_at(bool zombie, int track_y)
{
    int maxRow = anim_viewer_grid_max_scroll_row(zombie);
    if (maxRow <= 0)
        return 0;
    SDL_Rect track, thumb;
    anim_viewer_grid_scrollbar(zombie, 0, track, thumb);
    int travel = track.h - VIEWER_SCROLL_THUMB;
    int row = travel > 0 ? (int)((double)(track_y - track.y) * maxRow / travel + 0.5) : 0;
    if (row < 0)
        row = 0;
    if (row > maxRow)
        row = maxRow;
    return row;
}

// State labels of one subject (game: states first, then the raw reanim
// labels), in the viewer's own order -- the check pins the classic roster's
// idle replacement states through this.
std::vector<std::string> anim_viewer_state_labels(bool zombie, int type)
{
    static vector<CharDef> chars = buildChars();
    std::vector<std::string> out;
    int idx = zombie ? type : (int)COUNT_ZOMBIE_TYPE + type;
    if (idx < 0 || idx >= (int)chars.size())
        return out;
    for (const AnimState &s : chars[idx].states)
        out.push_back(s.label);
    return out;
}

// F12: flush + read back the current frame into a PNG (designer eyeball
// dumps; the headless check drops the viewer's decorated frame to
// /tmp/anim_viewer.png through the same path).
void anim_viewer_dump_frame(const char *path)
{
    win.sprite_batch().flush();
    vector<unsigned char> buf((size_t)WINDOW_WIDTH * WINDOW_HEIGHT * 4);
    pvzgl::glReadPixels(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, pvzgl::PVZ_GL_RGBA,
                        pvzgl::PVZ_GL_UNSIGNED_BYTE, buf.data());
    // The vendored IMG_SavePNG dumps surface memory verbatim, ignoring pixel
    // masks: ABGR8888 little-endian keeps R,G,B,A byte order (mirrors
    // demo/include/check_runtime.cpp's dump_png).
    SDL_Surface *shot = SDL_CreateRGBSurfaceWithFormat(0, WINDOW_WIDTH, WINDOW_HEIGHT, 32,
                                                       SDL_PIXELFORMAT_ABGR8888);
    for (int y = 0; y < WINDOW_HEIGHT; y++)
        memcpy((char *)shot->pixels + (size_t)y * shot->pitch,
               buf.data() + (size_t)(WINDOW_HEIGHT - 1 - y) * WINDOW_WIDTH * 4,
               (size_t)WINDOW_WIDTH * 4);
    if (IMG_SavePNG(shot, path) != 0)
        SDL_Log("anim_viewer: IMG_SavePNG %s failed: %s", path, IMG_GetError());
    else
        SDL_Log("anim_viewer: frame dumped to %s", path);
    SDL_FreeSurface(shot);
}

void display_anim_viewer()
{
    if (!reanimz::ready())
        reanimz::init(win.sprite_renderer());
    if (!reanimplant::ready())
        reanimplant::init(win.sprite_renderer());

    static vector<CharDef> chars = buildChars();
    // One selection index per subject page (chars[] is the zombie block first,
    // then plants); selChar is derived from the active page each frame.
    int pickTab[2] = {0, 0};   // [0] plants, [1] zombies
    int gridScroll[2] = {0, 0}; // grid scroll row per subject page
    int page = VIEWER_PAGE_INDEX;
    int prevPick = -1, prevPage = -1; // scroll-follow triggers: pick/page change
    int selState = 0;
    bool showParts = false; // the picker card's list box: STATES (false) | PARTS
    float tick = 0.0f; // sheet-path clock (game ticks; fractional at 0.5x)
    float rt = 0.0f;   // reanim-path clock (reanim frames, 0.2/tick at 1x)
    float idxClock = 0.0f; // INDEX page mascot clock
    bool compare = false, paused = false, typing = false, leave = false;
    int dumpPending = 0; // F12: deferred snapshot (runs while the viewer stays)
    int speedIdx = 0; // SPEEDS[] index
    string frameBuf;
    PlantOpts plantOpts;

    const int xoff = (WINDOW_WIDTH - 800) / 2; // the almanac scene's page offset

    while (!quit && (!leave || dumpPending > 0))
    {
        int mx = 0, my = 0;
        SDL_GetMouseState(&mx, &my);

        // ------------------------------------------------------------------
        // INDEX page: the bookshelf -- VIEW PLANTS / VIEW ZOMBIES SeedChooser
        // buttons, the sunflower / zombie mascots, CLOSE.
        // ------------------------------------------------------------------
        if (page == VIEWER_PAGE_INDEX)
        {
            win.clear_renderer();
            win.draw_png(ALMANAC_INDEX_BACK_DIRECTORY, xoff, 0, 800, 600);
            drawPageTitle("ANIMATION VIEWER", VIEW_TITLE_GRAY, HOUSEOFTERROR_TTF, 28);

            // Mascots on the two book piles, anchored like the almanac index
            // (sunflower bottom-center ~(322,340); the zombie at full in-game
            // scale above the VIEW ZOMBIES button).
            if (reanimplant::ready())
            {
                int ns = all_img[SUNFLOWER_SHEET_DIRECTORY].n_sheet;
                if (ns <= 0)
                    ns = 60;
                reanimplant::drawPlant(SUNFLOWER_TYPE, SUNFLOWER_SHEET_DIRECTORY,
                                       (float)((long)idxClock % ns), ns, idxClock,
                                       322 - 40, 322 + 40, 340, 60, false);
            }
            if (reanimz::ready())
                reanimz::drawZombie(ZOMBIE_WALK1_DIRECTORY, -1, idxClock, 584, 202,
                                    false, false);

            drawViewBtn(BTN_VIEW_PLANTS, "VIEW PLANTS", mx, my);
            drawViewBtn(BTN_VIEW_ZOMBIES, "VIEW ZOMBIES", mx, my);
            drawCloseBtn(BTN_CLOSE, mx, my);
            win.show_text_shadowed("VIEW buttons / TAB / LEFT RIGHT: pages   "
                                   "ESC/F1/CLOSE: exit",
                                   15, WINDOW_HEIGHT - 32, VIEW_PARCH, BRIANNE_TTF, 16, 1);

            HANDLE_SDL_EVENT(
                QUIT();
                LCLICK({
                    if (hitBtn(BTN_CLOSE, mouse_x, mouse_y))
                        leave = true;
                    else if (hitBtn(BTN_VIEW_PLANTS, mouse_x, mouse_y))
                    {
                        page = VIEWER_PAGE_PLANTS;
                        selState = 0;
                        showParts = false;
                        tick = 0.0f;
                        rt = 0.0f;
                    }
                    else if (hitBtn(BTN_VIEW_ZOMBIES, mouse_x, mouse_y))
                    {
                        page = VIEWER_PAGE_ZOMBIES;
                        selState = 0;
                        showParts = false;
                        tick = 0.0f;
                        rt = 0.0f;
                    }
                });
                KEY_PRESS(ESCAPE, leave = true;);
                KEY_PRESS(F1, leave = true;);
                KEY_PRESS(F12, dumpPending = 2;);
                KEY_PRESS(TAB, {
                    page = VIEWER_PAGE_ZOMBIES;
                    selState = 0;
                    showParts = false;
                    tick = 0.0f;
                    rt = 0.0f;
                });
                KEY_PRESS(LEFT, {
                    page = VIEWER_PAGE_ZOMBIES;
                    selState = 0;
                    showParts = false;
                    tick = 0.0f;
                    rt = 0.0f;
                });
                KEY_PRESS(RIGHT, {
                    page = VIEWER_PAGE_PLANTS;
                    selState = 0;
                    showParts = false;
                    tick = 0.0f;
                    rt = 0.0f;
                });
            );

            if (dumpPending > 0 && --dumpPending == 0)
                anim_viewer_dump_frame("/tmp/anim_viewer.png");
            idxClock += 0.2f;
            win.update_screen();
            continue;
        }

        // ------------------------------------------------------------------
        // Subject pages (PLANTS / ZOMBIES): the almanac page art, the subject
        // grid + slim scrollbar, the picker card with the viewer's working
        // controls.
        // ------------------------------------------------------------------
        const bool zombieTab = page == VIEWER_PAGE_ZOMBIES;
        int selChar = zombieTab ? pickTab[1] : (int)COUNT_ZOMBIE_TYPE + pickTab[0];
        int tabN = tabCount(zombieTab);
        CharDef &ch = chars[selChar];
        buildCharPanel(ch);

        // grid scroll: clamp, then follow the selection ONLY when the pick or
        // the page changed (wheel/scrollbar browsing must not snap back)
        int &gScroll = gridScroll[zombieTab ? 1 : 0];
        const int gMaxScroll = anim_viewer_grid_max_scroll_row(zombieTab);
        if (gScroll > gMaxScroll)
            gScroll = gMaxScroll;
        if (gScroll < 0)
            gScroll = 0;
        {
            int curPick = pickTab[zombieTab ? 1 : 0];
            if (curPick != prevPick || page != prevPage)
            {
                int selRow = curPick / VIEWER_GRID_COLS;
                int vis = tabVisRows(zombieTab);
                if (selRow < gScroll)
                    gScroll = selRow;
                if (selRow >= gScroll + vis)
                    gScroll = selRow - vis + 1;
                prevPick = curPick;
                prevPage = page;
            }
        }

        int hovGrid = hitGridCell(zombieTab, gScroll, mx, my);
        SDL_Rect sbTrack, sbThumb;
        const bool sbVisible = anim_viewer_grid_scrollbar(zombieTab, gScroll, sbTrack, sbThumb);
        const Btn viewZone = gridViewBtn(zombieTab); // cells + the scrollbar strip
        const bool gridHover = mx >= viewZone.x && mx < viewZone.x + viewZone.w &&
                               my >= viewZone.y && my < viewZone.y + viewZone.h;

        auto resetClock = [&]() {
            tick = 0.0f;
            rt = 0.0f;
        };
        // LEFT/RIGHT walk the grid of the active page (wrap)
        auto navSubject = [&](int d) {
            int &p = pickTab[zombieTab ? 1 : 0];
            if ((p += d) < 0)
                p = tabN - 1;
            if (p >= tabN)
                p = 0;
            selState = 0;
            resetClock();
        };
        // TAB leaves toward the other page, then back to INDEX
        auto cyclePage = [&]() {
            page = zombieTab ? VIEWER_PAGE_PLANTS : VIEWER_PAGE_INDEX;
            selState = 0;
            showParts = false;
            resetClock();
        };

        // page art + title + subject grid + scrollbar + the card's preview
        // backdrop plate (the card art overlays the preview later)
        auto drawChrome = [&]()
        {
            win.clear_renderer();
            win.draw_png(zombieTab ? ALMANAC_ZOMBIEBACK_DIRECTORY : ALMANAC_PLANTBACK_DIRECTORY,
                         xoff, 0, 800, 600);
            drawPageTitle(zombieTab ? "ANIMATION VIEWER - ZOMBIES"
                                    : "ANIMATION VIEWER - PLANTS",
                          zombieTab ? VIEW_HEAD_GREEN : VIEW_HEAD_GOLD, DWARVESC_TTF, 24);
            int selIdx = pickTab[zombieTab ? 1 : 0];
            int base = zombieTab ? 0 : (int)COUNT_ZOMBIE_TYPE;
            for (int i = 0; i < tabN; i++)
            {
                int row = i / VIEWER_GRID_COLS;
                if (row < gScroll || row >= gScroll + tabVisRows(zombieTab))
                    continue; // scrolled out of the view
                drawGridCell(zombieTab, i, gScroll, chars[base + i], i == selIdx,
                             i == hovGrid, rt);
            }
            drawGridScrollbar(zombieTab, gScroll);
            const SDL_Rect &pp = zombieTab ? VIEWER_ZPATCH : VIEWER_PPATCH;
            win.sprite_batch().fillRect((float)pp.x, (float)pp.y, (float)pp.w, (float)pp.h,
                                        20, 34, 14, 255);
        };

        // the almanac picker card art over the preview zone
        auto drawCardOverlay = [&]()
        {
            win.draw_png(zombieTab ? ALMANAC_ZOMBIE_CARD_DIRECTORY : ALMANAC_PLANT_CARD_DIRECTORY,
                         zombieTab ? 580 : 584, zombieTab ? 78 : 86,
                         zombieTab ? 324 : 341, zombieTab ? 497 : 494);
        };

        // Asset-less subject (missing resources/reanim or sheets): the subject
        // picker still works so the designer can move on.
        if (ch.states.empty())
        {
            drawChrome();
            drawCardOverlay();
            const SDL_Rect &pp = zombieTab ? VIEWER_ZPATCH : VIEWER_PPATCH;
            drawSectionHeader(ch.name, CARD_X, pageNameY(zombieTab),
                              zombieTab ? VIEW_HEAD_GREEN : VIEW_HEAD_GOLD);
            win.show_text_shadowed("no animation data for this subject",
                                   pp.x + 8, pp.y + 88, RGB(255, 120, 120),
                                   BRIANNE_TTF, 18, 1);
            drawIndexBtn(BTN_BACK_INDEX, mx, my);
            drawCloseBtn(BTN_CLOSE, mx, my);
            HANDLE_SDL_EVENT(
                QUIT();
                LCLICK({
                    if (hitBtn(BTN_CLOSE, mouse_x, mouse_y))
                        leave = true;
                    else if (hitBtn(BTN_BACK_INDEX, mouse_x, mouse_y))
                        page = VIEWER_PAGE_INDEX;
                    else if (sbVisible && mouse_x >= sbTrack.x - 2 &&
                             mouse_x < sbTrack.x + sbTrack.w + 2 &&
                             mouse_y >= sbTrack.y && mouse_y < sbTrack.y + sbTrack.h)
                        gScroll = anim_viewer_grid_scroll_row_at(zombieTab, mouse_y);
                    else
                    {
                        int h = hitGridCell(zombieTab, gScroll, mouse_x, mouse_y);
                        if (h >= 0)
                        {
                            pickTab[zombieTab ? 1 : 0] = h;
                            selState = 0;
                            resetClock();
                        }
                    }
                });
                if (e.type == SDL_MOUSEWHEEL && gridHover)
                    gScroll -= e.wheel.y; // clamped at the top of the loop
                KEY_PRESS(ESCAPE, page = VIEWER_PAGE_INDEX;);
                KEY_PRESS(F1, page = VIEWER_PAGE_INDEX;);
                KEY_PRESS(F12, dumpPending = 2;);
                KEY_PRESS(TAB, cyclePage(););
                KEY_PRESS(LEFT, navSubject(-1););
                KEY_PRESS(RIGHT, navSubject(1););
            );
            if (dumpPending > 0 && --dumpPending == 0)
                anim_viewer_dump_frame("/tmp/anim_viewer.png");
            win.update_screen();
            continue;
        }

        AnimState &st = ch.states[selState];
        int clk = 1, nf = 1;
        sheetInfo(st, clk, nf);
        if (nf <= 0)
            nf = 1;
        const long ltick = (long)floorf(tick);
        const float speed = SPEEDS[speedIdx];
        const bool labelPath = st.mode == P_SKIN_LABEL;
        // reanim-frame clock states: every zombie state when reanimz loaded;
        // plant states only the raw label playback (sheet states keep the
        // sheet clock so their "sync" preview works)
        const bool rtDriven = (ch.is_zombie && reanimz::ready()) ||
                              (!ch.is_zombie && labelPath && reanimplant::ready());
        const float period = labelPath ? labelPeriod(st)
                                       : (ch.is_zombie ? reanimPeriod(st) : 0.0f);

        auto stepFrame = [&](int d) {
            paused = true;
            if (rtDriven) // step in reanim frames
            {
                rt += (float)d;
                while (rt < 0.0f)
                    rt += period;
                rt = fmodf(rt, period);
                tick = rt * 5.0f;
            }
            else // step in sheet frames
            {
                long idx = (ltick / clk + d) % nf;
                if (idx < 0)
                    idx += nf;
                tick = (float)(idx * clk);
            }
        };
        auto commitJump = [&]()
        {
            typing = false;
            SDL_StopTextInput();
            if (frameBuf.empty())
                return;
            int v = atoi(frameBuf.c_str());
            if (rtDriven) // number = reanim frame inside the segment/label
            {
                int vmax = (int)period - 1;
                if (v < 0)
                    v = 0;
                if (v > vmax)
                    v = vmax;
                rt = (float)v;
                tick = rt * 5.0f;
            }
            else // number = sheet frame [0, n_sheet-1]
            {
                if (v < 0)
                    v = 0;
                if (v > nf - 1)
                    v = nf - 1;
                tick = (float)(v * clk);
            }
        };

        // per-track snapshot at the preview frame; death/drop states draw no
        // body doc, sheet-fallback plant states have no live status either.
        unordered_map<string, reanimz::TrackStatus> smap;
        bool hasStats = false;
        if (ch.is_zombie && reanimz::ready())
        {
            if (st.mode == P_REANIM_LOOP)
                for (const auto &ts : reanimz::trackStatuses(st.dir, st.armor, rt))
                    smap[ts.name] = ts;
            else if (st.mode == P_REANIM_IDLE)
            {
                float window = reanimPeriod(st);
                float local = st.segFirst + (window > 0.0f ? fmodf(rt, window) : 0.0f);
                for (const auto &ts : reanimz::segmentTrackStatuses(st.skin, st.seg.c_str(), local))
                    smap[ts.name] = ts;
            }
            else if (labelPath)
                for (const auto &ts : reanimz::segmentTrackStatuses(st.skin, st.seg.c_str(), rt))
                    smap[ts.name] = ts;
            hasStats = !smap.empty();
        }
        else if (!ch.is_zombie && reanimplant::ready() && st.ptype >= 0)
        {
            if (labelPath)
            {
                for (const auto &ts : reanimplant::segmentTrackStatuses(st.ptype, st.seg.c_str(), rt))
                    smap[ts.name] = adaptTrackStatus(ts);
            }
            else
            {
                int sframe = nf > 0 ? (int)(ltick / clk) % nf : 0;
                for (const auto &pts : reanimplant::trackStatuses(st.ptype, st.dir, sframe,
                                                                  all_img[st.dir].n_sheet))
                    smap[pts.name] = adaptTrackStatus(pts);
            }
            hasStats = !smap.empty();
        }

        // the card's one list box (STATES | PARTS): scroll clamps + the zone
        const int rows = pageListRows(zombieTab);
        int maxScroll = (int)ch.items.size() > rows ? (int)ch.items.size() - rows : 0;
        if (ch.scroll > maxScroll)
            ch.scroll = maxScroll;
        if (ch.scroll < 0)
            ch.scroll = 0;
        const bool listHover = mx >= CARD_X - 8 && mx < CARD_R + 8 &&
                               my >= pageListHdrY(zombieTab) - 8 && my < LIST_BOT + 20;
        const bool partsHover = showParts && listHover;

        // viewer-driven track visibility (zombie/plant reanim paths)
        unordered_set<string> extraHide, extraShow;
        if (ch.showAll)
            extraShow.insert(ch.allTracks.begin(), ch.allTracks.end());
        else
        {
            extraHide = ch.userHide;
            extraShow = ch.userShow;
        }

        drawChrome();

        // ---- state preview on the picker card, clipped to the card zone ----
        bool used_reanim = false;
        int partsDrawn = 0;
        bool haveBounds = false;
        int boxW = 0, boxH = 0;
        string hoveredTrack;
        SDL_Rect clip{CARD_CLIP_X, CARD_CLIP_TOP, 330, CARD_CLIP_BOT - CARD_CLIP_TOP};
        win.sprite_batch().setClipRect(&clip);
        if (ch.is_zombie)
        {
            for (int vi = 0; vi < (compare && !labelPath ? 2 : 1); vi++)
            {
                Block cell = previewCell(true, vi, compare && !labelPath);
                int x = cell.x1, y = cell.y1 - 50; // the zombie game box's top
                // lawn cell + zombie hitbox + shadow (same coords as in-game)
                win.draw_rect(cell.x1, cell.y1, cell.x2 - cell.x1, cell.y2 - cell.y1, WHITE);
                if (st.mode == P_REANIM_LOOP || st.mode == P_REANIM_IDLE ||
                    (labelPath && st.skin != reanimz::SKIN_FLAGPOLE))
                    drawZombieShadow(cell);
                win.draw_rect(x, y, ZOMBIE_WIDTH, ZOMBIE_HEIGHT, RGB(255, 120, 120));
                bool sheet_side = compare && !labelPath && vi == 1;
                if (sheet_side)
                    drawZombieSheet(st, ltick, x, y);
                else
                {
                    reanim::DrawStats stt;
                    if (labelPath)
                        used_reanim = drawZombieLabel(st, rt, x, y, extraHide, extraShow, partsDrawn, stt);
                    else
                        used_reanim = drawZombieReanim(st, rt, x, y, extraHide, extraShow, partsDrawn, &stt);
                    if (stt.hasBounds)
                    {
                        haveBounds = true;
                        boxW = (int)(stt.maxX - stt.minX);
                        boxH = (int)(stt.maxY - stt.minY);
                    }
                }
                if (!sheet_side && !used_reanim && !labelPath)
                    drawZombieSheet(st, ltick, x, y);
                win.show_text_shadowed(sheet_side ? "SHEET"
                                       : labelPath ? (used_reanim ? "LABEL" : "LABEL (no data)")
                                                   : (used_reanim ? "REANIM" : "SHEET (fallback)"),
                                       x + 2, y - 4, RGB(255, 255, 120), BRIANNE_TTF, 12, 1);
            }
        }
        else
        {
            Block cell = previewCell(false, 0, false);
            if (labelPath)
            {
                if (plantOpts.cellBox)
                    win.draw_rect(cell.x1, cell.y1, cell.x2 - cell.x1, cell.y2 - cell.y1, WHITE);
                if (plantOpts.shadow)
                    drawPlantShadow(cell);
                used_reanim = drawPlantLabel(st, rt, cell, plantOpts.blink, extraHide, extraShow, partsDrawn);
                win.show_text_shadowed(used_reanim ? "LABEL" : "LABEL (no data)",
                                       cell.x1, cell.y1 - 24, RGB(255, 255, 120), BRIANNE_TTF, 12, 1);
            }
            else
            {
                drawPlantState(st, ltick, cell, plantOpts, extraHide, extraShow,
                               used_reanim, partsDrawn);
                win.show_text_shadowed(used_reanim ? "REANIM" : "SHEET (fallback)",
                                       cell.x1, cell.y1 - 24, RGB(255, 255, 120), BRIANNE_TTF, 12, 1);
            }
        }
        win.sprite_batch().setClipRect(nullptr);

        drawCardOverlay();

        // ---- picker card text zone: name, state nav, OPTIONS, the list ----
        {
            int nw = 0, nh = 0;
            TTF_SizeText(win.get_font(DWARVESC_TTF, 18), ch.name.c_str(), &nw, &nh);
            win.show_text_shadowed(ch.name, CARD_X + (CARD_R - CARD_X - nw) / 2,
                                   pageNameY(zombieTab),
                                   zombieTab ? VIEW_HEAD_GREEN : VIEW_HEAD_GOLD,
                                   DWARVESC_TTF, 18, 1);
        }
        drawBtn(navPrevBtn(zombieTab), "<", false, mx, my);
        drawBtn(navNextBtn(zombieTab), ">", false, mx, my);
        {
            string s = to_string(selState + 1) + "/" + to_string(ch.states.size()) +
                       "  " + st.label;
            int sw = 0, sh = 0;
            TTF_SizeText(win.get_font(BRIANNE_TTF, 13), s.c_str(), &sw, &sh);
            win.show_text_shadowed(s, CARD_X + (CARD_R - CARD_X - sw) / 2,
                                   pageNavY(zombieTab), VIEW_PARCH, BRIANNE_TTF, 13, 1);
        }
        if (!zombieTab)
            drawPlantOpts(plantOpts, false, mx, my);
        drawListPlate(zombieTab);
        drawSectionHeader(showParts ? "PARTS" : "STATES", CARD_X, pageListHdrY(zombieTab),
                          zombieTab ? VIEW_HEAD_GREEN : VIEW_HEAD_GOLD, 16);
        drawBtn(partsModeBtn(zombieTab), showParts ? "STATES" : "PARTS", showParts, mx, my);
        if (showParts)
        {
            if ((ch.is_zombie ? reanimz::ready() : reanimplant::ready()) && !ch.items.empty())
                hoveredTrack = drawPartsList(ch, zombieTab, smap, hasStats, mx, my);
            else
                win.show_text_shadowed("reanim assets missing", CARD_X + 8, pageListY(zombieTab),
                                       RGB(255, 120, 120), BRIANNE_TTF, 13, 1);
        }
        else
            drawStatesList(ch, selState, zombieTab, mx, my);

        // status line + the frame-jump box
        string info = ch.name + " / " + st.label;
        info += "  [" + string(SPEED_NAMES[speedIdx]);
        if (paused)
            info += " PAUSED";
        if (ch.showAll)
            info += " ALL parts";
        info += "]";
        if (labelPath)
        {
            char tmp[96];
            snprintf(tmp, sizeof tmp, "  frame %d/%d in %s (abs %d..%d)  parts %d",
                     (int)rt, (int)period - 1, st.seg.c_str(), st.segFirst, st.segLast,
                     partsDrawn);
            info += tmp;
            if (haveBounds)
            {
                snprintf(tmp, sizeof tmp, "  bbox %dx%d", boxW, boxH);
                info += tmp;
            }
        }
        else if (ch.is_zombie && used_reanim)
        {
            char tmp[96];
            snprintf(tmp, sizeof tmp, "  t=%.1f/%d (reanim frames)  parts %d",
                     rt, (int)period, partsDrawn);
            info += tmp;
            if (haveBounds)
            {
                snprintf(tmp, sizeof tmp, "  bbox %dx%d", boxW, boxH);
                info += tmp;
            }
        }
        else
        {
            info += "  f " + to_string((int)(ltick / clk)) + "/" + to_string(nf) +
                    " sheet frames (";
            int dirShow = st.mode == P_REANIM_DEATH ? ZOMBIE_DIE1_DIRECTORY
                          : st.sheet_dir >= 0        ? st.sheet_dir
                                                     : st.dir;
            info += baseName(all_img[dirShow].img_dir) + ", " + to_string(clk) + " t/f)";
            if (!ch.is_zombie && used_reanim)
                info += " synced -> reanim segment";
            if (partsDrawn > 0)
                info += "  parts " + to_string(partsDrawn);
        }
        win.show_text_shadowed(info, CARD_X - 6, STATUS_Y, VIEW_INFO, BRIANNE_TTF, 13, 1);
        drawBtn(frameBtn(), typing ? frameBuf + "_" : "frame #", typing, mx, my);
        if (typing)
        {
            char tmp[80];
            if (rtDriven)
                snprintf(tmp, sizeof tmp, "type reanim frame 0..%d then Enter (Esc cancels)",
                         (int)period - 1);
            else
                snprintf(tmp, sizeof tmp, "type sheet frame 0..%d then Enter (Esc cancels)",
                         nf - 1);
            win.show_text_shadowed(tmp, CARD_X, frameBtn().y - 17, RGB(255, 255, 120),
                                   BRIANNE_TTF, 12, 1);
        }

        // hover tooltip plate (paused parts row first, then the grid subject)
        string tip;
        if (paused && showParts && !hoveredTrack.empty())
        {
            if (hoveredTrack.rfind("group:", 0) == 0)
                tip = "group " + hoveredTrack.substr(6) + ": click toggles every member";
            else
            {
                auto si = smap.find(hoveredTrack);
                if (si != smap.end())
                {
                    const reanimz::TrackStatus &ts = si->second;
                    tip = ts.name + ": ";
                    if (!ts.hasImage)
                        tip += "no image in timeline (marker track)";
                    else if (trackWillDraw(ts, ch.showAll, ch.userShow, ch.userHide))
                    {
                        tip += "DRAWN now";
                        if (ts.dataHidden)
                            tip += " (f=-1 in data, force-shown)";
                    }
                    else if (ch.userHide.count(ts.name))
                        tip += "hidden by you (click = undo)";
                    else if (ts.codeHidden)
                        tip += "hidden by state rules (click = force show)";
                    else if (ts.dataHidden)
                        tip += "f=-1 in data at this frame (click = force show)";
                    else
                        tip += "hidden";
                    if (ch.userShow.count(ts.name) || ch.userHide.count(ts.name))
                        tip += "  *overridden*";
                }
            }
        }
        if (tip.empty() && hovGrid >= 0)
            tip = chars[(zombieTab ? 0 : (int)COUNT_ZOMBIE_TYPE) + hovGrid].name +
                  "  --  click to view";
        if (!tip.empty())
        {
            int tw = 0, th = 0;
            TTF_SizeText(win.get_font(BRIANNE_TTF, 12), tip.c_str(), &tw, &th);
            int tx = mx + 14, ty = my + 8;
            if (tx + tw + 10 > WINDOW_WIDTH)
                tx = mx - tw - 14;
            fillPlate(tx, ty, tw + 10, th + 8, 210);
            win.show_text_shadowed(tip, tx + 5, ty + 5, RGB(140, 220, 255),
                                   BRIANNE_TTF, 12, 1);
        }

        drawIndexBtn(BTN_BACK_INDEX, mx, my);
        drawCloseBtn(BTN_CLOSE, mx, my);

        HANDLE_SDL_EVENT(
            QUIT();
            if (typing)
            {
                if (e.type == SDL_TEXTINPUT)
                    for (const char *p = e.text.text; *p; p++)
                        if (*p >= '0' && *p <= '9' && frameBuf.size() < 5)
                            frameBuf += *p;
                if (e.type == SDL_KEYDOWN)
                {
                    if (e.key.keysym.sym == SDLK_BACKSPACE)
                    {
                        if (!frameBuf.empty())
                            frameBuf.pop_back();
                    }
                    else if (e.key.keysym.sym == SDLK_RETURN || e.key.keysym.sym == SDLK_KP_ENTER)
                        commitJump();
                    else if (e.key.keysym.sym == SDLK_ESCAPE)
                    {
                        typing = false;
                        SDL_StopTextInput();
                    }
                }
            }
            else
            {
                LCLICK({
                    if (hitBtn(BTN_CLOSE, mouse_x, mouse_y))
                        leave = true;
                    else if (hitBtn(BTN_BACK_INDEX, mouse_x, mouse_y))
                        page = VIEWER_PAGE_INDEX;
                    else if (hitBtn(frameBtn(), mouse_x, mouse_y))
                    {
                        typing = true;
                        frameBuf.clear();
                        paused = true;
                        SDL_StartTextInput();
                    }
                    else if (hitBtn(navPrevBtn(zombieTab), mouse_x, mouse_y))
                    {
                        if (--selState < 0)
                            selState = (int)ch.states.size() - 1;
                        resetClock();
                    }
                    else if (hitBtn(navNextBtn(zombieTab), mouse_x, mouse_y))
                    {
                        if (++selState >= (int)ch.states.size())
                            selState = 0;
                        resetClock();
                    }
                    else if (hitBtn(partsModeBtn(zombieTab), mouse_x, mouse_y))
                        showParts = !showParts;
                    else if (sbVisible && mouse_x >= sbTrack.x - 2 &&
                             mouse_x < sbTrack.x + sbTrack.w + 2 &&
                             mouse_y >= sbTrack.y && mouse_y < sbTrack.y + sbTrack.h)
                        gScroll = anim_viewer_grid_scroll_row_at(zombieTab, mouse_y);
                    else if (hitGridCell(zombieTab, gScroll, mouse_x, mouse_y) >= 0)
                    {
                        pickTab[zombieTab ? 1 : 0] =
                            hitGridCell(zombieTab, gScroll, mouse_x, mouse_y);
                        selState = 0;
                        resetClock();
                    }
                    else if (showParts && listHover)
                    {
                        // PARTS list: ALL/RESET, scroll buttons, row toggles
                        if (hitBtn(partsAllBtn(zombieTab), mouse_x, mouse_y))
                            ch.showAll = !ch.showAll;
                        else if (hitBtn(partsResetBtn(zombieTab), mouse_x, mouse_y))
                        {
                            ch.userShow.clear();
                            ch.userHide.clear();
                            ch.showAll = false;
                        }
                        else if (hitBtn(partsUpBtn(zombieTab), mouse_x, mouse_y))
                        {
                            ch.scroll -= 2;
                            if (ch.scroll < 0)
                                ch.scroll = 0;
                        }
                        else if (hitBtn(partsDownBtn(zombieTab), mouse_x, mouse_y))
                        {
                            ch.scroll += 2;
                            if (ch.scroll > maxScroll)
                                ch.scroll = maxScroll;
                        }
                        else if (hasStats && mouse_y >= pageListY(zombieTab))
                        {
                            int row = (mouse_y - pageListY(zombieTab)) / LIST_LINE + ch.scroll;
                            if (row >= 0 && row < (int)ch.items.size())
                            {
                                const PanelItem &it = ch.items[row];
                                if (it.header)
                                {
                                    // group row: any member drawn -> hide all,
                                    // else force-show all
                                    bool anyShown = false;
                                    for (const string &m : groupTracks(ch, it.group))
                                    {
                                        auto si = smap.find(m);
                                        if (si != smap.end() &&
                                            trackWillDraw(si->second, ch.showAll,
                                                          ch.userShow, ch.userHide))
                                            anyShown = true;
                                    }
                                    for (const string &m : groupTracks(ch, it.group))
                                    {
                                        if (anyShown)
                                        {
                                            ch.userShow.erase(m);
                                            ch.userHide.insert(m);
                                        }
                                        else
                                        {
                                            ch.userHide.erase(m);
                                            ch.userShow.insert(m);
                                        }
                                    }
                                }
                                else
                                {
                                    auto si = smap.find(it.track);
                                    if (si != smap.end())
                                        toggleTrack(it.track, si->second, ch.userShow,
                                                    ch.userHide);
                                }
                            }
                        }
                    }
                    else if (!showParts && !zombieTab && optRowAt(false, mouse_x, mouse_y) >= 0)
                    {
                        int row = optRowAt(false, mouse_x, mouse_y);
                        if (row == 0)
                            plantOpts.shadow = !plantOpts.shadow;
                        else if (row == 1)
                            plantOpts.cellBox = !plantOpts.cellBox;
                        else if (row == 2)
                            plantOpts.hitbox = !plantOpts.hitbox;
                        else if (row == 3)
                            plantOpts.blink = !plantOpts.blink;
                    }
                    else if (!showParts && statesRowAt(ch, selState, zombieTab,
                                                       mouse_x, mouse_y) >= 0)
                    {
                        selState = statesRowAt(ch, selState, zombieTab, mouse_x, mouse_y);
                        resetClock();
                    }
                });
                if (e.type == SDL_MOUSEWHEEL && gridHover)
                    gScroll -= e.wheel.y; // clamped at the top of the loop
                if (e.type == SDL_MOUSEWHEEL && !gridHover && listHover)
                {
                    if (showParts)
                        ch.scroll -= e.wheel.y * 2; // clamped at the top of the loop
                    else
                    {
                        // one page-state per notch, wrap
                        if (e.wheel.y < 0 && ++selState >= (int)ch.states.size())
                            selState = 0;
                        if (e.wheel.y > 0 && --selState < 0)
                            selState = (int)ch.states.size() - 1;
                        resetClock();
                    }
                }
                KEY_PRESS(ESCAPE, page = VIEWER_PAGE_INDEX;);
                KEY_PRESS(F1, page = VIEWER_PAGE_INDEX;);
                KEY_PRESS(F12, dumpPending = 2;);
                KEY_PRESS(TAB, cyclePage(););
                KEY_PRESS(LEFT, navSubject(-1););
                KEY_PRESS(RIGHT, navSubject(1););
                KEY_PRESS(SPACE, paused = !paused;);
                KEY_PRESS(v, speedIdx = (speedIdx + 1) % 3;);
                KEY_PRESS(t, compare = !compare;);
                KEY_PRESS(DOWN, {
                    if (++selState >= (int)ch.states.size())
                        selState = 0;
                    resetClock();
                });
                KEY_PRESS(UP, {
                    if (--selState < 0)
                        selState = (int)ch.states.size() - 1;
                    resetClock();
                });
                KEY_PRESS(RIGHTBRACKET, {
                    if (++selState >= (int)ch.states.size())
                        selState = 0;
                    resetClock();
                });
                KEY_PRESS(LEFTBRACKET, {
                    if (--selState < 0)
                        selState = (int)ch.states.size() - 1;
                    resetClock();
                });
                KEY_PRESS(PAGEDOWN, {
                    if (partsHover)
                        ch.scroll += rows; // clamped at the top of the loop
                    else
                    {
                        if (++selState >= (int)ch.states.size())
                            selState = 0;
                        resetClock();
                    }
                });
                KEY_PRESS(PAGEUP, {
                    if (partsHover)
                        ch.scroll -= rows; // clamped at the top of the loop
                    else
                    {
                        if (--selState < 0)
                            selState = (int)ch.states.size() - 1;
                        resetClock();
                    }
                });
                KEY_PRESS(COMMA, stepFrame(-1););
                KEY_PRESS(PERIOD, stepFrame(1););
            });

        // advance clocks like the game does: 60 tps, reanim at 0.2 frame/tick
        if (!paused && !typing)
        {
            tick += speed;
            rt += 0.2f * speed;
            if (rtDriven && period > 0.0f && rt >= period)
                rt -= period;
        }

        if (dumpPending > 0 && --dumpPending == 0)
            anim_viewer_dump_frame("/tmp/anim_viewer.png");
        win.update_screen();
    }
    win.clear_renderer();
}
