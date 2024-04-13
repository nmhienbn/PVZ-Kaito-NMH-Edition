#include "snowpea.hpp"

static ifstream snowpea("./image/json/snowpea.json");
json snowpea_data = json::parse(snowpea);

auto SNOWPEA_ASSET = PlantAssets(snowpea_data["frame"], snowpea_data["img_width"], snowpea_data["img_height"],
                                 snowpea_data["game_width"], snowpea_data["game_height"],
                                 snowpea_data["x_alias"], snowpea_data["y_alias"]);

extern int game_state;
extern Map cells;
extern Window win;

/*
Snow Pea constructor
*/
SnowPea::SnowPea(const int &_row, const int &_col) : Plants(_row, _col)
{
    type = SNOWPEA_TYPE;
    health = PLANT_HEALTH_LIMIT[SNOWPEA_TYPE];
    sec_for_prepare = 1;
    directory_num = SNOWPEA_SHEET_DIRECTORY;
    attack_range = ONE_ROW_AHEAD;
}

/*
Snow Pea destructor
*/
SnowPea::~SnowPea()
{
}

/*
Create new pea from a snow pea.
*/
void SnowPea::fire_pea(vector<Bullet *> &bullets)
{
    if (directory_num == SNOWPEA_ATTACK_DIRECTORY && frame == 17 * SNOWPEA_ASSET.frame)
    {
        play_sound_effect(FIRE_PEA_MUSIC_DIRECTORY);
        bullets.push_back(new SnowzPea(row, cells[row][col].x2 - 10));
    }
}

/*
Change snow pea's appearance if a snow pea need to attack or not.
(Snow Pea is attack only if there are some zombies in the row.)
Updated: Zombie position > snow pea position
*/
void SnowPea::determine_appearance(bool check_zombie_in_attack_range)
{
    // Attack
    if (check_zombie_in_attack_range)
    {
        if (directory_num == SNOWPEA_SHEET_DIRECTORY)
        {
            if (frame == 0)
            {
                directory_num = SNOWPEA_ATTACK_DIRECTORY;
            }
        }
    }
    // No attack
    else
    {
        if (directory_num == SNOWPEA_ATTACK_DIRECTORY)
        {
            {
                directory_num = SNOWPEA_SHEET_DIRECTORY;
            }
        }
    }
}

/*
Display snow pea in row
*/
void SnowPea::display(const int &_row)
{
    if (row == _row)
        Plants::display(SNOWPEA_ASSET);
}
