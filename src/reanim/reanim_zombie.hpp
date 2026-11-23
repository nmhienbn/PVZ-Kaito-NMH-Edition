#pragma once

// Bridge between the game's zombie state machine (ImageType directory_num +
// armor_dir + degrade chain) and the reanim renderer: maps every zombie
// sprite-sheet directory to a reanim segment + track-visibility set + optional
// armor image override, then draws with the RETAIL placement formula (repvz
// Zombie::UpdateReanim + GetDrawPos, Zombie.cpp:5307-5362/5899-6162): the doc
// origin lands at
//   ox = x + 45 + offX - 30*s,  oy = y + 132 + offY - 120*s   (s = skin scale
// x the editor/big-trouble mul), so doc point (30, 120) is the scale-invariant
// pivot the art grows about and the per-type retail image offsets (offX/offY)
// shift the whole pose. See reanim_zombie.cpp's placement block for the
// line-by-line derivation.

#include <SDL.h>
#include <string>
#include <unordered_set>
#include <vector>

class ISpriteRenderer;

namespace reanim { struct DrawStats; } // reanim_sdl.hpp

namespace reanimz {

// Sentinel img_num for ZombiePart: play the reanim death animation instead of
// the old sprite sheets.
const int DEATH_PART_IMG = -100;

// Sentinel img_num for ZombiePart: play the charred (explosion/burn) corpse.
// The ZombiePart `skin` slot then carries the CHARRED_* kind below, not a
// skin. repvz Zombie::ApplyBurn (Zombie.cpp:8756-8905): the charred corpse
// is a SEPARATE play-once-and-disappear reanimation anim (standing blink ->
// crumble to ash pile), NOT a track of the zombie's own skin.
const int CHARRED_PART_IMG = -106;

// Charred corpse kinds: one doc per zombie group (repvz Zombie.cpp:8824-8862):
// generic Zombie_charred for every common type; dedicated art for imp,
// digger (with or without the pickaxe), zamboni, catapult and gargantuar.
const int CHARRED_GENERIC = 0;
const int CHARRED_IMP = 1;
const int CHARRED_DIGGER = 2;
const int CHARRED_ZAMBONI = 3;
const int CHARRED_CATAPULT = 4;
const int CHARRED_GARGANTUAR = 5;
const int CHARRED_DIGGER_NOAXE = 6; // digger variant: PHASE_DIGGER_WALKING_WITHOUT_AXE
// zombie type (game_param.hpp ZombieType) -> charred kind
int charredKindForType(int zombie_type);

// Sentinels for the tossed-part effects (arm torn off, armor popping off):
// rendered as physics particles like the original engine's emitters.
const int DROP_ARM_IMG = -101;    // outer arm, armless zombie
const int DROP_CONE_IMG = -102;
const int DROP_BUCKET_IMG = -103;
const int DROP_DOOR_IMG = -104;
const int DROP_FLAG_IMG = -105;
// D1 head pop: the severed head tossed when the body pool falls under 1/3
// (repvz Zombie::DropHead, Zombie.cpp:3511-3540). Uses the same pop physics
// as the death-anim head, but as a standalone part with its own lifetime.
const int DROP_HEAD_IMG = -107;

// Reanim skins for the extra character docs (0 = the plain zombie doc).
const int SKIN_PLAIN = 0;
const int SKIN_POLEVAULT = 1;
const int SKIN_NEWSPAPER = 2;
const int SKIN_FOOTBALL = 3;
const int SKIN_DANCER = 4; // GOTY disco art
// Area 3-5 zombies (pool/fog/roof). Behavior is STUB-AREA-AGENT for now;
// the skins below load the real 2Pak reanim art so the stubs render
// correctly. (Ducky-tube stays on the plain doc: it only unhides the tube.)
const int SKIN_SNORKEL = 5;
// Pseudo-skin for debug tooling: the flagpole attachment doc
// (Zombie_flagpole.reanim), normally driven by drawZombie's hand attach.
const int SKIN_FLAGPOLE = -2;
const int SKIN_ZOMBONI = 6;
const int SKIN_BOBSLED = 7;
const int SKIN_DOLPHINRIDER = 8;
const int SKIN_JACKBOX = 9;
const int SKIN_BALLOON = 10;
const int SKIN_DIGGER = 11;
const int SKIN_POGO = 12;
const int SKIN_BUNGEE = 13;
const int SKIN_LADDER = 14;
const int SKIN_CATAPULT = 15;
const int SKIN_GARGANTUAR = 16;
const int SKIN_IMP = 17;
const int SKIN_ZOMBOSS = 18;
// Zomboss fight props (roof agent): boss boulders rolling down a lane.
const int SKIN_BOSS_FIREBALL = 19;
const int SKIN_BOSS_ICEBALL = 20;
// Backup dancer (GOTY new art, light-blue outfit): its own doc
// (Zombie_backup.reanim, retail pvz1 fla export). The doc reuses the disco
// track names but carries backup images; the head still comes from the
// dancer set (IMAGE_REANIM_ZOMBIE_DANCER__HEAD in the data).
const int SKIN_BACKUP = 21;
// Zombie Yeti (rare animated cameo: walks in, then turns and runs away).
// Doc already shipped in resources/reanim (retail pvz1, byte-identical to
// the assets_src fla export).
const int SKIN_YETI = 22;
const int COUNT_ZOMBIE_SKIN = 23; // skins 1..22 have their own reanim doc
// armor_dir encodings for the extra skins (set by the zombie classes):
const int POLE_ARMLESS_STATE = 1;    // pole vaulter lost its outer arm
const int PAPER_DAMAGED2_STATE = 2;  // newspaper paper, first damage stage
const int PAPER_DAMAGED3_STATE = 3;  // newspaper paper, second damage stage
const int PAPER_ARMLESS_STATE = 4;   // newspaper zombie lost its arm
const int FOOT_ARMLESS_STATE = 5;    // football zombie lost its left arm
const int DANCER_ARMLESS_STATE = 6;  // dancer/backup lost its outer arm
// Dolphin rider locomotion phases (set by DolphinRiderZombie; armor_dir
// -1 = wading after the dolphin is gone):
const int DOLPHIN_RIDING_STATE = 7;     // mounted, fast ("anim_ride")
const int DOLPHIN_JUMPING_STATE = 8;    // mid-jump over a plant ("anim_dolphinjump")
const int DOLPHIN_INTO_POOL_STATE = 9;  // leaping into the pool ("anim_jumpinpool")
const int DOLPHIN_CARRY_STATE = 10;     // shore walk, dolphin carried ("anim_walkdolphin")
// Snorkel zombie phase (set by SnorkelZombie; armor_dir -1 = walking/swimming):
const int SNORKEL_INTO_POOL_STATE = 11; // leaping into the pool ("anim_jumpinpool")

// Big Trouble Little Zombie (3-5): re-anchored half-size zombie art.
// 1.0 = normal rendering; the game hook sets 0.5 for big_trouble levels.
void setZombieScaleMul(float k);

// Big Trouble Little Zombie (3-5): animation playback speed factor.
// 1.0 = normal reanim pace; the game hook feeds it level.zombie_time_mul
// (JSON "zombie_time_mul", default 4.0 for big_trouble) so the mini zombies'
// walk/eat animations run comically fast (their steps are faster too). The
// game-side reanim_time counter is unchanged; the factor is applied when the
// drawn frame is computed.
void setZombieTimeMul(float k);
float zombieTimeMul();

// Local frame inside the directory's segment that drawZombie would render for
// a given reanim_time, after the big-trouble timescale, the retail
// playback-rate rules and the chilled half-rate (chilled=false matches the
// un-chilled draw; repvz ApplyAnimRate x0.5, Zombie.cpp:6676-6682).
// 0 when the reanim assets are not loaded or the directory is unmappable.
float effectiveLocalFrame(int directory_num, float reanim_time, bool chilled = false);

// On-screen head anchor for the kernel-pult butter pat (repvz
// Zombie::DrawButter, Zombie.cpp:6274-6321): center-X and top of the zombie's
// anim_head1 image at its currently drawn frame, through the body's own draw
// geometry (per-skin scale x the big-trouble/editor scale mul); artMul
// reports that combined art scale (a plain zombie = 1.0), for sizing the
// pat. False when the reanim path isn't driving this state
// (caller keeps the legacy fixed offset).
bool headAnchor(int directory_num, int armor_dir, float reanim_time,
                bool cold, int x_location, int y_location,
                float& headCX, float& headTopY, float& artMul);

// Lazily initialized on first draw (needs the game's sprite renderer).
bool ready();
bool init(ISpriteRenderer& renderer);

// Frames in the segment mapped to `directory_num` (0 if unmappable). This is
// the WRAP WINDOW of the game-side reanim_time counter: for states with a
// retail playback-rate rule (kPlayRateRules in reanim_zombie.cpp) it is the
// trimmed segment length divided by the rate multiplier, so the drawn loop
// still wraps seamlessly. chilled=true passes the chilled half-rate, doubling
// the window (the frozen zombie's loop takes twice as many ticks to wrap).
float segmentLength(int directory_num, bool chilled = false);

// Distance-driven walk feed (retail Zombie::UpdateAnimSpeed, Zombie.cpp:6683-
// 6693: the foot-walk states lock the anim rate to the ground speed off the
// doc's "_ground" track): reanim_time FEED UNITS PER PIXEL of board travel for
// the directory's current walk segment, so feeding `reanim_time += walk_v *
// this` keeps the drawn feet planted at the zombie's real speed (chilled
// 0.4-vs-0.5 and big-trouble scale/time pairs compensated). Negative when the
// state is not a distance-driven walk -- eat segments, one-shots, snorkel
// swim, pogo, bobsled, catapult, dolphin-rider states -- where retail keeps
// its fixed rate: the caller then keeps its pinned per-tick feed.
float walkFeedPerPx(int directory_num, int armor_dir, bool cold);

// D2 movement source (retail Zombie::UpdateZombieWalking, Zombie.cpp:4126-
// 4133): advance `reanim_time` by the distance-locked walk feed for
// `speed_px` board px this tick (the same feed walkFeedPerPx derives) and
// return the board-px `_ground` track delta that advance produces, so the
// logic can move BY the animation. Returns false (and leaves reanim_time
// untouched) for states with no usable _ground walk track -- the caller then
// keeps moving by raw velX. The delta already includes the doc's skin scale
// and the big-trouble/editor render multiplier, and is positive toward the
// zombie's forward (left) direction.
bool stepWalkReanim(int directory_num, int armor_dir, float &reanim_time,
                    float speed_px, bool cold, float &ground_dx);

// Retail playback-rate multiplier of a directory state (kPlayRateRules in
// reanim_zombie.cpp): 1.0 for one-shots / unlisted segments and before init.
// Debug tooling.
float playbackRateForDir(int directory_num, int armor_dir = -1);

// Draw one zombie. Returns false (and draws nothing) when reanim assets are
// unavailable -- the caller then falls back to the old sprite-sheet code.
//   x_location/y_location: game zombie position (top-left of 166x144 box);
//     the retail pivot formula places doc point (30, 120) at
//     (x + 45 + offX, y + 132 + offY), scale-invariant under the art scale.
//   reanim_time: local time in reanim frames inside the current segment.
//   extraHide: extra track names to hide on top of the state-derived set
//     (debug tooling; any case). extraShow: track names to force-show instead
//     (removes them from the hide set AND adds them to DrawOptions::forceShow,
//     revealing f=-1 tracks too; debug tooling). partsDrawn: out count of
//     drawn parts. stats, when given, receives the accumulated DrawStats
//     (body + attachments): bounds of everything drawn this call.
//   attack_fade: 0..1 strength of the damage flash; the game hook feeds it
//     attacked_time/MAX_TIME_BLINK so the flash fades out over the blink
//     budget, like the original's mJustGotShotCounter grayness ramp
//     (Zombie.cpp:5744-5750). 1.0 = full strength (debug tooling default).
bool drawZombie(int directory_num, int armor_dir, float reanim_time,
                int x_location, int y_location, bool cold, bool attacked,
                const std::unordered_set<std::string> *extraHide = nullptr,
                const std::unordered_set<std::string> *extraShow = nullptr,
                int *partsDrawn = nullptr, reanim::DrawStats *stats = nullptr,
                float attack_fade = 1.0f);

// Track names of the main zombie reanim doc (lowercased), for debug tooling.
std::vector<std::string> trackNames();

// Per-track visibility snapshot drawZombie would produce at
// (directory_num, armor_dir, reanim_time), for debug tooling: one entry per
// track of the main zombie doc, plus the flagpole doc when loaded.
struct TrackStatus
{
    std::string name; // lowercase
    bool hasImage;    // track references an image somewhere in the timeline
    bool codeHidden;  // in drawZombie's state-derived hide set
    bool dataHidden;  // frame flag f == -1 at the sampled frame
    bool forceShown;  // revealed via drawZombie's forceShow set
};
std::vector<TrackStatus> trackStatuses(int directory_num, int armor_dir,
                                       float reanim_time);

// Draw one reanim death corpse; `aging_time` runs in reanim frames (0.2 per
// game tick on the game hook feed). The fall plays at the original's
// PlayDeathAnim rates (24-30 fps, seeded per corpse; football/snorkel/garg/
// digger/yeti fixed bands -- Zombie.cpp:9078-9108), then the corpse holds
// 0.9s and fades over 0.1s. in_pool: died in the water -- plays
// anim_waterdeath when the doc has it and lingers only a flash
// (repvz mZombieFade = 10 vs 100).
void drawDeathPart(float aging_time, int x_location, int y_location, bool cold,
                   int skin = SKIN_PLAIN, bool in_pool = false,
                   bool headless = false);

// Death-anim playback rate in reanim frames/sec for `skin` (repvz
// Zombie.cpp:9078-9108): 24-30 fps seeded per corpse; football 24,
// snorkel/yeti/gargantuar 14, digger 18. Debug tooling / headless checks.
float deathRateFps(int skin, unsigned seed);

// Ticks (game frames) a death corpse of `skin` stays on screen: the fall at
// the seeded death rate plus the hold+fade tail. `seed` must match the draw
// side (derived from the part's x/y). in_pool shortens the tail.
int deathLingerTicks(int skin, bool in_pool, unsigned seed);

// Draw a tossed-part particle (DROP_*_IMG); `aging_time` runs in reanim frames.
void drawDropPart(int drop_img, float aging_time, int x_location, int y_location,
                  bool cold, int skin = SKIN_PLAIN);

// Ticks (game frames) a tossed part stays on screen before being erased.
int dropPartLinger(int drop_img);

// Charred corpse: draw one frame of the charred reanim doc of `kind`
// (CHARRED_*), aging in GAME TICKS (60 tps; converted to the doc's authored
// fps inside). The timeline plays once and the part vanishes at its end
// (charredLingerTicks) like the original's REANIM_PLAY_ONCE reanimation.
void drawCharredPart(int kind, int ticks, int x_location, int y_location);
// Game ticks the charred corpse of `kind` stays on screen (whole timeline).
int charredLingerTicks(int kind);

// Pixels the vault animation moves the pole vaulter forward (baked anim
// translation x skin calibration); ~2 tiles.
float poleVaultTravelPx();

// Roof/boss zombies drive several reanim segments from their own state
// machines (bungee drop/hold/grab, gargantuar smash/throw, boss phases,
// boulders), outside the directory_num state table.

// Frames in `segment` of skin `skin`'s doc (0 when the skin/segment is
// missing -- callers then skip the visual state).
float skinSegmentLength(int skin, const char *segment);

// Wrap cross-blend length (reanim frames) for one looping segment window.
// 1.0 everywhere by default; >1 for the 2Pak takes listed in kWrapBlendRules
// whose authored ends do not close (balloon/dolphin/ladder walk, pogo/dolphin
// /imp eat) -- see tools/find_walk_loop.py measurements.
float loopBlendFrames(int skin, int first, int last);

// Draw one segment of an extra skin through the RETAIL pivot anchor (same
// formula as drawZombie: ox = x + 45 + offX - 30*s, oy = y + 132 + offY -
// 120*s, s = skin scale x scale_mul), with caller draw deltas and a scale
// multiplier (Zomboss stands 2x). Returns false when the skin/segment is
// unavailable. One-shot segments are clamped at their last frame instead of
// wrapping when loop == false. ddx/ddy: bungee passes its hang (plays retail
// mBodyY), the Zomboss body passes bossDocCompensation (below).
// skin may also be SKIN_PLAIN (base doc, kBaseHidden accessories hidden the
// way the game shows them) or SKIN_FLAGPOLE (the flag-pole doc, drawn at its
// authored rest pose -- no hand attach).
// extraHide/extraShow/partsDrawn/stats: debug tooling (anim viewer), same
// semantics as drawZombie; extraShow wins over every hide and reveals f=-1
// tracks. stats, when given, receives the renderer's DrawStats (drawn-part
// count + screen bounds).
bool drawSkinSegment(int skin, const char *segment, float reanim_time,
                     int x_location, int y_location, bool cold, bool attacked,
                     int ddx = 0, int ddy = 0, float scale_mul = 1.0f,
                     bool loop = true,
                     const std::unordered_set<std::string> *extraHide = nullptr,
                     const std::unordered_set<std::string> *extraShow = nullptr,
                     int *partsDrawn = nullptr,
                     reanim::DrawStats *stats = nullptr);

// --- placement probes (almanac / anim viewer) --------------------------------
// Measured geometry of a draw pose in the retail pivot frame (doc point
// (30, 120) = the scale-invariant anchor of the draw formula above):
//   pivotX/pivotY: where that doc point lands on screen for a draw at
//     (x_location, y_location) with scale_mul 1 and no deltas -- (45+offX,
//     132+offY), the point the art grows about.
//   C/B: pose bbox center-x / bottom in PIVOT space (doc - (30, 120)),
//     pre-multiplied by the skin scale: a caller wanting the bbox centered
//     on (ax, ay) at scale mul m passes
//       x_location = ax - pivotX - m*C,  y_location = ay - pivotY - m*B
//   W/H: bbox size at mul 1 (skin scale folded in).
// The bbox is the renderer's OPAQUE union over the pose's play window (the
// part PNGs are cropped to their art, and measureOnly bounds follow the
// opaque pixels, so the probe reads the box the eye sees). Results are
// cached per (pose, visibility) after the first call; ok=false when the
// reanim assets are not loaded or the pose is unmappable.
struct PoseGeom
{
    float pivotX = 0.0f, pivotY = 0.0f;
    float C = 0.0f, B = 0.0f;
    float W = 0.0f, H = 0.0f;
    bool ok = false;
};
// drawZombie's pose for a (directory_num, armor_dir) state -- the plain doc,
// the extra-skin docs and the flag-bearer's flag doc union (the pole rides
// the flagHandDelta attach, like the real draw).
bool zombiePoseGeom(int directory_num, int armor_dir, PoseGeom &out);
// drawSkinSegment's pose for a skin segment; extraHide mirrors its parameter
// (e.g. the almanac Zomboss body-out set). SKIN_FLAGPOLE measures the rest
// pose of the flagpole doc.
bool skinPoseGeom(int skin, const char *segment, PoseGeom &out,
                  const std::unordered_set<std::string> *extraHide = nullptr);
// Draw pivot of a skin: (45+offX, 132+offY) from the retail formula (the
// doc point (30, 120) anchor at mul 1, no deltas).
void skinDrawPivot(int skin, float &pivotX, float &pivotY);

// Zomboss doc compensation: drawSkinSegment's retail pivot anchor centers
// the doc's empty space (the boss art sits ~672/325 doc units off the box
// origin); the measured idle anchor (pre-rework calibration) is restored by
// passing these ints as drawSkinSegment's ddx/ddy. bossdriver::draw adds
// the same pair to its own origin so the dome driver stays glued to the
// body. Exact: ddx = -642.75*m - 45, ddy = -205.47*m - 132.
void bossDocCompensation(float scale_mul, int &ddx, int &ddy);

// Retail mScaleZombie of a zombie TYPE (0.8 for the GOTY dancers, 1.0 for
// everyone else; Big Trouble's 0.5 is NOT folded in -- it rides the editor
// scale mul). Debug/shadow tooling.
float skinScaleForType(int zombie_type);
// Reanim skin (SKIN_*) the draw path uses for a zombie TYPE (D1 drop-part FX
// choose the same doc as the body).
int zombieSkinForType(int zombie_type);
// Retail per-type image offsets (GetDrawPos netted with the balloon/pogo
// spawn raises and bungee's -18): the same values the draw path's
// skinImageOffset table carries, keyed by ZombieType. Shadow tooling.
void zombieImageOffsetForType(int zombie_type, float &dx, float &dy);

// --- debug tooling (anim viewer): doc introspection -------------------------
struct SegmentInfo
{
    std::string name; // "anim_*" label
    int first, last;  // absolute frame range in the doc
};
// Animation labels of a skin's doc, in timeline order (SKIN_PLAIN = base
// doc, SKIN_FLAGPOLE = flagpole doc). Empty when not loaded.
std::vector<SegmentInfo> skinSegments(int skin);

// Track names (lowercase) of one doc only: SKIN_PLAIN = base doc without the
// flagpole, SKIN_FLAGPOLE = the flagpole doc, 1..20 = that skin's doc.
std::vector<std::string> skinTrackNames(int skin);

// Per-track visibility snapshot for the raw segment playback of
// drawSkinSegment: codeHidden reflects the SKIN_PLAIN kBaseHidden rule (and
// SKIN_FLAGPOLE's no-hide rule), dataHidden the sampled f=-1 frame flag.
std::vector<TrackStatus> segmentTrackStatuses(int skin, const char *segment,
                                              float reanim_time);

} // namespace reanimz
