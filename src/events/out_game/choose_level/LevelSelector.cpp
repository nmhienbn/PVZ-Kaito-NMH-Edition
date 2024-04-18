#include "LevelSelector.hpp"
#include <fstream>
#include <nlohmann/json.hpp>
using namespace nlohmann;

extern Window win;

static const int selector_width = 139;

static const map<string, int> zombie_type_map = {
    {"normal", NORMAL_DIRECTORY},
    {"flag", FLAG_DIRECTORY},
    {"conehead", CONE_DIRECTORY},
    {"buckethead", BUCKET_DIRECTORY},
    {"door", DOOR_DIRECTORY}};

static const map<string, int> plant_type_map = {
    {"peashooter", PEASHOOTER_DIRECTORY},
    {"sunflower", SUNFLOWER_DIRECTORY},
    {"wallnut", WALLNUT_DIRECTORY},
    {"snowpea", SNOWPEA_DIRECTORY},
    {"potato_mine", POTATOMINE_DIRECTORY},
    {"cherrybomb", CHERRYBOMB_DIRECTORY}};

int LevelSelector::BACKGROUND_CHOOSE_LEVEL = CHOOSE_LEVELS_1_DIRECTORY;
int LevelSelector::TYPE_LEVEL = DAY_LIT_DIRECTORY;

LevelSelector::LevelSelector(int level_id)
{
    auto j = get_lv_json(level_id);
    this->bg_dir = (j["objects"]["map_type"] <= 3 ? THUMB_BG_DAY_DIRECTORY : THUMB_BG_NIGHT_DIRECTORY);
    this->display_zombie = zombie_type_map.at(j["objects"]["display_zombie"]);
    if (j["objects"].find("unlocked_plant") != j["objects"].end())
    {
        this->display_plant = plant_type_map.at(j["objects"]["unlocked_plant"]);
    }
    else
    {
        this->display_plant = -1;
    }

    this->level_name = j["#comment"];
}

void LevelSelector::display(int x, int y, RGB color)
{
    win.draw_png_height_scaled(bg_dir, x + 5, y + 10, 128);
    win.draw_png_width_scaled(display_zombie, x + 55, y + 20, 70);
    win.draw_png_height_scaled(SELECTOR_DIRECTORY, x, y, selector_width);
    if (display_plant != -1)
    {
        win.draw_png_width_scaled(CONTAINER_DIRECTORY, x + 45, y + 110, 50);
        win.set_texture_alpha(display_plant, 255);
        win.draw_png_width_scaled(display_plant, x + 45, y + 110, 50);
        win.set_texture_alpha(display_plant, 120);
        int w, h;
        TTF_SizeText(win.get_font(PVZUI_TTF, 30), level_name.c_str(), &w, &h);
        int kc = (selector_width - w) / 2;
        win.show_text_shadowed(level_name, x + kc, y + 155, color, PVZUI_TTF, 30);
    }
    else
    {
        int w, h;
        TTF_SizeText(win.get_font(PVZUI_TTF, 40), level_name.c_str(), &w, &h);
        int kc = (selector_width - w) / 2;
        win.show_text_outlined(level_name, x + kc, y + 130, color, PVZUI_TTF, 40);
    }
}

void LevelSelector::display_locked(int x, int y)
{
    win.set_texture_color(SELECTOR_BLINK_DIRECTORY, 99, 99, 132);
    win.set_texture_alpha(SELECTOR_BLINK_DIRECTORY, 180);
    win.draw_png_height_scaled(SELECTOR_BLINK_DIRECTORY, x, y, selector_width);
    win.draw_png(LOCK_DIRECTORY, x + selector_width - 25, y - 10, 35, 44);
}

void LevelSelector::display_blink(int x, int y)
{
    win.set_texture_color(SELECTOR_BLINK_DIRECTORY, 255, 255, 255);
    win.set_texture_alpha(SELECTOR_BLINK_DIRECTORY, 70);
    win.draw_png_height_scaled(SELECTOR_BLINK_DIRECTORY, x, y, selector_width);
}

void LevelSelector::display_complete(int x, int y)
{
    win.draw_png(COMPLETED_DIRECTORY, x + selector_width - 20, y - 10, 35, 28);
}

bool LevelSelector::is_mouse_in(int mouse_x, int mouse_y, int x, int y)
{
    return (mouse_x >= x && mouse_x <= x + selector_width && mouse_y >= y && mouse_y <= y + 200);
}
