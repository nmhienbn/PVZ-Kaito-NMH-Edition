#include "dave_scene.hpp"

#include "draw/render_elements.hpp"
#include "draw/rsdl.hpp"
#include "elements/Level/Level.hpp"
#include "elements/Map/Map.hpp"
#include "elements/Player/Player.hpp"
#include "elements/conveyor/conveyor.hpp"
#include "elements/elements.hpp"
#include "elements/plants/plant_type.hpp"
#include "music/music.hpp"
#include "reanim/reanim_sdl.hpp"

#include <SDL_mixer.h>
#include <cmath>

extern int game_state;
extern bool quit;
extern Level level;
extern Elements game_characters;
extern Player player;
extern Map cells;
extern Window win;
extern int sfx_volume; // defined in music/music.cpp

namespace dave
{
namespace
{

// ---------------------------------------------------------------------------
// Reanim actor: Crazydave.reanim (2Pak conversion) over the game GL pipeline.
// Segments (measured from the XML): anim_enter 0..38, anim_idle 39..95,
// anim_smalltalk 96..102, anim_mediumtalk 103..124, anim_blahblah 125..171,
// anim_crazy 172..223, anim_leave 224..240 (fps 12).
// ---------------------------------------------------------------------------
const char *REANIM_DIR = "./resources/reanim";
const char *REANIM_FILE = "./resources/reanim/Crazydave.reanim";

const float DAVE_ART_HEIGHT = 380.0f; // on-screen height of the idle art
const float DAVE_FOOT_Y = 510.0f;     // bottom anchor (screen px)
const float DAVE_CENTER_X = 165.0f;   // horizontal center (screen px)

struct AnimState
{
    reanim::ReanimDoc doc;
    reanim::SdlReanimRenderer *renderer = nullptr;
    float scale = 1.0f;
    float ref_cx = 0.0f;    // idle bbox center x, reanim units
    float ref_bottom = 0.0f; // idle bbox bottom, reanim units

