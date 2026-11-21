#include "selector_scene.hpp"
#include "elements/Player/Player.hpp"
#include "elements/button/button.hpp"
#include "events/almanac/almanac.hpp"
#include "events/anim_viewer/anim_viewer.hpp"
#include "music/music.hpp"
#include "player/player_name.hpp"
#include "reanim/reanim.hpp"
#include "reanim/reanim_sdl.hpp"
#include <cmath>
#include <unordered_set>

extern int game_state;
extern bool quit;
extern Player player;
extern Window win;

// The clone window is 1050 wide; the original scene is authored for 800x600
// at 1:1 scale, centered horizontally (+125 px).
const int SELECTOR_X_OFF = (WINDOW_WIDTH - 800) / 2;
// SelectorScreen.reanim: anim_open 0..12, anim_sign 13..40, then the ambient
// loop (idle/grass/flowers/clouds) 41..705 at the doc's 20 fps.
const float SELECTOR_REST_FRAME = 41.0f;
const float SELECTOR_LAST_FRAME = 705.0f;

// Mode planks (reanim tracked positions, verified against the 2Pak
// SelectorScreen.reanim rest pose) and the bottom-row tombs/buttons.
const Button ADVENTURE_BUTTON(SELECTOR_X_OFF + 405, SELECTOR_X_OFF + 405 + 330, 80, 222);
const Button MINIGAMES_BUTTON(SELECTOR_X_OFF + 406, SELECTOR_X_OFF + 406 + 313, 173, 306);
const Button PUZZLE_BUTTON(SELECTOR_X_OFF + 410, SELECTOR_X_OFF + 410 + 286, 257, 379);
const Button SURVIVAL_BUTTON(SELECTOR_X_OFF + 413, SELECTOR_X_OFF + 413 + 266, 328, 451);
const Button ALMANAC_TOMB_BUTTON(SELECTOR_X_OFF + 327, SELECTOR_X_OFF + 327 + 128, 428, 556);
const Button STORE_TOMB_BUTTON(SELECTOR_X_OFF + 405, SELECTOR_X_OFF + 405 + 170, 484, 573);
const Button ZEN_GARDEN_BUTTON(SELECTOR_X_OFF + 171, SELECTOR_X_OFF + 171 + 191, 401, 564);
const Button OPTIONS_BUTTON(SELECTOR_X_OFF + 565, SELECTOR_X_OFF + 565 + 81, 490, 521);
const Button HELP_BUTTON(SELECTOR_X_OFF + 647, SELECTOR_X_OFF + 647 + 48, 529, 551);
const Button QUIT_BUTTON_SELECTOR(SELECTOR_X_OFF + 715, SELECTOR_X_OFF + 715 + 47, 515, 542);

/*Wood-sign chrome a la the retail GameSelector: the big hanging plaque shows
the current player's name ("Name!", FONT_BRIANNETOD16 white-cream), and the
smaller plank under it is the rename hot spot (retail mChangeUserButton,
tracked to woodsign2 with offset +24/+10 and size 250x30). Both texts ride
the sign's settle swing by sampling the reanim track every frame; the hit box
follows the same sample, so hover/click stay glued to the moving plank.*/
const int WOOD_SIGN_NAME_SIZE = 22;
const int WOOD_SIGN_HINT_SIZE = 15;
const RGB WOOD_SIGN_INK(255, 245, 200); // retail ink color
const RGB WOOD_SIGN_INK_HOT(255, 255, 240);

// (ALMANAC_UNLOCK_LEVEL comes from almanac.hpp; shared with the seed
// chooser's VIEW ALMANAC button.)

