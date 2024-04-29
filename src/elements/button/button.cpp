#include "button.hpp"
#include "game_param.hpp"

extern Window win;

const int ICON_BAR_X1 = 20;
const int ICON_BAR_Y1 = 90;
const int ICON_BAR_WIDTH = 120;
const int ICON_WIDTH = 105;
const int ICON_HEIGHT = 65;

const int MENU_X1 = 325;
const int MENU_X2 = 725;
const int MENU_Y1 = 66;
const int MENU_Y2 = 534;

const int CONTINUE_WIDTH = 185;
const int CONTINUE_HEIGHT = 37;

const Button Shovel_bar(ICON_BAR_X1, ICON_BAR_X1 + 75, 500, 575);

const vector<Button> plant_seed = []()
{
    vector<Button> buttons;
    buttons.reserve(PLANT_COUNT);
    int y_start = ICON_BAR_Y1 + 1;

    for (int i = 0; i < PLANT_COUNT; ++i)
    {
        buttons.push_back({ICON_BAR_X1, ICON_BAR_X1 + ICON_WIDTH, y_start, y_start + ICON_HEIGHT});
        y_start += ICON_HEIGHT;
    }

    return buttons;
}();

const Button PAUSE_ICON(1000, 1045, 0, 45);
const Button TURBO_ICON(955, 1000, 0, 45);

const Button MENU(MENU_X1, MENU_X2, MENU_Y1, MENU_Y2);
const Button BACK_TO_GAME(MENU_X1 + 30, MENU_X1 + 376, MENU_Y1 + 366, MENU_Y1 + 464);
const Button MAIN_MENU(MENU_X1 + 105, MENU_X1 + 310, MENU_Y1 + 317, MENU_Y1 + 357);
const Button RESTART(MENU_X1 + 105, MENU_X1 + 310, MENU_Y1 + 274, MENU_Y1 + 314);

const Button CONTINUE((WINDOW_WIDTH - CONTINUE_WIDTH) / 2, (WINDOW_WIDTH + CONTINUE_WIDTH) / 2, 540, 540 + CONTINUE_HEIGHT);

const Button PREV_PAGE_BUTTON(15, 65, 125, 175);
const Button NEXT_PAGE_BUTTON(985, 1035, 125, 175);

// Music volume limit
const Button MUSIC_VOLUME(MENU_X1 + 194, MENU_X1 + 334, MENU_Y1 + 139, MENU_Y1 + 168);
// SFX volume limit
const Button SFX_VOLUME(MENU_X1 + 194, MENU_X1 + 334, MENU_Y1 + 168, MENU_Y1 + 197);

/*Button constructor*/
Button::Button()
{
}

/*Button constructor with rectangle coordination.*/
Button::Button(int _x1, int _x2, int _y1, int _y2)
{
    x1 = _x1;
    x2 = _x2;
    y1 = _y1;
    y2 = _y2;
}

/*Button destructor*/
Button::~Button()
{
}

/*
@return true if current mouse is over the button
*/
bool Button::is_mouse_in(int mouse_x, int mouse_y) const
{
    if (x1 < mouse_x && mouse_x < x2 &&
        y1 < mouse_y && mouse_y < y2)
        return true;
    return false;
}

/*
Display blink effect when mouse move over the button.
*/
void Button::blink() const
{
    int _x, _y;
    SDL_GetMouseState(&_x, &_y);
    if (is_mouse_in(_x, _y))
    {
        win.draw_png(BUTTON_BLINK_DIRECTORY, x1, y1, x2 - x1, y2 - y1);
    }
}

void Button::show_text(const string &text, const int &x_bias, const int &y_bias,
                       const int &font_size, const RGB &color, const RGB &shadow_color) const
{
    int w, h;
    TTF_SizeText(win.get_font(DWARVESC_TTF, font_size), text.c_str(), &w, &h);
    int x = (x2 + x1 - w) / 2 + x_bias;
    int y = (y2 + y1 - h) / 2 + y_bias;
    win.show_text_shadowed(text, x, y, color, DWARVESC_TTF, font_size, 0, 1, shadow_color);
}
/*
Display Game Buttons
*/
void Button::display(const int &button_directory, int angle, SDL_RendererFlip flip) const
{
    win.draw_png(button_directory, x1, y1, x2 - x1, y2 - y1, angle, flip);
}

/*
Display transparent black when level is locked
*/
void display_level_is_locked(const Button &button)
{
    win.draw_png_height_scaled(BLACK_SCREEN_DIRECTORY, button.x1, button.y1, button.x2 - button.x1);
}

void Button::display_with_text(const int &button_dir, const string &text,
                               const int &font_size, const RGB &color, const RGB &shadow_color) const
{
    display(button_dir);
    show_text(text, -3, -3, font_size, color, shadow_color);
    int _x, _y;
    SDL_GetMouseState(&_x, &_y);
    if (is_mouse_in(_x, _y))
        win.draw_png(blink_of(button_dir), x1, y1, x2 - x1, y2 - y1);
}