    int seg_first = 0, seg_last = 0;
    bool seg_loop = true;
    Uint32 seg_begin = 0;
};
AnimState gAnim;
bool gLoadTried = false;
bool gLoaded = false;

// Never draw the anim_grab zombie swing (zombie_* / credits_* tracks) or the
// artist guide layers.
bool hide_track(const string &name)
{
    string n = name;
    for (char &c : n)
        c = (char)tolower((unsigned char)c);
    return n.find("zombie") != string::npos || n.find("credits") != string::npos ||
           n.find("layer") != string::npos;
}

bool set_segment(const char *name)
{
    auto it = gAnim.doc.segments.find(name);
    if (it == gAnim.doc.segments.end() || it->second.last < it->second.first)
        return false;
    gAnim.seg_first = it->second.first;
    gAnim.seg_last = it->second.last;
    return true;
}

bool load_dave()
{
    if (gLoadTried)
        return gLoaded;
    gLoadTried = true;
    static reanim::SdlReanimRenderer renderer(win.sprite_renderer());
    gAnim.renderer = &renderer;
    if (!renderer.BindDirectory(REANIM_DIR))
    {
        SDL_Log("dave scene: no part images in %s (scene is skipped)", REANIM_DIR);
        return false;
    }
    string err;
    if (!reanim::LoadReanimXml(REANIM_FILE, gAnim.doc, err))
    {
        SDL_Log("dave scene: %s (scene is skipped)", err.c_str());
        return false;
    }
    // Calibrate scale/anchor against the anim_idle bbox so Dave lands at a
    // fixed on-screen size, feet on DAVE_FOOT_Y.
    int first = 0, last = 0;
    set_segment("anim_idle");
    first = gAnim.seg_first;
    last = gAnim.seg_last;
    reanim::DrawOptions measure;
    measure.measureOnly = true;
    reanim::DrawStats acc;
    for (int f = first; f <= last; f += 4)
    {
        reanim::DrawStats st = renderer.Draw(gAnim.doc, (float)f, 0, 0, 1.0f, hide_track, measure);
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
    if (!acc.hasBounds || acc.maxY - acc.minY <= 1.0f)
    {
        SDL_Log("dave scene: empty anim_idle (scene is skipped)");
        return false;
    }
    gAnim.scale = DAVE_ART_HEIGHT / (acc.maxY - acc.minY);
    gAnim.ref_cx = (acc.minX + acc.maxX) * 0.5f;
    gAnim.ref_bottom = acc.maxY;
    SDL_Log("dave scene: idle bbox %.0fx%.0f, scale %.3f", acc.maxX - acc.minX,
            acc.maxY - acc.minY, gAnim.scale);
    gLoaded = true;
    return true;
}

void play_segment(const char *name, bool loop)
{
    if (!set_segment(name))
        set_segment("anim_idle");
    gAnim.seg_loop = loop;
    gAnim.seg_begin = SDL_GetTicks();
}

// Live frame of the current segment (looped, or clamped for one-shots).
float segment_frame()
{
    int len = gAnim.seg_last - gAnim.seg_first + 1;
    float elapsed = (float)(SDL_GetTicks() - gAnim.seg_begin) * gAnim.doc.fps / 1000.0f;
    if (gAnim.seg_loop)
    {
        float local = fmodf(elapsed, (float)len);
        return (float)gAnim.seg_first + local;
    }
    return (float)gAnim.seg_first + std::min(elapsed, (float)(len - 1));
}

bool segment_finished() // one-shots only
{
    int len = gAnim.seg_last - gAnim.seg_first + 1;
    float elapsed = (float)(SDL_GetTicks() - gAnim.seg_begin) * gAnim.doc.fps / 1000.0f;
    return elapsed >= (float)len;
}

void draw_dave()
{
    float frame = segment_frame();
    reanim::DrawOptions opt;
    // Loop wrap (see loopWrapFirst in DrawOptions): one-shots clamp instead.
    opt.loopWrapFirst = gAnim.seg_loop ? gAnim.seg_first : -1;
    opt.loopWrapLast = gAnim.seg_loop ? gAnim.seg_last : -1;
    float ox = DAVE_CENTER_X - gAnim.scale * gAnim.ref_cx;
    float oy = DAVE_FOOT_Y - gAnim.scale * gAnim.ref_bottom;
    gAnim.renderer->Draw(gAnim.doc, frame, ox, oy, gAnim.scale, hide_track, opt);
}

const char *talk_segment(DaveVoice voice)
{
    switch (voice)
    {
    case DAVE_VOICE_SHORT:
        return "anim_smalltalk";
    case DAVE_VOICE_SCREAM:
        return "anim_crazy";
    case DAVE_VOICE_EXTRALONG:
        return "anim_blahblah";
    default:
        return "anim_mediumtalk";
    }
}

// ---------------------------------------------------------------------------
// Voice clips. Kept out of music/music.hpp on purpose (its entry table is
// mid-edit by another change): lazy Mix_LoadWAV + static cache.
// TODO: fold these into the global sound registry (music.hpp/music.cpp) once
// that file is free to extend.
// ---------------------------------------------------------------------------
void play_voice(DaveVoice voice)
{
    static const char *files[DAVE_VOICE_COUNT][3] = {
        {"./resources/audio/crazydaveshort1.ogg", "./resources/audio/crazydaveshort2.ogg",
         "./resources/audio/crazydaveshort3.ogg"},
        {"./resources/audio/crazydavelong1.ogg", "./resources/audio/crazydavelong2.ogg",
         "./resources/audio/crazydavelong3.ogg"},
        {"./resources/audio/crazydaveextralong1.ogg", "./resources/audio/crazydaveextralong2.ogg",
         "./resources/audio/crazydaveextralong3.ogg"},
        {"./resources/audio/crazydavescream.ogg", "./resources/audio/crazydavescream2.ogg", nullptr},
    };
    static Mix_Chunk *cache[DAVE_VOICE_COUNT][3] = {};
    static bool tried[DAVE_VOICE_COUNT][3] = {};
    if (voice < 0 || voice >= DAVE_VOICE_COUNT)
        return;
    int variants = 0;
    while (variants < 3 && files[voice][variants] != nullptr)
        variants++;
    if (variants == 0)
        return;
    int i = rand(0, variants - 1);
    if (!tried[voice][i])
    {
        tried[voice][i] = true;
        cache[voice][i] = Mix_LoadWAV(files[voice][i]);
        if (cache[voice][i] == nullptr)
            printf("dave scene: failed to load %s! SDL_mixer Error: %s\n", files[voice][i],
                   Mix_GetError());
        else
            Mix_VolumeChunk(cache[voice][i], sfx_volume);
    }
    if (cache[voice][i] != nullptr)
        Mix_PlayChannel(-1, cache[voice][i], 0);
}

// ---------------------------------------------------------------------------
// Speech bubble: 9-slice from the PvZ1 dialog tiles (corner art is large:
// top edge 97 high, bottom 114, sides 107/120 wide; the middle strips
// stretch). Text is Brianne's hand, centered in the paper area.
// ---------------------------------------------------------------------------
const int TILE_LEFT_W = 107;
const int TILE_RIGHT_W = 120;
const int TILE_TOP_H = 97;
const int TILE_BOT_H = 114;
const int BUBBLE_FONT_SIZE = 26;
const int BUBBLE_LINE_H = 30;
const int BUBBLE_X = 250;
const int BUBBLE_Y = 30;
const int BUBBLE_TEXT_MAX_W = 380;
const int BUBBLE_MAX_W = 640;

vector<string> wrap_words(const string &text, int max_w)
{
    TTF_Font *font = win.get_font(BRIANNE_TTF, BUBBLE_FONT_SIZE);
    vector<string> out;
    string cur;
    size_t pos = 0;
    while (pos < text.size())
    {
        size_t sp = text.find(' ', pos);
        string word = text.substr(pos, sp == string::npos ? string::npos : sp - pos);
        string trial = cur.empty() ? word : cur + " " + word;
        int w = 0, h = 0;
        TTF_SizeText(font, trial.c_str(), &w, &h);
        if (!cur.empty() && w > max_w)
        {
            out.push_back(cur);
            cur = word;
        }
        else
            cur = trial;
        if (sp == string::npos)
            break;
        pos = sp + 1;
    }
    if (!cur.empty())
        out.push_back(cur);
    if (out.empty())
        out.push_back("");
    return out;
}

void draw_bubble(const string &text)
{
    TTF_Font *font = win.get_font(BRIANNE_TTF, BUBBLE_FONT_SIZE);
    vector<string> lines = wrap_words(text, BUBBLE_TEXT_MAX_W);
    int max_line_w = 0;
    for (const auto &line : lines)
    {
        int w = 0, h = 0;
        TTF_SizeText(font, line.c_str(), &w, &h);
        max_line_w = max(max_line_w, w);
    }
    int box_w = min(BUBBLE_MAX_W, max_line_w + 190);
    int box_h = TILE_TOP_H + TILE_BOT_H + (int)lines.size() * BUBBLE_LINE_H + 4;
    int bx = BUBBLE_X;
    int by = BUBBLE_Y;

    win.draw_png(DIALOG_TOPLEFT_DIRECTORY, bx, by, TILE_LEFT_W, TILE_TOP_H);
    win.draw_png(DIALOG_TOPMIDDLE_DIRECTORY, bx + TILE_LEFT_W, by,
                 box_w - TILE_LEFT_W - TILE_RIGHT_W, TILE_TOP_H);
    win.draw_png(DIALOG_TOPRIGHT_DIRECTORY, bx + box_w - TILE_RIGHT_W, by,
                 TILE_RIGHT_W, TILE_TOP_H);
    int mid_h = box_h - TILE_TOP_H - TILE_BOT_H;
    win.draw_png(DIALOG_CENTERLEFT_DIRECTORY, bx, by + TILE_TOP_H, TILE_LEFT_W, mid_h);
    win.draw_png(DIALOG_CENTERMIDDLE_DIRECTORY, bx + TILE_LEFT_W, by + TILE_TOP_H,
                 box_w - TILE_LEFT_W - TILE_RIGHT_W, mid_h);
    win.draw_png(DIALOG_CENTERRIGHT_DIRECTORY, bx + box_w - TILE_RIGHT_W, by + TILE_TOP_H,
                 TILE_RIGHT_W, mid_h);
    win.draw_png(DIALOG_BOTTOMLEFT_DIRECTORY, bx, by + box_h - TILE_BOT_H, TILE_LEFT_W, TILE_BOT_H);
    win.draw_png(DIALOG_BOTTOMMIDDLE_DIRECTORY, bx + TILE_LEFT_W, by + box_h - TILE_BOT_H,
                 box_w - TILE_LEFT_W - TILE_RIGHT_W, TILE_BOT_H);
    win.draw_png(DIALOG_BOTTOMRIGHT_DIRECTORY, bx + box_w - TILE_RIGHT_W, by + box_h - TILE_BOT_H,
                 TILE_RIGHT_W, TILE_BOT_H);
    win.draw_png(DIALOG_HEADER_DIRECTORY, bx + (box_w - 187) / 2, by + 4, 187, 64);

    int ty = by + TILE_TOP_H + 2;
    for (const auto &line : lines)
    {
        int w = 0, h = 0;
        TTF_SizeText(font, line.c_str(), &w, &h);
        win.show_text(line, bx + (box_w - w) / 2, ty + (BUBBLE_LINE_H - h) / 2, BLACK,
                      BRIANNE_TTF, BUBBLE_FONT_SIZE);
        ty += BUBBLE_LINE_H;
    }
}

// ---------------------------------------------------------------------------
// Blocking scene loop over the frozen lawn (display_game_layout redraws the
// level; handle_changes never runs while the dialog owns the frame).
// Returns true when the player asked to advance (click / any key).
// ---------------------------------------------------------------------------
const DaveLine *g_bubble_line = nullptr;

void draw_frame()
{
    // Blocking dialog loop: the gameplay frame driver (level.cpp) is not
    // running, so the auto-FPS gate would hold a stale value -- force it on
    // so lawn anims (1-5's pre-placed peashooters) keep animating per frame.
    logic_ticked = 1;
    win.clear_renderer();
    display_game_layout();
    if (gLoaded)
        draw_dave();
    if (g_bubble_line != nullptr)
        draw_bubble(g_bubble_line->text);
    win.update_screen();
}

bool pump_events()
{
    bool advance = false;
    HANDLE_SDL_EVENT(
        QUIT();
        if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT)
            advance = true;
        if (e.type == SDL_KEYDOWN)
            advance = true;);
    return advance;
}

void wait_advance()
{
    while (!quit)
    {
        draw_frame();
        if (pump_events())
            return;
        SDL_Delay(1);
    }
}

// Enter / leave clips play out unattended; a click fast-forwards.
void play_until_done()
{
    while (!quit)
    {
        draw_frame();
        if (segment_finished() || pump_events())
            return;
        SDL_Delay(1);
    }
}

void run_dialog(const vector<DaveLine> &lines)
{
    if (!load_dave())
        return; // assets missing: skip the scene, keep the level playable
    if (gLoaded)
    {
        play_segment("anim_enter", false);
        play_until_done();
    }
    for (const auto &line : lines)
    {
        play_voice(line.voice);
        if (gLoaded)
            play_segment(talk_segment(line.voice), true);
        g_bubble_line = &line;
        wait_advance();
        if (quit)
        {
            g_bubble_line = nullptr;
            return;
        }
    }
    g_bubble_line = nullptr;
    if (gLoaded)
    {
        play_segment("anim_leave", false);
        play_until_done();
    }
}

// ---------------------------------------------------------------------------
// Level 1-5 dig phase.
// ---------------------------------------------------------------------------
enum DigState
{
    DIG_INACTIVE = 0,
    DIG_ACTIVE,
    DIG_FINISHED,
};
int dig_state = DIG_INACTIVE;
int dug_count = 0;
string dig_advice_shown;

void place_dave_peashooters()
{
    dug_count = 0;
    for (const auto &cr : preplaced_peashooters_1_5())
    {
        int col = cr.first, row = cr.second;
        if (row < 0 || col < 0 || row >= (int)cells.size() || col >= (int)cells[row].size())
            continue;
        game_characters.plants.push_back(init_plant(PEASHOOTER_TYPE, row, col));
        cells[row][col].is_planted = true;
        cells[row][col].is_block_zombie = true;
    }
}

void intro_1_5()
{
    // Dave's lawn: 3 pre-placed peashooters, shovel present, bowling belt held
    // closed until the dig phase resolves (reopened in update_in_game()).
    place_dave_peashooters();
    level.has_shovel = true;
    belt.active = false;
    run_dialog(script_intro_1_5());
    dig_state = DIG_ACTIVE;
    dig_advice_shown.clear();
}

void set_dig_advice(const char *text)
{
    if (dig_advice_shown == text)
        return;
    dig_advice_shown = text;
    level.advice.show(text, ADVICE_STYLE_BANNER, ADVICE_CLK_PERSISTENT);
}

} // namespace

bool is_first_play()
{
    return level.level_num == player.unlocked_level;
}

void play_intro()
{
    dig_state = DIG_INACTIVE;
    if (!has_intro_scene(level.level_num) || !is_first_play())
        return;
    if (level.level_num == 5)
        intro_1_5();
    else // 11: first night level, plain dialog
        run_dialog(script_intro_2_1());
}

void update_in_game()
{
    if (dig_state != DIG_ACTIVE)
        return;

    // Hold wave 1 while the lawn clearing is in progress (belt is inactive).
    level.next_wave_clk = max(level.next_wave_clk, 600);

    int left = (int)game_characters.plants.size();
    if (left < (int)preplaced_peashooters_1_5().size())
        dug_count = 1;

    if (left > 0)
    {
        if (player.is_shoveling)
            set_dig_advice(advice_remove_plant());
        else if (dug_count == 0)
            set_dig_advice(advice_pick_shovel());
        else
            set_dig_advice(advice_keep_digging());
        return;
    }

    // Lawn clear: Dave hands over the Wall-nut and the bowling starts.
    dig_state = DIG_FINISHED;
    level.advice.hide();
    dig_advice_shown.clear();
    player.is_shoveling = false;
    run_dialog(script_bowling_1_5());
    belt.active = true;
    belt.push(WALLNUT_TYPE); // "HERE, TAKE THIS WALL-NUT!!"
    level.next_wave_clk = 600;
    level.has_shovel = false; // bowling is throw-only (level_data rule)
}

} // namespace dave