namespace
{
reanim::ReanimDoc g_selector_doc;
reanim::SdlReanimRenderer *g_selector_renderer = nullptr;
bool g_selector_ready = false;
bool g_selector_tried = false;
float g_selector_frame = SELECTOR_REST_FRAME;

// ADVENTURE-click grave sequence: the Zombie_hand reanim rises from the dirt
// under the tombstone while gravebutton + evil laugh play, then the scene
// leaves for the level list (retail GameSelector does the same).
reanim::ReanimDoc g_hand_doc;
bool g_hand_ready = false;
bool g_hand_tried = false;
int g_hand_tick = -1; // -1 idle; 60-tps ticks since the ADVENTURE click
// 25-frame hand doc at the scene's 20 fps = 1.25 s of rising; hold the risen
// pose a few more ticks so the whole bit lands at ~1.5 s.
const int HAND_LEAVE_TICKS = 90; // ~1.5 s at 60 tps

bool selector_reanim_ready()
{
    if (g_selector_tried)
        return g_selector_ready;
    g_selector_tried = true;
    std::string err;
    if (!reanim::LoadReanimXml("./resources/reanim/SelectorScreen.reanim",
                               g_selector_doc, err))
    {
        SDL_Log("selector scene: %s", err.c_str());
        return g_selector_ready;
    }
    static reanim::SdlReanimRenderer renderer(win.sprite_renderer());
    g_selector_renderer = &renderer;
    if (!g_selector_renderer->BindDirectory("./resources/reanim"))
    {
        SDL_Log("selector scene: no part images in resources/reanim");
        return g_selector_ready;
    }
    g_selector_ready = true;
    return true;
}

bool hand_reanim_ready()
{
    if (g_hand_tried)
        return g_hand_ready;
    g_hand_tried = true;
    if (!selector_reanim_ready()) // the renderer and its bound dir come from here
        return g_hand_ready;
    std::string err;
    if (!reanim::LoadReanimXml("./resources/reanim/Zombie_hand.reanim",
                               g_hand_doc, err))
    {
        SDL_Log("selector scene: %s", err.c_str());
        return g_hand_ready;
    }
    g_hand_ready = true;
    return true;
}

// The 2Pak doc drives everything with the intro segments (anim_open/anim_sign,
// frames 0..40) and then hides (f=-1) every pose-static part for the whole
// ambient loop -- the original keeps the finished intro pose up: three
// background pieces, the four mode tombstones with their dirt mounds, the
// hanging wood sign. Their keyframes hold a constant rest pose over 41..705,
// so force-showing them reproduces the retail scene (the wood sign keeps its
// 41..77 settle swing because force-show only rescues f=-1 frames).
// StartAdventure and its shadow stay hidden on purpose; the almanac shadow
// and 8x gradient stay hidden too (own almanac tomb art / stretched bg).
const std::unordered_set<std::string> &selector_rest_parts()
{
    static const std::unordered_set<std::string> rest_parts = {
        "selectorscreen_bg_left",   // left woods
        "selectorscreen_bg_center", // middle grass + dirt band
        "selectorscreen_bg_right",  // right hill the tombstones sit on
        // the four mode planks and their dirt mounds (the doc's
        // Survival/Challenges/ZenGarden track labels predate the shipped
        // plank captions; the existing hover rects match their rest poses)
        "selectorscreen_adventure_shadow",
        "selectorscreen_adventure_button", // ADVENTURE tombstone
        "selectorscreen_survival_shadow",
        "selectorscreen_survival_button",
        "selectorscreen_challenges_shadow",
        "selectorscreen_challenges_button",
        "selectorscreen_zengarden_shadow",
        "selectorscreen_zengarden_button",
        "woodsign1", "woodsign2", "woodsign3", // hanging wood sign
    };
    return rest_parts;
}

// The plank/dir highlighting arts (drawn on top of the reanim plank when the
// mouse is over it).
void draw_plank_highlight(const Button &b, int highlight_dir)
{
    int _x = 0, _y = 0;
    SDL_GetMouseState(&_x, &_y);
    if (b.is_mouse_in(_x, _y))
        win.draw_png(highlight_dir, b.x1, b.y1, b.x2 - b.x1, b.y2 - b.y1);
}

void draw_tomb_button(const Button &b, int normal_dir, int highlight_dir)
{
    int _x = 0, _y = 0;
    SDL_GetMouseState(&_x, &_y);
    b.display(b.is_mouse_in(_x, _y) ? highlight_dir : normal_dir);
}

const reanim::Track *find_doc_track(const char *name)
{
    for (const reanim::Track &t : g_selector_doc.tracks)
        if (t.name == name)
            return &t;
    return nullptr;
}

// Live rename-plank hit box, refreshed by render_selector_frame from the
// woodsign2 track sample (so it keeps following the intro settle swing);
// consumed by both the hover tint and the selector's click handler.
Button g_rename_sign_hit;

void draw_wood_sign_chrome()
{
    if (const reanim::Track *sign = find_doc_track("woodsign1"))
    {
        const reanim::Transform st = reanim::SampleTrack(*sign, g_selector_frame);
        if (player.name.size())
        {
            const std::string welcome = player.name + "!";
            TTF_Font *font = win.get_font(BRIANNE_TTF, WOOD_SIGN_NAME_SIZE);
            int tw = 0, th = 0;
            TTF_SizeText(font, welcome.c_str(), &tw, &th);
            // retail: centered at local x 170.5, baseline local y 102.5
            const int tx = SELECTOR_X_OFF + (int)lroundf(st.x) + 170 - tw / 2;
            const int ty = (int)lroundf(st.y + 102.5f) - TTF_FontAscent(font);
            win.show_text(welcome, tx, ty, WOOD_SIGN_INK, BRIANNE_TTF, WOOD_SIGN_NAME_SIZE);
        }
    }
    if (const reanim::Track *plank = find_doc_track("woodsign2"))
    {
        const reanim::Transform st = reanim::SampleTrack(*plank, g_selector_frame);
        const int px = SELECTOR_X_OFF + (int)lroundf(st.x);
        const int py = (int)lroundf(st.y);
        g_rename_sign_hit = Button(px + 24, px + 274, py + 10, py + 40);
        int _x = 0, _y = 0;
        SDL_GetMouseState(&_x, &_y);
        const bool hot = g_rename_sign_hit.is_mouse_in(_x, _y);
        const string hint = "if this is not you, click here";
        TTF_Font *font = win.get_font(BRIANNE_TTF, WOOD_SIGN_HINT_SIZE);
        int tw = 0, th = 0;
        TTF_SizeText(font, hint.c_str(), &tw, &th);
        win.show_text(hint, (g_rename_sign_hit.x1 + g_rename_sign_hit.x2 - tw) / 2,
                      (g_rename_sign_hit.y1 + g_rename_sign_hit.y2) / 2 + 1 - th / 2,
                      hot ? WOOD_SIGN_INK_HOT : WOOD_SIGN_INK,
                      BRIANNE_TTF, WOOD_SIGN_HINT_SIZE);
    }
}

} // namespace

