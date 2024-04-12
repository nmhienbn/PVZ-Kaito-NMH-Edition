#include "start_game.hpp"
#include "LoadSprout.hpp"
#include "LoadZomHead.hpp"

extern bool quit;
extern Level level;
extern Elements game_characters;
extern Player player;
extern Map cells;
extern Window win;

const int LOADING_TIME = 250;
const int DISPLAY_LOGO_TIME = 200;

vector<LoadSprout> sprouts{
    LoadSprout(240, SDL_FLIP_NONE, 80, 54),
    LoadSprout(300, SDL_FLIP_HORIZONTAL, 104, 70),
    LoadSprout(410, SDL_FLIP_NONE, 67, 45),
    LoadSprout(530, SDL_FLIP_HORIZONTAL, 89, 60)};
LoadZomHead zom_head(650, SDL_FLIP_NONE, 122, 104);

const RGB UNTAP(225, 187, 34);
const RGB TAPPED(255, 33, 4);

/*Initialize game:
    Display loading and starting game.
    Read player data
    Create block on frontyard (playground)
*/
void init_game()
{
    play_music(OPENING_MUSIC_DIRECTORY);
    display_loading_screen();
    display_starting_screen();
    read_savedata();
    cells = create_a_collection_of_blocks();
}

/*
Display starting screen.
Updated: Now player can exit game from here.
Handle player click on TAP_TO_START
*/
void display_starting_screen()
{
    play_music(OPENING_MUSIC_DIRECTORY);
    bool game_started = false;
    bool quit = false;
    while (!quit && !game_started)
    {
        // Change color when mouse over TAP_TO_START
        int _x = 0, _y = 0;
        SDL_GetMouseState(&_x, &_y);
        win.draw_png_height_scaled(STARTING_SCREEN_DIRECTORY, 0, 0, WINDOW_WIDTH);
        win.draw_png(LOAD_BAR_DIRT_DIRECTORY, 204, 490, 642, 106);
        win.draw_png(LOAD_BAR_GRASS_DIRECTORY, 200, 455, 628, 66);
        for (auto &sprout : sprouts)
            sprout.display(INF);
        zom_head.display(INF);
        string loading = "CLICK TO START!";
        int w = 0, h = 0;
        TTF_SizeText(win.get_font(BRIANNE_TTF, 40), loading.c_str(), &w, &h);
        if (TAP_TO_START.is_mouse_in(_x, _y))
        {
            win.show_text_shadowed(loading, 204 + (642 - w) / 2, 490 + (106 - h) / 2, TAPPED, BRIANNE_TTF, 40, 1);
        }
        else
        {
            win.show_text_shadowed(loading, 204 + (642 - w) / 2, 490 + (106 - h) / 2, UNTAP, BRIANNE_TTF, 40, 1);
        }
        HANDLE_SDL_EVENT(
            QUIT();
            // Handle player click on TAP_TO_START
            LCLICK({
                if (TAP_TO_START.is_mouse_in(mouse_x, mouse_y))
                    game_started = true;
            });

        );

        win.update_screen();
    }
    win.fade_out();
    win.clear_renderer();
}

void display_logo()
{
    for (int clk = 0; clk < DISPLAY_LOGO_TIME; clk++)
    {
        int alpha = 510 * (clk <= (DISPLAY_LOGO_TIME / 2) ? clk : DISPLAY_LOGO_TIME - clk) / DISPLAY_LOGO_TIME;
        win.draw_bg(BLACK_SCREEN_DIRECTORY);
        // EA logo
        win.set_texture_alpha(EA_LOGO_DIRECTORY, alpha);
        win.draw_png_center(EA_LOGO_DIRECTORY, 1, 150);
        // Popcap logo
        win.set_texture_alpha(POPCAP_LOGO_DIRECTORY, alpha);
        win.draw_png_center(POPCAP_LOGO_DIRECTORY, 1, 600);
        // Nguyen Minh Hien Edition
        int w = 0, h = 0;
        auto edition = "NGUYEN MINH HIEN Edition";
        TTF_SizeText(win.get_font(HOUSEOFTERROR_TTF, 60), edition, &w, &h);
        win.show_text(edition, 204 + (642 - w) / 2, 490 + (106 - h) / 2, UNTAP, HOUSEOFTERROR_TTF, 60, alpha);
        // Update screen
        HANDLE_SDL_EVENT(
            QUIT(););
        win.update_screen();
    }
    const int FADE_IN_TIME = 30;
    for (int clk = 0; clk <= FADE_IN_TIME; clk++)
    {
        // Starting screen
        win.draw_bg(STARTING_SCREEN_DIRECTORY);
        // Fly in loading bar
        int dy = 490 + (WINDOW_HEIGHT - 490) * (FADE_IN_TIME - clk) / FADE_IN_TIME;
        win.draw_png(LOAD_BAR_DIRT_DIRECTORY, 204, dy, 642, 106);
        // Update screen
        HANDLE_SDL_EVENT(
            QUIT(););
        win.update_screen();
    }
}

/*
Display loading screen.
*/
void display_loading_screen()
{
    play_music(OPENING_MUSIC_DIRECTORY);
    display_logo();
    string loading = "LOADING.";
    for (int clk = 1; clk <= LOADING_TIME; clk++)
    {
        if (clk % 45 == 15)
            loading = "LOADING..";
        else if (clk % 45 == 30)
            loading = "LOADING...";
        else if (clk % 45 == 0)
            loading = "LOADING.";
        // Loading screen
        win.draw_png_height_scaled(STARTING_SCREEN_DIRECTORY, 0, 0, WINDOW_WIDTH);
        // Loading dirt
        win.draw_png(LOAD_BAR_DIRT_DIRECTORY, 204, 490, 642, 106);
        // Loading text
        int w = 0, h = 0;
        TTF_SizeText(win.get_font(BRIANNE_TTF, 40), loading.c_str(), &w, &h);
        win.show_text_shadowed(loading, 204 + (642 - w) / 2, 490 + (106 - h) / 2, UNTAP, BRIANNE_TTF, 40, 1);
        // Loading grass
        int dx = 628 * clk / LOADING_TIME;
        win.draw_png(LOAD_BAR_GRASS_DIRECTORY, 0, 0, dx / 2, 33, 200, 455, dx, 66);
        // Loading roll cap
        int pos = dx + 200;
        int cap_width = 75 + 50 * (LOADING_TIME - clk) / LOADING_TIME;
        win.draw_png(SOD_ROLL_CAP, pos - cap_width / 2, 515 - cap_width, cap_width, cap_width, 4 * clk);
        // Loading sprout & zombie head
        for (auto &sprout : sprouts)
            sprout.display(pos - cap_width / 2);
        zom_head.display(pos);

        HANDLE_SDL_EVENT(
            QUIT(););

        win.update_screen();
    }
}
