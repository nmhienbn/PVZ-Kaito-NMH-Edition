#include "game.hpp"
#include "elements/Level/Level.hpp"
#include "elements/Map/Map.hpp"
#include "elements/Player/Player.hpp"
#include "elements/elements.hpp"
#include "elements/mower/mower.hpp"
#include "events/choose_level_scene/quit_menu/quit_menu.hpp"
#include "events/choose_level_scene/render_scene/display_choose_level.hpp"
#include "events/choose_level_scene/reset_level/reset_menu.hpp"
#include "events/in_game/credit/credit.hpp"
#include "events/autoplay/autoplay.hpp"
#include "events/layout_editor/layout_editor.hpp"
#include "events/scene_dump/scene_dump.hpp"
#include "events/seed_chooser/seed_chooser.hpp"
#include "events/selector_scene/selector_scene.hpp"
#include <deque>
#include <functional>
#include <iomanip>
using namespace std;

int game_state = 0;
bool quit = false;
int clk = 0;
Level level;
Elements game_characters;
Player player;
Map cells;
deque<int> shown_level{1, 2, 3, 4, 5};
Window win(WINDOW_WIDTH, WINDOW_HEIGHT);

// F2 layout-editor live redraws for the scenes this file dispatches (see
// layout_editor.hpp for the frozen-replay fallback used elsewhere).
static void choose_level_redraw()
{
    // Same draw part as handle_choosing_level_screen(), without its win.update_screen()
    // (the editor presents the frame itself, after the overlay).
    if (check_status(game_state, IS_QUIT) == true)
        display_game_quit();
    else if (check_status(game_state, IS_RESET) == true)
        display_game_reset();
    else
        display_choose_level(true);
}

// Survey view behind the intro (credit pan / READY-SET-PLANT / the sod
// roll): the unpanned lawn -- exactly where everything lands when gameplay
// starts -- with the parked mowers and the seed bank.
static void survey_redraw()
{
    win.draw_bg(BLACK_SCREEN_DIRECTORY);
    win.draw_bg(level.background_directory, 0, 0);
    display_mowers(game_characters.mowers);
    display_seed_packets_bar();
}