/*
Draw one frame of the selector screen (no present): stretched backdrop, then
the authentic 2Pak SelectorScreen reanim scene (four mode planks, clouds,
flowers, signs), then the bottom-row tombs (almanac/store/zen garden) and the
options/help/quit strip. Exported so the offscreen frame-dump harness can
read the composed frame back before a present swaps it away; the in-game loop
uses display_selector_screen().
*/
void render_selector_frame()
{
    // Blocking scene pacing itself at the 60 fps cap: force the auto-FPS
    // gate so the gated callees below (show_announcer_text's countdown)
    // keep advancing per frame -- logic_ticked would otherwise hold a
    // stale value from the last gameplay frame.
    logic_ticked = 1;
    win.clear_renderer();
    win.draw_png(SELECTORS_BACKGROUND_DIRECTORY, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    if (selector_reanim_ready())
    {
        reanim::DrawOptions rest_opts;
        rest_opts.forceShow = &selector_rest_parts();
        // StartAdventure plank and its shadow stay hidden; the clone always
        // shows the regular ADVENTURE plank.
        g_selector_renderer->Draw(g_selector_doc, g_selector_frame,
                                  SELECTOR_X_OFF, 0, 1.0f,
                                  [](const std::string &name)
                                  {
                                      return name == "SelectorScreen_StartAdventure_button" ||
                                             name == "SelectorScreen_StartAdventure_shadow";
                                  },
                                  rest_opts);
        draw_plank_highlight(ADVENTURE_BUTTON, SELECTORS_ADVENTURE_HIGHLIGHT_DIRECTORY);
        draw_plank_highlight(MINIGAMES_BUTTON, SELECTORS_MINIGAMES_HIGHLIGHT_DIRECTORY);
        draw_plank_highlight(PUZZLE_BUTTON, SELECTORS_PUZZLE_HIGHLIGHT_DIRECTORY);
        draw_plank_highlight(SURVIVAL_BUTTON, SELECTORS_SURVIVAL_HIGHLIGHT_DIRECTORY);
        draw_wood_sign_chrome();
    }

    // Bottom area: almanac (once found), store and zen garden tombs
    if (player.unlocked_level >= ALMANAC_UNLOCK_LEVEL)
        draw_tomb_button(ALMANAC_TOMB_BUTTON, SELECTORS_ALMANAC_DIRECTORY,
                         SELECTORS_ALMANAC_HIGHLIGHT_DIRECTORY);
    draw_tomb_button(STORE_TOMB_BUTTON, SELECTORS_STORE_DIRECTORY,
                     SELECTORS_STORE_HIGHLIGHT_DIRECTORY);
    draw_tomb_button(ZEN_GARDEN_BUTTON, SELECTORS_ZEN_DIRECTORY,
                     SELECTORS_ZEN_HIGHLIGHT_DIRECTORY);

    // Options / Help / Quit text strip
    draw_tomb_button(OPTIONS_BUTTON, SELECTORS_OPTIONS_DIRECTORY,
                     SELECTORS_OPTIONS_HIGHLIGHT_DIRECTORY);
    draw_tomb_button(HELP_BUTTON, SELECTORS_HELP_DIRECTORY,
                     SELECTORS_HELP_HIGHLIGHT_DIRECTORY);
    draw_tomb_button(QUIT_BUTTON_SELECTOR, SELECTORS_QUIT_DIRECTORY,
                     SELECTORS_QUIT_HIGHLIGHT_DIRECTORY);

    // ADVENTURE-click sequence: the zombie hand rises from the dirt (over the
    // scene, doc authored in 800x600 scene space) until the level list takes
    // over.
    if (g_hand_tick >= 0 && hand_reanim_ready())
        g_selector_renderer->Draw(g_hand_doc, g_hand_tick * 20.0f / FPS,
                                  SELECTOR_X_OFF, 0, 1.0f);

    win.show_announcer_text();

    g_selector_frame += 20.0f / FPS;
    if (g_selector_frame > SELECTOR_LAST_FRAME)
        g_selector_frame = SELECTOR_REST_FRAME;
}

/*One full scene frame: draw + present (per-frame handler's draw half).*/
void display_selector_screen()
{
    render_selector_frame();
    win.update_screen();
}

// Backdrop pass for the modal rename dialog opened from the wood sign:
// keep the tombstone scene animating behind the dim + dialog art.
void selector_dialog_backdrop(const bool &)
{
    render_selector_frame();
}

/*
Handle one frame of the selector/mode screen.
ADVENTURE starts the grave sequence: gravebutton + evil laugh, the Zombie_hand
reanim rises from the dirt (~1.5 s, all clicks but QUIT are swallowed), then
hands over to the existing level-list scene; the wood sign's small plank
opens the rename dialog (modal, selector animating behind it); ALMANAC opens
the suburban almanac and comes back; QUIT ends the game; the remaining modes
use the same "not yet" announcement as the original stub.
*/
void handle_selector_screen()
{
    display_selector_screen();
    HANDLE_SDL_EVENT(
        QUIT();
        LCLICK({
            if (QUIT_BUTTON_SELECTOR.is_mouse_in(mouse_x, mouse_y))
            {
                play_sound_effect(BUTTON_CLICK_MUSIC_DIRECTORY);
                quit = true;
                break;
            }
            if (g_hand_tick >= 0)
                break; // mid grave sequence: swallow every other click
            // Rename: the small wood-sign plank ("if this is not you...").
            if (g_rename_sign_hit.is_mouse_in(mouse_x, mouse_y))
            {
                play_sound_effect(BUTTON_CLICK_MUSIC_DIRECTORY);
                display_get_name_player(RENAME_MENU_DIRECTORY, selector_dialog_backdrop);
                break;
            }
            if (ADVENTURE_BUTTON.is_mouse_in(mouse_x, mouse_y))
            {
                play_sound_effect(GRAVEBUTTON_MUSIC_DIRECTORY);
                play_sound_effect(EVIL_LAUGH_MUSIC_DIRECTORY);
                g_hand_tick = 0;
                break;
            }
            if (player.unlocked_level >= ALMANAC_UNLOCK_LEVEL &&
                ALMANAC_TOMB_BUTTON.is_mouse_in(mouse_x, mouse_y))
            {
                play_sound_effect(BUTTON_CLICK_MUSIC_DIRECTORY);
                run_almanac();
                break;
            }
            if (MINIGAMES_BUTTON.is_mouse_in(mouse_x, mouse_y) ||
                PUZZLE_BUTTON.is_mouse_in(mouse_x, mouse_y) ||
                SURVIVAL_BUTTON.is_mouse_in(mouse_x, mouse_y) ||
                STORE_TOMB_BUTTON.is_mouse_in(mouse_x, mouse_y) ||
                ZEN_GARDEN_BUTTON.is_mouse_in(mouse_x, mouse_y) ||
                OPTIONS_BUTTON.is_mouse_in(mouse_x, mouse_y) ||
                HELP_BUTTON.is_mouse_in(mouse_x, mouse_y))
            {
                play_sound_effect(BUTTON_CLICK_MUSIC_DIRECTORY);
                win.show_announcer_text("NOT AVAILABLE YET!");
                break;
            }
        });
        // Debug animation viewer (designer tooling; blocks over the selector,
        // ESC/F1 inside it returns here).
        KEY_PRESS(F1, display_anim_viewer(););
    );
    if (g_hand_tick >= 0)
    {
        g_hand_tick++;
        if (g_hand_tick >= HAND_LEAVE_TICKS)
        {
            g_hand_tick = -1;
            set_status(game_state, IS_SELECTOR_SCREEN, false);
        }
    }
}
