#include "mower.hpp"
#include "draw/rsdl.hpp"
#include "draw/world_scale.hpp"
#include "elements/Level/Level.hpp"
#include "elements/Map/Map.hpp"
#include "game_param.hpp" // BOARD_OFFSET_X
#include "music/music.hpp"
#include "reanim/one_shot_fx.hpp"
#include "reanim/reanim.hpp"
#include "reanim/reanim_sdl.hpp"
#include <algorithm>
#include <cmath>

extern int game_state;
extern Window win;
extern Map cells;
extern Level level;

namespace
{
// 2Pak lawn-cleaner reanim wiring (selector-scene style one-off loader):
//   plain day/night lanes  -> LawnMower.reanim
//   roof maps (5-x, 5-10)  -> RoofCleaner.reanim
//   pool/fog water lanes   -> PoolCleaner.reanim
// The old Icons/mower.png sheet stays as the fallback when a doc or its part
// images are missing.
reanim::SdlReanimRenderer *g_mower_renderer = nullptr;
reanim::ReanimDoc g_lawn_doc, g_roof_doc, g_pool_doc;
bool g_lawn_ok = false, g_roof_ok = false, g_pool_ok = false;
bool g_mower_tried = false;

struct MowerSkin
{
    const char *anim_file;
    float fps;             // doc frames per second (advance = fps/100 per tick)
    int idle_frame;        // frozen pose while parked
    int run_first, run_last; // loop range while rolling (loopWrap bounds)
    float box_w, box_h;  // on-screen fit box (px) the rolling art fits inside
    float scale;
    float cx, baseY; // doc-space bbox center-x and bottom line (pre-scale)
};
MowerSkin g_skins[3]; // 0 = lawn, 1 = roof, 2 = pool

// Measure the loop-range bbox once (measureOnly pass) so the doc lands on its
// lane at the retail size: art bottom at mower_ground_y, horizontally centered
// on the mower's x anchor. Fit box per skin (build_artifacts/scale_audit.md
// S8.4): LawnMower keeps 92x66 (-5% vs retail 85x70); RoofCleaner 71x52 and
// PoolCleaner 69x56 are the retail median px sizes.

/*Vertical anchor of the lane surface under this mower, bottom-anchored --
exactly the rect the F2 layout editor calibrates against (mower_rect: top
y1 + 40, height MOWER_HEIGHT). Flat stages (day/night/pool) park on the flat
lane base cells[row][0].y1 + 97. Roof lanes park on the designer's hand-tuned
per-row ground (layout dump: parked art bottoms 274/355/449/544/615; the
bottom row's 615 keeps the retail ~15-px crop past the 600-px window) and
re-ground the art on every rendered frame so the cleaner rides the slope
(repvz LawnMower.cpp:17 recomputes mPosY = GetPosYBasedOnRow(mPosX + 40, row)
+ 23 per tick), but as a DELTA off that parked anchor instead of the raw
repvz term: with slope() = roof_slope_offset, ground = parked_ground +
slope(x + 40) - slope(parked_x + 40): delta 0 parked, ~20 px of climb per
sloped column. Render anchor only; never touches movement/collision.*/
int mower_parked_x(int row); // forward: calibration tables live below
bool calc_skin(reanim::ReanimDoc &doc, MowerSkin &sk)
{
    reanim::DrawOptions measure;
    measure.measureOnly = true;
    reanim::DrawStats acc;
    for (float fr = (float)sk.run_first; fr <= (float)sk.run_last; fr += 4.0f)
    {
        reanim::DrawStats st = g_mower_renderer->Draw(doc, fr, 0, 0, 1.0f, nullptr, measure);
        if (!st.hasBounds)
            continue;
        if (!acc.hasBounds)
            acc = st;
        else
        {
            acc.minX = std::min(acc.minX, st.minX);
            acc.minY = std::min(acc.minY, st.minY);
            acc.maxX = std::max(acc.maxX, st.maxX);
            acc.maxY = std::max(acc.maxY, st.maxY);
        }
    }
    if (!acc.hasBounds)
        return false;
    float w = acc.maxX - acc.minX, h = acc.maxY - acc.minY;
    if (w <= 0 || h <= 0)
        return false;
    sk.scale = std::min(sk.box_w / w, sk.box_h / h);
    sk.cx = (acc.minX + acc.maxX) * 0.5f;
    sk.baseY = acc.maxY;
    return true;
}

bool mower_reanim_ready()
{
    if (g_mower_tried)
        return g_lawn_ok || g_roof_ok || g_pool_ok;
    g_mower_tried = true;
    static reanim::SdlReanimRenderer renderer(win.sprite_renderer());
    g_mower_renderer = &renderer;
    if (!renderer.BindDirectory("./resources/reanim"))
        return false;
    // Idle = rest pose, active = rolling loop. PoolCleaner: frames 20..40 are
    // the anim_suck window (tube/funnel down, whitewater streaming).
    struct Def
    {
        const char *file;
        float fps;
        int idle;
        int f0, f1;
        float box_w, box_h; // retail fit box (px), scale audit S8.4
        MowerSkin *skin;
        reanim::ReanimDoc *doc;
        bool *ok;
    } defs[] = {
        {"LawnMower.reanim", 20.0f, 0, 0, 16, 92.0f, 66.0f, &g_skins[0], &g_lawn_doc, &g_lawn_ok},
        {"RoofCleaner.reanim", 12.0f, 0, 0, 8, 71.0f, 52.0f, &g_skins[1], &g_roof_doc, &g_roof_ok},
        {"PoolCleaner.reanim", 12.0f, 0, 20, 40, 69.0f, 56.0f, &g_skins[2], &g_pool_doc, &g_pool_ok},
    };
    for (auto &d : defs)
    {
        std::string err;
        if (!reanim::LoadReanimXml(std::string("./resources/reanim/") + d.file, *d.doc, err))
        {
            SDL_Log("mower reanim: %s", err.c_str());
            continue;
        }
        d.skin->fps = d.fps;
        d.skin->idle_frame = d.idle;
        d.skin->run_first = d.f0;
        d.skin->run_last = d.f1;
        d.skin->box_w = d.box_w;
        d.skin->box_h = d.box_h;
        *d.ok = calc_skin(*d.doc, *d.skin);
        if (!*d.ok)
            SDL_Log("mower reanim: %s has no visible parts in the roll range", d.file);
    }
    return g_lawn_ok || g_roof_ok || g_pool_ok;
}

// Which cleaner runs on this mower's lane.
int mower_skin_for(int row)
{
    if ((level.map_type == 6 || level.map_type == 7) && g_roof_ok)
        return 1; // roof / roof_night -> RoofCleaner
    if (level.row_has_water(row) && g_pool_ok)
        return 2; // water lane -> PoolCleaner
    if (g_lawn_ok)
        return 0;
    return -1;
}

/*Parked-lane anchor x: the original parks every cleaner at mPosX = -21
(LawnMower.cpp:193 rolls -160 -> -21; pool/roof use the same anchor), so the
clone's board shift puts it at BOARD_OFFSET_X - 21 = 199.
Vertical placement is NOT here: the art bottom-anchors on mower_ground_y,
which rides the roof slope delta once the cleaner rolls.*/
int mower_parked_x(int row)
{
    (void)row;
    return BOARD_OFFSET_X - 21;
}

} // namespace

