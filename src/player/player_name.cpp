#include "player_name.hpp"
#include "draw/atlas_font.hpp"
#include "elements/Player/Player.hpp"
#include "events/choose_level_scene/render_scene/display_choose_level.hpp"
#include "events/in_game/pause_menu/pvz1_ui.hpp"
#include "music/music.hpp"

const int GET_NAME_MENU_WIDTH = 677;
const int GET_NAME_MENU_HEIGHT = 400;
const int GET_NAME_INPUT_X = 75;   // edit box art-local left edge + pad
const int GET_NAME_INPUT_CY = 228; // edit box vertical center (art-local)
/*PvZ1 NewUserDialog palette (repvz): the typed name sits in the edit box in
the LawnEditWidget ink (gLawnEditWidgetColors COLOR_TEXT), with the original
caret blink (mBlinkDelay 14); the OK/CANCEL GameButton labels are readable
Dwarvesc TTF (user request: buttons back to English -- green ink + dark
shadow, brightened on hover) via the shared draw_classic_dialog_button.*/
const int GET_NAME_INPUT_SIZE = 28;
const RGB NAME_INK(240, 240, 255);

#define GET_NAME_MENU_X (WINDOW_WIDTH - GET_NAME_MENU_WIDTH) / 2
#define GET_NAME_MENU_Y (WINDOW_HEIGHT - GET_NAME_MENU_HEIGHT) / 2
const Button OK(GET_NAME_MENU_X + 45, GET_NAME_MENU_X + 325, GET_NAME_MENU_Y + 310, GET_NAME_MENU_Y + 370);
const Button CANCEL(GET_NAME_MENU_X + 350, GET_NAME_MENU_X + 630, GET_NAME_MENU_Y + 310, GET_NAME_MENU_Y + 370);
const Button OK2(GET_NAME_MENU_X + 45, GET_NAME_MENU_X + 630, GET_NAME_MENU_Y + 310, GET_NAME_MENU_Y + 370);

extern bool quit;
extern Player player;
extern Window win;

/*
Change player's name in data file.
*/
void update_player_name(const string &new_name)
{
    // Read data
    string line;
    ifstream infile(SAVED_DATA_DIRECTORY);
    if (infile.is_open())
    {
        getline(infile, line);
        getline(infile, line);
        player.unlocked_level = stoi(line);
        infile.close();
    }
    else
        cout << "Unable to open saved data file!\n";

    // Write data
    ofstream outfile(SAVED_DATA_DIRECTORY);
    if (outfile.is_open())
    {
        player.name = new_name;
        if (player.unlocked_level < 0)
            player.unlocked_level = 0;
        outfile << player.name << '\n'
                << player.unlocked_level;
        outfile.close();
    }
    else
        cout << "Unable to open saved data file!\n";
}

/*
OK/CANCEL as the PvZ1 GameButton: Lawn button art + the real inset dwarven
label atlas (18 GreenInset, BrightGreenInset on hover), centered on the
button. Shared with the quit/reset dialogs through pvz1_ui.
*/
static void show_get_name_button(const Button &b, const string &text)
{
    draw_classic_dialog_button(b, text, pvz1_hot(b));
}

