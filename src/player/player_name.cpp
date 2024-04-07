#include "player_name.hpp"

const int GET_NAME_MENU_WIDTH = 677;
const int GET_NAME_MENU_HEIGHT = 400;
const int GET_NAME_INPUT_X = 75;
const int GET_NAME_INPUT_Y = 208;

#define GET_NAME_MENU_X (WINDOW_WIDTH - GET_NAME_MENU_WIDTH) / 2
#define GET_NAME_MENU_Y (WINDOW_HEIGHT - GET_NAME_MENU_HEIGHT) / 2
const Button OK(GET_NAME_MENU_X + 45, GET_NAME_MENU_X + 325, GET_NAME_MENU_Y + 310, GET_NAME_MENU_Y + 370);
const Button CANCEL(GET_NAME_MENU_X + 350, GET_NAME_MENU_X + 630, GET_NAME_MENU_Y + 310, GET_NAME_MENU_Y + 370);
const Button OK2(GET_NAME_MENU_X + 45, GET_NAME_MENU_X + 630, GET_NAME_MENU_Y + 310, GET_NAME_MENU_Y + 370);

extern bool quit;
extern Player player;
extern window win;

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
void display_get_name_player(const int &get_name_dir)
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
        // display choosing level screen that not handle mouse over
        display_choose_level(false);
        // dim
        win.draw_png(BLACK_SCREEN_DIRECTORY, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
        win.draw_png(get_name_dir, GET_NAME_MENU_X, GET_NAME_MENU_Y, GET_NAME_MENU_WIDTH, GET_NAME_MENU_HEIGHT);
        if (is_new_user)
        {
            OK2.blink();
        }
        else
        {
            OK.blink();
            CANCEL.blink();
        }

        // Text is not empty
        if (inputText != "")
        {
            while (inputText.size() > 20)
                inputText.erase(inputText.begin());
            // Render new text
            win.show_text(inputText.c_str(), GET_NAME_MENU_X + GET_NAME_INPUT_X, GET_NAME_MENU_Y + GET_NAME_INPUT_Y, WHITE, HOUSEOFTERROR_TTF, 30);
        }
        win.update_screen();
    }

    // Disable text input
    SDL_StopTextInput();
}