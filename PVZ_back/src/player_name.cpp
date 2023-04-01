#include "player_name.h"
#define GET_NAME_MENU_WIDTH 677
#define GET_NAME_MENU_HEIGHT 400
#define GET_NAME_INPUT_X 75
#define GET_NAME_INPUT_Y 215

const int GET_NAME_MENU_X = (WINDOW_WIDTH - GET_NAME_MENU_WIDTH) / 2;
const int GET_NAME_MENU_Y = (WINDOW_HEIGHT - GET_NAME_MENU_HEIGHT) / 2;
const Button OK(GET_NAME_MENU_X + 45, GET_NAME_MENU_X + 325, GET_NAME_MENU_Y + 310, GET_NAME_MENU_Y + 370);
const Button CANCEL(GET_NAME_MENU_X + 350, GET_NAME_MENU_X + 630, GET_NAME_MENU_Y + 310, GET_NAME_MENU_Y + 370);
const Button OK2(GET_NAME_MENU_X + 45, GET_NAME_MENU_X + 630, GET_NAME_MENU_Y + 310, GET_NAME_MENU_Y + 370);

void display_get_name_player(window &win, Player &player, bool &quit, const int &get_name_dir)
{
    bool is_new_user = (get_name_dir == NEW_USER_DIRECTORY);
    if (is_new_user && player.name != "")
        return;
    SDL_StartTextInput();

    // Event handler
    SDL_Event e;

    // The current input text.
    string inputText = player.name;

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
                    update_player_name(player, inputText);
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
                    if (inputText.size() > 10)
                        inputText.erase(inputText.begin());
                }
            }
            KEY_PRESS(q, quit = true);
            LCLICK({
                if (is_new_user)
                {
                    if (OK2.is_mouse_in(mouse_x, mouse_y))
                    {
                        update_player_name(player, inputText);
                        SDL_StopTextInput();
                        win.clear_renderer();
                        return;
                    }
                }
                else
                {
                    if (OK.is_mouse_in(mouse_x, mouse_y))
                    {
                        update_player_name(player, inputText);
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

        render_choose_level_no_mouse(win, player);
        win.draw_png(BLACK_SCREEN_DIRECTORY, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
        win.draw_png(get_name_dir, GET_NAME_MENU_X, GET_NAME_MENU_Y, GET_NAME_MENU_WIDTH, GET_NAME_MENU_HEIGHT);
        if (is_new_user)
        {
            OK2.blink(win);
        }
        else
        {
            OK.blink(win);
            CANCEL.blink(win);
        }

        // Text is not empty
        if (inputText != "")
        {
            // Render new text
            win.show_text(inputText.c_str(), GET_NAME_MENU_X + GET_NAME_INPUT_X, GET_NAME_MENU_Y + GET_NAME_INPUT_Y);
        }
        win.update_screen();
    }

    // Disable text input
    SDL_StopTextInput();
}