/*
One get-name dialog frame (split out of display_get_name_player so the
headless dialog dump can render it): the scene the dialog was opened from
behind (choose-level by default; the caller's `backdrop` when it passed one,
e.g. the selector/mode screen for the wood-sign rename), then dim,
the NEW USER/RENAME dialog art (its header + prompt line are baked into the
art), the typed name in Brianne icy-white inside the edit box with the
original's blinking caret, and the GameButtons.
*/
void draw_get_name_dialog(const int &get_name_dir, const string &inputText, int frame_clk,
                          SceneBackdrop backdrop)
{
    bool is_new_user = (get_name_dir == NEW_USER_DIRECTORY);
    // frame of the scene behind, no mouse-over handling
    (backdrop != nullptr ? backdrop : (SceneBackdrop)display_choose_level)(false);
    // dim
    win.draw_png(BLACK_SCREEN_DIRECTORY, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    win.draw_png(get_name_dir, GET_NAME_MENU_X, GET_NAME_MENU_Y, GET_NAME_MENU_WIDTH, GET_NAME_MENU_HEIGHT);
    if (is_new_user)
    {
        show_get_name_button(OK2, "OK");
    }
    else
    {
        show_get_name_button(OK, "OK");
        show_get_name_button(CANCEL, "CANCEL");
    }

    // Edit box text (LawnEditWidget: BRIANNETOD16, icy-white ink), vertically
    // centered in the box; blinking caret after the last character.
    int tw = 0, th = 0, tx = GET_NAME_MENU_X + GET_NAME_INPUT_X;
    TTF_SizeText(win.get_font(BRIANNE_TTF, GET_NAME_INPUT_SIZE), inputText.c_str(), &tw, &th);
    int ty = GET_NAME_MENU_Y + GET_NAME_INPUT_CY - th / 2;
    // Text is not empty
    if (inputText != "")
        win.show_text(inputText, tx, ty, NAME_INK, BRIANNE_TTF, GET_NAME_INPUT_SIZE);
    if ((frame_clk / 14) % 2 == 0)
        win.show_text("|", tx + tw + 1, ty, NAME_INK, BRIANNE_TTF, GET_NAME_INPUT_SIZE);
}

/*
Display player get_name menu:
    Handle quit event.
    Handle key down event:
        Backspace
        Ctrl + C
        Ctrl + V
        Enter
    If not those key down events, handle text input (Do not let text input length > 20).
    Handle mouse event:
        OK to rename.
        CANCEL to keep old name.
    OR
        OK to set name for new player
*/
void display_get_name_player(const int &get_name_dir, SceneBackdrop backdrop)
{
    bool is_new_user = (get_name_dir == NEW_USER_DIRECTORY);
    if (is_new_user && player.name != "")
        return;

    SDL_StartTextInput();
    // Event handler
    SDL_Event e;
    // The current input text.
    string inputText = player.name;
    // frame counter for the caret blink
    int frame_clk = 0;
    // Enable text input
    SDL_StartTextInput();
    // While application is running
    while (!quit)
    {
        // Handle events on queue
        while (SDL_PollEvent(&e) != 0)
        {
            // User requests quit
            if (e.type == SDL_QUIT)
            {
                quit = true;
            }
            // Special key input
            else if (e.type == SDL_KEYDOWN)
            {
                // Handle backspace
                if (e.key.keysym.sym == SDLK_BACKSPACE && inputText.length() > 0)
                {
                    // lop off character
                    inputText.pop_back();
                }
                // Handle copy
                else if (e.key.keysym.sym == SDLK_c && SDL_GetModState() & KMOD_CTRL)
                {
                    SDL_SetClipboardText(inputText.c_str());
                }
                // Handle paste
                else if (e.key.keysym.sym == SDLK_v && SDL_GetModState() & KMOD_CTRL)
                {
                    inputText = SDL_GetClipboardText();
                }
                // Handle Enter
                else if (e.key.keysym.sym == SDLK_RETURN)
                {
                    update_player_name(inputText);
                    SDL_StopTextInput();
                    win.clear_renderer();
                    return;
                }
            }
            // Special text input event
            else if (e.type == SDL_TEXTINPUT)
            {
                // Not copy or pasting
                if (!(SDL_GetModState() & KMOD_CTRL && (e.text.text[0] == 'c' || e.text.text[0] == 'C' || e.text.text[0] == 'v' || e.text.text[0] == 'V')))
                {
                    // Append character
                    inputText += e.text.text;
                }
            }
            // Mouse event
            LCLICK({
                if (is_new_user)
                {
                    if (OK2.is_mouse_in(mouse_x, mouse_y))
                    {
                        update_player_name(inputText);
                        SDL_StopTextInput();
                        win.clear_renderer();
                        return;
                    }
                }
                else
                {
                    if (OK.is_mouse_in(mouse_x, mouse_y))
                    {
                        update_player_name(inputText);
                        play_sound_effect(BUTTON_CLICK_MUSIC_DIRECTORY);
                        SDL_StopTextInput();
                        win.clear_renderer();
                        return;
                    }
                    if (CANCEL.is_mouse_in(mouse_x, mouse_y))
                    {
                        play_sound_effect(BUTTON_CLICK_MUSIC_DIRECTORY);
                        SDL_StopTextInput();
                        win.clear_renderer();
                        return;
                    }
                }
            });
        }
        // Text input length capped at 20 visible characters
        while (inputText.size() > 20)
            inputText.erase(inputText.begin());
        draw_get_name_dialog(get_name_dir, inputText, frame_clk, backdrop);
        frame_clk++;
        win.update_screen();
    }

    // Disable text input
    SDL_StopTextInput();
}