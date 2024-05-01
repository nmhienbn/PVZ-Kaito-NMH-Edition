#include "SeedPacket.hpp"
#include "elements/button/button.hpp"

extern Window win;
const int level_unlock_new_plant[] = {1, 2, 4, 6, 8, 10};

PlantType SeedPacket::chosen_plant = PLANT_COUNT;

SeedPacket::SeedPacket(int plant_name)
{
    try
    {
        this->plant_name = static_cast<PlantType>(plant_name);
    }
    catch (const std::exception &e)
    {
        throw invalid_argument("Invalid plant name in function SeedPacket::SeedPacket(int plant_name)");
    }
    this->loading_time = PLANT_LOADING_TIME[plant_name];
    this->remaining_time = 0;
    sun_cost = PLANT_SUN_COST[plant_name];
}

void SeedPacket::reset_remaining_time()
{
    remaining_time = loading_time;
}

void SeedPacket::display(int x, int y, int player_sun_count)
{
    int plant_dir = PEASHOOTER_DIRECTORY + plant_name;
    // Seed packet
    win.draw_png_height_scaled(SEED_PACKET_DIRECTORY, x, y, ICON_WIDTH);

    // Plant on seed packet
    win.set_texture_alpha(plant_dir, 255);
    win.draw_png_width_scaled(plant_dir, x + 8, y + 2, ICON_HEIGHT - 3);
    win.set_texture_alpha(plant_dir, 200);

    // Sun tag
    win.draw_png_height_scaled(SUN_TAG_DIRECTORY, x + 58, y + 25, 41);
    win.draw_png_height_scaled(SEED_PACKET_BOT_BORDER_DIRECTORY, x, y + 60, ICON_WIDTH);

    // Not enough sun or is chosen
    if (player_sun_count < sun_cost || SeedPacket::chosen_plant == plant_name || remaining_time)
    {
        win.draw_png(BLACK_SCREEN_DIRECTORY, x, y, ICON_WIDTH, ICON_HEIGHT);
    }

    if (remaining_time == 0)
    {
        if (plant_name == chosen_plant)
            win.draw_png_height_scaled(SEED_CHOSEN_DIRECTORY, x, y, ICON_WIDTH);
    }
    else
    {
        // Remaining time
        win.draw_png(BLACK_SCREEN_DIRECTORY, x, y, ICON_WIDTH,
                     remaining_time * ICON_HEIGHT / loading_time);
    }

    int w, h;
    int fsize = 30;
    TTF_SizeText(win.get_font(PVZUI_TTF, fsize), to_string(sun_cost).c_str(), &w, &h);
    w = x + ICON_WIDTH - 4 - w;
    h = y + ICON_HEIGHT - h;
    win.show_text(to_string(sun_cost),
                  w + 2, h + 3, BLACK, PVZUI_TTF, fsize);
    win.show_text_outlined(to_string(sun_cost),
                           w, h, WHITE, PVZUI_TTF, fsize);
}

void SeedPacket::blink(int x, int y) const
{
    int _x, _y;
    SDL_GetMouseState(&_x, &_y);
    if (is_in(x, _x, x + ICON_WIDTH) && is_in(y, _y, y + ICON_HEIGHT))
    {
        win.draw_png(WHITE_SCREEN_DIRECTORY, x, y, ICON_WIDTH, ICON_HEIGHT);
    }
}