/*Shared mower lane-surface anchor (declared in mower.hpp; the layout editor
reads it too). Roof lanes park on the hand-tuned per-row ground and ride the
slope as a delta; flat lanes use y1 + 97. Render anchor only.*/
int mower_ground_y(int x, int row)
{
    if (level.is_roof())
    {
        static const int ROOF_PARKED_GROUND[] = {274, 355, 449, 544, 615};
        int base = ROOF_PARKED_GROUND[row < 0 ? 0 : (row > 4 ? 4 : row)];
        return base + roof_slope_offset(x + 40) - roof_slope_offset(mower_parked_x(row) + 40);
    }
    return cells[row][0].y1 + MOWER_HEIGHT + 40;
}


/*Reanim display path; returns false when no doc covers this lane (falls back
to the mower sheet).*/
bool Mower::display_reanim()
{
    if (!mower_reanim_ready())
        return false;
    int skin = mower_skin_for(row);
    if (skin < 0)
        return false;
    const MowerSkin &sk = g_skins[skin];
    reanim::ReanimDoc &doc = skin == 1 ? g_roof_doc : (skin == 2 ? g_pool_doc : g_lawn_doc);

    // F2 layout-editor debug scale (rsdl registry): 1.0 in normal gameplay.
    const float dbg = rsdl_debug_scale(this);
    const float draw_scale = sk.scale * dbg * WORLD_SCALE;

    float local, wf = -1, wl = -1;
    if (status == MOWER_ACTIVE)
    {
        float len = (float)(sk.run_last - sk.run_first + 1);
        local = fmodf(rframe, len);
        wf = (float)sk.run_first;
        wl = (float)sk.run_last;
    }
    else
    {
        local = (float)sk.idle_frame;
    }
    float absFrame = (status == MOWER_ACTIVE ? (float)sk.run_first : 0.0f) + local;

    // Art sits on the lane surface under it (mower_ground_y: flat on
    // day/pool lanes, sliding down the roof with its own x on roof maps).
    // With a debug scale the art grows about its bottom-center contact point.
    float oy = (float)mower_ground_y(x + layout_dx, row) + layout_dy - draw_scale * sk.baseY;
    float ox = (float)(x + layout_dx + MOWER_WIDTH / 2) - draw_scale * sk.cx;
    reanim::DrawOptions opt;
    if (wf >= 0)
    {
        opt.loopWrapFirst = (int)wf;
        opt.loopWrapLast = (int)wl;
    }
    g_mower_renderer->Draw(doc, absFrame, ox, oy, draw_scale, nullptr, opt);

    // anim advances per logic tick, not per render (auto-FPS)
    for (int _lt = logic_ticked; _lt > 0 && status == MOWER_ACTIVE && check_status(game_state, IS_PAUSED) == false; _lt--)
        rframe += sk.fps / 100.0f;
    return true;
}

