#include "player_name.h"
#define RENAME_MENU_WIDTH 677
#define RENAME_MENU_HEIGHT 400
#define RENAME_INPUT_X 75
#define RENAME_INPUT_Y 215

void update_player_name(Player &player, string new_name)
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
        if (player.unlocked_level < 1)
            player.unlocked_level = 1;
        outfile << player.name << '\n'
                << player.unlocked_level;
        outfile.close();
    }
    else
        cout << "Unable to open saved data file!\n";
}

void render_rename_bg(window &win, Player &player)
{
    win.clear_renderer();
    win.draw_png_scale(CHOOSE_LEVELS_DIRECTORY, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    display_button(win, RENAME_BUTTON, RENAME_DIRECTORY);
    display_button(win, RESET_LEVEL_BUTTON, RESET_LEVEL_DIRECTORY);
    display_button(win, QUIT_BUTTON, QUIT_DIRECTORY);

    for (int i = 1; i <= LEVEL_COUNT; i++)
    {
        if (player.unlocked_level >= i)
        {
            win.show_text("Level " + to_string(i), LEVEL_BUTTON[i].x1 + 60, LEVEL_BUTTON[i].y1 + 130);
        }
        else
        {
            win.show_text("Level " + to_string(i), LEVEL_BUTTON[i].x1 + 60, LEVEL_BUTTON[i].y1 + 130, BLACK);

            display_level_is_locked(win, LEVEL_BUTTON[i]);
        }
    }
}

void display_rename_player(window &win, Player &player, bool &quit)
{
    SDL_StartTextInput();

    // Event handler
    SDL_Event e;

    // The current input text.
    string inputText = player.name;

    // Enable text input
    SDL_StartTextInput();

    int RENAME_MENU_X = (WINDOW_WIDTH - RENAME_MENU_WIDTH) / 2;
    int RENAME_MENU_Y = (WINDOW_HEIGHT - RENAME_MENU_HEIGHT) / 2;
    const Button OK(RENAME_MENU_X + 45, RENAME_MENU_X + 325, RENAME_MENU_Y + 310, RENAME_MENU_Y + 370);
    const Button CANCEL(RENAME_MENU_X + 350, RENAME_MENU_X + 630, RENAME_MENU_Y + 310, RENAME_MENU_Y + 370);

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
                if (OK.is_mouse_in(mouse_x, mouse_y))
                {
                    update_player_name(player, inputText);
                    SDL_StopTextInput();
                    win.clear_renderer();
                    return;
                }
                if (CANCEL.is_mouse_in(mouse_x, mouse_y))
                {
                    SDL_StopTextInput();
                    win.clear_renderer();
                    return;
                }
            });
        }

        render_rename_bg(win, player);
        win.draw_png(BLACK_SCREEN_DIRECTORY, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
        win.draw_png(RENAME_MENU_DIRECTORY, RENAME_MENU_X, RENAME_MENU_Y, RENAME_MENU_WIDTH, RENAME_MENU_HEIGHT);

        // Text is not empty
        if (inputText != "")
        {
            // Render new text
            win.show_text(inputText.c_str(), RENAME_MENU_X + RENAME_INPUT_X, RENAME_MENU_Y + RENAME_INPUT_Y);
        }
        win.update_screen();
    }

    // Disable text input
    SDL_StopTextInput();
}

void display_new_player_name(window &win, Player &player, bool &quit)
{
    if (player.name != "")
        return;
    SDL_StartTextInput();

    // Event handler
    SDL_Event e;

    // The current input text.
    string inputText = "";

    // Enable text input
    SDL_StartTextInput();

    int RENAME_MENU_X = (WINDOW_WIDTH - RENAME_MENU_WIDTH) / 2;
    int RENAME_MENU_Y = (WINDOW_HEIGHT - RENAME_MENU_HEIGHT) / 2;
    const Button OK(RENAME_MENU_X + 45, RENAME_MENU_X + 630, RENAME_MENU_Y + 310, RENAME_MENU_Y + 370);

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
                if (OK.is_mouse_in(mouse_x, mouse_y))
                {
                    update_player_name(player, inputText);
                    SDL_StopTextInput();
                    win.clear_renderer();
                    return;
                }
            });
        }

        render_rename_bg(win, player);
        win.draw_png(BLACK_SCREEN_DIRECTORY, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
        win.draw_png(NEW_USER_DIRECTORY, RENAME_MENU_X, RENAME_MENU_Y, RENAME_MENU_WIDTH, RENAME_MENU_HEIGHT);

        // Text is not empty
        if (inputText != "")
        {
            // Render new text
            win.show_text(inputText.c_str(), RENAME_MENU_X + RENAME_INPUT_X, RENAME_MENU_Y + RENAME_INPUT_Y);
        }
        win.update_screen();
    }

    // Disable text input
    SDL_StopTextInput();
}
