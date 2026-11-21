// Game hook for the PvZ2 sod-roll lawn intro. Kept separate from pam.cpp /
// pam_sdl.cpp so those stay linkable without the whole game (used by
// demo/pam_demo), mirroring reanim_game_hooks.cpp.
//
// The intro is a progressive reveal: the previous stage's lawn stays as the
// base and the current level's lawn is clipped in, left to right, behind the
// PAM unroll animation. The reveal edge is the PAM roll head, mapped onto
// the strip the original unrolls (sod_strip_geom: repvz Board.cpp:6111+), so
// the revealed sod line sits exactly under the roll art.

#include "pam_game_hooks.hpp"

#include "pam.hpp"
#include "pam_sdl.hpp"

#include "draw/rsdl.hpp"
#include "elements/Level/Level.hpp"
#include "elements/Map/Map.hpp"
#include "events/in_game/credit/credit.hpp"
#include "music/music.hpp"
#include "reanim/one_shot_fx.hpp"

extern int game_state;
extern Level level;
extern Map cells;
extern Window win;

namespace pamsod {

namespace {

pam::PamDoc gDoc;
pam::SdlPamRenderer* gRenderer = nullptr;
bool gReady = false;
bool gInitTried = false;

pam::Segment gSegment;
// Leading edge of the roll art per animation frame, in raw PAM units; the
// union bbox of the whole sweep. Measured once; same data every level.
std::vector<float> gEdgeRaw;
pam::PamDrawStats gUnion;

// Screen-space fit for the current level's strip (invalid when gFitLevel !=
// level.level_num): the roll head travels the strip x0..x0+w (repvz
// Board::DrawBackdrop), not the whole window.
int gFitLevel = -1;
float gScale = 1.0f;
float gOx = 0.0f, gOy = 0.0f;
std::vector<float> gEdgeX;

const char* PAM_DIR = "./resources/pvz2anim";

bool fit_to_strip() {
    if (gFitLevel == level.level_num) return true;
    if (gEdgeRaw.empty()) return false;
    float head0 = gEdgeRaw.front(), head1 = head0;
    for (float e : gEdgeRaw) head1 = std::max(head1, e);
    if (head1 <= head0) return false;
    SodStrip strip = sod_strip_geom();
    gScale = strip.w / (head1 - head0);
    gOx = strip.x0 - gScale * head0;
    float lawnTop = (float)cells[0][0].y1;
    float lawnBottom = (float)cells[cells.size() - 1][0].y2;
    gOy = (lawnTop + lawnBottom) * 0.5f - gScale * (gUnion.minY + gUnion.maxY) * 0.5f;
    // Leading edge in screen x per frame, clamped and made monotone so the
    // reveal never shrinks while the roll advances; it lands exactly on the
    // strip's end column.
    gEdgeX.assign(gEdgeRaw.size(), strip.x0);
    float run = (float)strip.x0;
    for (size_t i = 0; i < gEdgeRaw.size(); i++) {
        run = std::max(run, gOx + gScale * gEdgeRaw[i]);
        gEdgeX[i] = std::min(run, (float)(strip.x0 + strip.w));
    }
    SDL_Log("pamsod: fit strip x=%d..%d, scale %.3f, origin (%.1f, %.1f)",
            strip.x0, strip.x0 + strip.w, gScale, gOx, gOy);
    gFitLevel = level.level_num;
    return true;
}

float edge_at_frame(int frame) {
    int fi = frame - gSegment.first;
    if (fi < 0 || fi >= (int)gEdgeX.size()) return (float)WINDOW_WIDTH;
    return gEdgeX[fi];
}

bool init_assets() {
    if (gInitTried) return gReady;
    gInitTried = true;
    // Escape hatch: PVZ_CLASSIC_SOD=1 forces the classic roll (debugging).
    if (getenv("PVZ_CLASSIC_SOD")) return gReady;
    std::string err;
    if (!pam::LoadPamJson(std::string(PAM_DIR) + "/pam/SODROLL.pam.json", gDoc, err)) {
        SDL_Log("pamsod: %s (falling back to classic sod roll)", err.c_str());
        return gReady;
    }
    static pam::SdlPamRenderer sRenderer(win.sprite_renderer());
    gRenderer = &sRenderer;
    if (!gRenderer->LoadAtlasMap(std::string(PAM_DIR) + "/atlas_map/SodRollGroup_1536.json",
                                 std::string(PAM_DIR) + "/atlases", err)) {
        SDL_Log("pamsod: %s (falling back to classic sod roll)", err.c_str());
        return gReady;
    }
    if (!pam::PamPlayer(gDoc).SegmentRange("animation", gSegment)) {
        SDL_Log("pamsod: no 'animation' segment (falling back to classic sod roll)");
        return gReady;
    }

    // Measure pass (no raster): union bbox of the sweep plus the roll head's
    // leading edge per frame, all in raw PAM units. fit_to_strip() maps that
    // travel onto the current level's sod strip.
    pam::PamPlayer player(gDoc);
    pam::PamDrawOptions measure;
    measure.measureOnly = true;
    std::vector<pam::DrawPart> parts;
    bool have = false;
    for (int k = gSegment.first; k <= gSegment.last; k++) {
        parts.clear();
        player.EvalSprite(-1, k, pam::Mat{}, pam::ColorMul{}, parts);
        pam::PamDrawStats st = gRenderer->Draw(gDoc, parts, 0, 0, 1.0f, measure);
        if (!st.hasBounds) continue;
        if (!have) {
            gUnion = st;
            have = true;
        } else {
            gUnion.minX = std::min(gUnion.minX, st.minX);
            gUnion.minY = std::min(gUnion.minY, st.minY);
            gUnion.maxX = std::max(gUnion.maxX, st.maxX);
            gUnion.maxY = std::max(gUnion.maxY, st.maxY);
        }
        gEdgeRaw.push_back(st.maxX);
    }
    if (!have) return gReady;
    SDL_Log("pamsod: ready; sweep %.0fx%.0f, head %.0f..%.0f",
            gUnion.maxX - gUnion.minX, gUnion.maxY - gUnion.minY,
            gEdgeRaw.front(), gEdgeRaw.back());
    gReady = true;
    return gReady;
}

} // namespace

bool sod_intro_ready() { return init_assets(); }

bool display_sod_roll_intro_frame(float prog) {
    if (!init_assets() || !fit_to_strip()) return false;
    int len = gSegment.last - gSegment.first + 1;
    int f = (int)(prog * (len - 1) + 0.5f);
    if (f < 0) f = 0;
    if (f >= len) f = len - 1;

    if (prog >= 1.0f) {
        // Final state: the plain new lawn, so gameplay inherits it seam-free.
        win.draw_bg(level.background_directory, 0, 0);
        return true;
    }
    // Base: the pre-expansion lawn (the art the roll replaces).
    win.draw_bg(sod_prev_background(), 0, 0);
    // Progressive reveal of the new lawn behind the roll's leading edge.
    int edge = (int)(edge_at_frame(f) + 0.5f);
    if (edge > 0)
        win.draw_png(level.background_directory,
                     0, 0, edge, WINDOW_HEIGHT,
                     0, 0, edge, WINDOW_HEIGHT);
    // Roll art over the seam.
    pam::PamPlayer player(gDoc);
    std::vector<pam::DrawPart> parts;
    player.EvalSegment(gSegment, (float)(f - gSegment.first), parts);
    gRenderer->Draw(gDoc, parts, gOx, gOy, gScale);
    reanimfx::sod_dust(edge); // dust puff period at the roll's leading edge + scene FX pump
    return true;
}

bool display_sod_roll_intro() {
    if (!sod_intro_ready()) return false;
    int len = gSegment.last - gSegment.first + 1;
    const int ticksPerFrame = 2; // anim runs at 24 fps, game at ~60 tps
    int total = len * ticksPerFrame;
    for (int t = 0; t < total; t++) {
        win.clear_renderer();
        display_sod_roll_intro_frame((float)t / (float)(total - 1));
        win.update_screen();
        if (t % 60 == 0)
            SDL_Log("pamsod: intro frame %d/%d", t, total);
        HANDLE_SDL_EVENT(
            QUIT();
            LOST_FOCUS(
                set_status(game_state, IS_PAUSED, true);
                Mix_PauseMusic();
                Mix_Pause(-1););)
    }
    SDL_Log("pamsod: intro done");
    return true;
}

} // namespace pamsod