/*
Mower constructor
*/
Mower::Mower()
{
}

/*
Mower constructor with details
*/
Mower::Mower(int _x, int _row, int _frame, Mower_status _status)
{
    x = _x;
    row = _row;
    frame = _frame;
    status = _status;
}

void Mower::display()
{
    // dust emitter cadence now counts logic-tick frames (auto-FPS), keeping its per-second rate
    for (int _lt = logic_ticked; _lt > 0 && status == MOWER_ACTIVE && !check_status(game_state, IS_PAUSED); _lt--)
        reanimfx::mower_dust((int)x - 8, mower_ground_y((int)x - 8, row), row); // dust puff kicked up behind the rolling blade
    if (display_reanim())
        return;
    // current frame (bottom-left anchor on the lane surface; the mower sheet
    // is padded to its frame bottom, so the frame edge is the ground line)
    int sframe = frame / MOWER_FRAME;
    // F2 layout-editor debug scale (rsdl registry): 1.0 in normal gameplay.
    // The sheet grows about its bottom-center contact point; the transient
    // layout offsets (0 in gameplay) shift the anchor.
    const float dbg = rsdl_debug_scale(this);
    const int dw = (int)lroundf(MOWER_WIDTH * dbg * WORLD_SCALE);
    const int dh = (int)lroundf(MOWER_HEIGHT * dbg * WORLD_SCALE);
    win.draw_nth_frame_bl(MOWER_DIRECTORY, sframe, MOWER_WIDTH, MOWER_HEIGHT,
                          (int)lroundf(x + layout_dx + MOWER_WIDTH / 2.0f - dw / 2.0f),
                          mower_ground_y(x + layout_dx, row) + layout_dy,
                          dw, dh);
    // move to next frame (anim advances per logic tick, not per render -- auto-FPS)
    for (int _lt = logic_ticked; _lt > 0 && status == MOWER_ACTIVE && check_status(game_state, IS_PAUSED) == false; _lt--)
    {
        if (++frame >= MOWER_FRAME * all_img[MOWER_DIRECTORY].n_sheet)
        {
            frame = 0;
        }
    }
}

bool Mower::active()
{
    if (status == MOWER_INACTIVE)
    {
        status = MOWER_ACTIVE;
        play_sound_effect(level.row_has_water(row) ? POOL_CLEANER_MUSIC_DIRECTORY
                                                   : MOWER_MUSIC_DIRECTORY);
        return true;
    }
    return false;
}

/*
Init the mower from l_row to r_row
*/
void init_mower(vector<Mower *> &mowers, int l_row, int r_row)
{
    mowers.assign(cells.size(), nullptr);
    for (int row = l_row; row <= r_row; row++)
    {
        mowers[row] = new Mower(mower_parked_x(row), row, 0, MOWER_INACTIVE);
    }
}

/*
Display the mowers (both inactive and active)
*/
void display_mowers(const vector<Mower *> &mowers)
{
    for (auto &mower : mowers)
        if (mower != nullptr)
            mower->display();
}

/*
Return true if can active a mower in given row
*/
bool active_mower(vector<Mower *> &mowers, int row)
{
    if (mowers[row] != nullptr)
        return mowers[row]->active();
    return false;
}