/*
Init game & music. Set mouse cursor
If player is a new user: Get name and unlock Peashooter.
If a level is not chosen:
    Handle unlocking screen if necessary.
    Handle choosing level screen.
Else: start level.
*/
int main(int argv, char **args)
{
    // PVZ_SCENE_DUMP=<out.png>: build the fixed debug scene, save a PNG +
    // report and exit 0 before any normal game init/menu/save-data path.
    if (scene_dump::run_if_requested())
        return 0;

    // PVZ_AUTOPLAY=<scenario> PVZ_AUTOPLAY_OUT=<dir>: headless gameplay
    // simulation; same pre-init placement as the scene dump.
    if (autoplay::run_if_requested())
        return 0;

    // Load all image to check image error
    // for (int i = 0; i < COUNT_USED_DIRECTORY; i++)
    // {
    //     win.load_texture(i);
    //     cout << setw(2) << all_img[i].n_sheet << ' ' << all_img[i].c_sheet << ' ';
    //     cout << all_img[i].img_dir << '\n';
    // }
    // cout << 0 << endl;
    // F2 layout editor: arm the event filter that opens the editor as a
    // nested blocking scene from any scene's event pump.
    layout_editor::install();

    // Init game & music.
    init_music();
    mouse_cursor().set_cursor();
    init_game();

    // If player is a new user: Get name and unlock Peashooter.
    display_get_name_player(NEW_USER_DIRECTORY);
    if (player.unlocked_level == 0)
    {
        set_status(game_state, IS_UNLOCKING_PLANT, true);
        while (!quit && check_status(game_state, IS_UNLOCKING_PLANT) == true)
        {
            unlock_plant(PEASHOOTER_TYPE);
        }
        update_unlocked_level();
    }

    // Land on the selector/mode screen after loading (tombstone picker; the
    // level list is behind ADVENTURE, the almanac behind its own tomb).
    set_status(game_state, IS_SELECTOR_SCREEN, true);

    // Main loop
    while (!quit)
    {
        // 2-5 Whack-a-Zombie: swap the normal cursor for the mallet while a
        // whack level is being played; everywhere else this restores it.
        // The swing animation advances on logic ticks only (auto-FPS: this
        // loop runs at the uncapped render rate during gameplay).
        mouse_cursor().update(check_status(game_state, IS_LEVEL_CHOSEN) &&
                              level.is_special(LevelSpecial::Whack),
                              logic_ticked);
        if (check_status(game_state, IS_LEVEL_CHOSEN) == false)
        {
            // F2 layout-editor redraw per menu scene: the selector screen and
            // the award cutscenes do a full-scene present of their own, so
            // the editor replays the frozen last frame over them (null);
            // the level-list scene redraws live.
            if (check_status(game_state, IS_SELECTOR_SCREEN) ||
                check_status(game_state, IS_UNLOCKING_PLANT))
                layout_editor::set_scene_redraw(nullptr);
            else
                layout_editor::set_scene_redraw(choose_level_redraw);
            if (check_status(game_state, IS_UNLOCKING_PLANT) == true)
            {
                // Code-keyed non-plant awards from the Dave scene table.
                dave::DaveAward award = dave::award_for_level(level.level_num);
                if (award == dave::DAVE_AWARD_SHOVEL)
                {
                    unlock_shovel();
                }
                else if (award == dave::DAVE_AWARD_NOTE)
                {
                    unlock_zombie_note();
                }
                else if (award == dave::DAVE_AWARD_NOTE2)
                {
                    unlock_zombie_note2();
                }
                else if (award == dave::DAVE_AWARD_ALMANAC)
                {
                    unlock_almanac();
                }
                else if (award == dave::DAVE_AWARD_CARKEYS)
                {
                    unlock_carkeys();
                }
                else if (award == dave::DAVE_AWARD_TACO)
                {
                    unlock_taco();
                }
                else if (award == dave::DAVE_AWARD_WATERINGCAN)
                {
                    unlock_wateringcan();
                }
                else
                {
                    for (int i = 0; i < PLANT_COUNT; i++)
                    {
                        if (level.level_num == level_unlock_new_plant[i] - 1)
                        {
                            unlock_plant(i);
                            break;
                        }
                    }
                }
            }
            else if (check_status(game_state, IS_SELECTOR_SCREEN) == true)
            {
                play_music(OPENING_MUSIC_DIRECTORY);
                handle_selector_screen();
            }
            else
            {
                play_music(OPENING_MUSIC_DIRECTORY);
                // win.clear_renderer();
                handle_choosing_level_screen();
                if (check_status(game_state, IS_LEVEL_CHOSEN))
                {
                    Mix_HaltMusic();
                    clk = 0;
                    set_status(game_state, IS_GAME_STARTED, false);
                    // Seed chooser (all_unlocked levels only): credit pan ->
                    // pick -> rewind pan; the in-game credit then skips its own
                    // pan. Restart reuses the pick stored by set_level_seeds.
                    if (level_needs_chooser())
                    {
                        // The chooser owns its blocking frame loop; the
                        // layout editor replays its frozen last frame there
                        // (the panel state is internal to the chooser).
                        layout_editor::set_scene_redraw(nullptr);
                        set_status(game_state, IS_CHOOSING_SEEDS, true);
                        SeedChooserExit chooser_exit = run_seed_chooser();
                        set_status(game_state, IS_CHOOSING_SEEDS, false);
                        if (chooser_exit == SeedChooserExit::BACK_TO_MENU)
                        {
                            // MENU in the chooser: drop the level pick and land
                            // back on the selector/mode screen.
                            set_status(game_state, IS_LEVEL_CHOSEN, false);
                            set_status(game_state, IS_SELECTOR_SCREEN, true);
                            continue;
                        }
                    }
                    // No fade into the level: the entry handshake (chooser
                    // rewind or the credit pan) already ends on the lawn that
                    // display_credit's sod roll starts from, so a fade_out
                    // here only blinked the frame black before READY-SET-PLANT.
                }
            }
        }
        else
        {
            win.clear_renderer();
            // F2 layout editor: the event filter opens it from the scene's
            // own pump (it covers the whole intro too -- credit pan, sod
            // roll, READY-SET-PLANT); give it the live redraw for this scene.
            layout_editor::set_scene_redraw(check_status(game_state, IS_GAME_STARTED)
                                                ? std::function<void()>(display_game_layout)
                                                : std::function<void()>(survey_redraw));
            start_level();
        }
    }
    close_music();
    return 0;
}
