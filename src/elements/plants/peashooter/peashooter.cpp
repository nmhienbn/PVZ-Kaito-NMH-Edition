#include "peashooter.hpp"
#include "elements/Map/Map.hpp"
#include "nlohmann/json.hpp"
using namespace nlohmann;
using namespace nlohmann;

static ifstream peashooter("./resources/images/json/peashooter.json");
json peashooter_data = json::parse(peashooter);

auto PEASHOOTER_ASSET = PlantAssets(peashooter_data["frame"], peashooter_data["img_width"], peashooter_data["img_height"],
                                    peashooter_data["game_width"], peashooter_data["game_height"],
                                    peashooter_data["x_alias"], peashooter_data["y_alias"]);

extern int game_state;
extern Map cells;
extern Window win;

/*
Peashooter constructor
*/
Peashooter::Peashooter(const int &_row, const int &_col) : Plants(_row, _col)
{
    type = PEASHOOTER_TYPE;
    health = PLANT_HEALTH_LIMIT[PEASHOOTER_TYPE];
    sec_for_prepare = 1;
    directory_num = PEASHOOTER_SHEET_DIRECTORY;
    attack_range = ONE_ROW_AHEAD;
}

/*
Peashooter destructor
*/
Peashooter::~Peashooter()
{
}

/*
Create new pea from a peashooter.
*/
void Peashooter::fire_pea(vector<Bullet *> &bullets)
{
    if (directory_num == PEASHOOTER_ATTACK_DIRECTORY && frame == 32 * PEASHOOTER_ASSET.frame)
    {
        play_sound_effect(FIRE_PEA_MUSIC_DIRECTORY);
        // repvz Plant::Fire: (mX + headX + 24, ...) -- headX ~= 16 puts the
        // muzzle at the cell's x1 + 40.
        bullets.push_back(new Pea(row, cells[row][col].x1 + 40));
    }
}

/*
Attack loop = one pea per 90 ticks: the original mLaunchRate for shooters is
150 cs (Plant.cpp gPlantDefs) = 90 ticks at 60 fps; wrap the attack reel there
instead of riding the full sheet.
*/
void Peashooter::next_frame(const PlantAssets &assets)
{
    next_frame_capped(assets, PEASHOOTER_ATTACK_DIRECTORY, 90);
}

/*
Change peashooter's appearance if a peashooter need to attack or not.
(Peashooter is attack only if there are some zombies in the row.)
Updated: Zombie position > peashooter position
*/
void Peashooter::determine_appearance(bool check_zombie_in_attack_range)
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
Display peashooters in row
*/
void Peashooter::display(const int &_row)
{
    if (row == _row)
        Plants::display(PEASHOOTER_ASSET);
}
