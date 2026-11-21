#include "repeater.hpp"
#include "elements/Map/Map.hpp"
#include "nlohmann/json.hpp"
using namespace nlohmann;

// Same body art as the peashooter (the repeater look comes from the reanim
// overlay parts in reanim_plant.cpp), but an own copy of the sheet timing.
static ifstream repeater("./resources/images/json/repeater.json");
json repeater_data = json::parse(repeater);

auto REPEATER_ASSET = PlantAssets(repeater_data["frame"], repeater_data["img_width"], repeater_data["img_height"],
                                  repeater_data["game_width"], repeater_data["game_height"],
                                  repeater_data["x_alias"], repeater_data["y_alias"]);

extern int game_state;
extern Map cells;
extern Window win;

/*
Repeater constructor
*/
Repeater::Repeater(const int &_row, const int &_col) : Plants(_row, _col)
{
    type = REPEATER_TYPE;
    health = PLANT_HEALTH_LIMIT[REPEATER_TYPE];
    sec_for_prepare = 1;
    directory_num = PEASHOOTER_SHEET_DIRECTORY;
    attack_range = ONE_ROW_AHEAD;
}

/*
Repeater destructor
*/
Repeater::~Repeater()
{
}

/*
Create two peas from a repeater: the second spit lags the first by ~16 ticks
(GOTY anim_shooting plays faster and lands two FIRE_PEA mouths).
*/
void Repeater::fire_pea(vector<Bullet *> &bullets)
{
    if (directory_num == PEASHOOTER_ATTACK_DIRECTORY &&
        (frame == 32 * REPEATER_ASSET.frame || frame == 40 * REPEATER_ASSET.frame))
    {
        play_sound_effect(FIRE_PEA_MUSIC_DIRECTORY);
        bullets.push_back(new Pea(row, cells[row][col].x2 - 10));
    }
}

/*
Attack loop = one volley per 90 ticks (original mLaunchRate 150 cs for
shooters); wrap the attack reel there instead of riding the full sheet.
*/
void Repeater::next_frame(const PlantAssets &assets)
{
    next_frame_capped(assets, PEASHOOTER_ATTACK_DIRECTORY, 90);
}

/*
Change repeater's appearance if a repeater need to attack or not.
(Repeater is attack only if there are some zombies in the row.)
Updated: Zombie position > repeater position
*/
void Repeater::determine_appearance(bool check_zombie_in_attack_range)
{
    // Attack
    if (check_zombie_in_attack_range)
    {
        if (directory_num == PEASHOOTER_SHEET_DIRECTORY)
        {
            if (frame == 0)
            {
                directory_num = PEASHOOTER_ATTACK_DIRECTORY;
            }
        }
    }
    // No attack
    else
    {
        if (directory_num == PEASHOOTER_ATTACK_DIRECTORY)
        {
            if (frame == 0)
            {
                directory_num = PEASHOOTER_SHEET_DIRECTORY;
            }
        }
    }
}

/*
Display repeaters in row
*/
void Repeater::display(const int &_row)
{
    if (row == _row)
        Plants::display(REPEATER_ASSET);
}
