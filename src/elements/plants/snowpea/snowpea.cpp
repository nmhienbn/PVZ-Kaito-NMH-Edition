#include "snowpea.hpp"

static ifstream f("./image/json/snowpea.json");
json snowpea_data = json::parse(f);

auto SNOWPEA_ASSET = PlantAssets(snowpea_data["frame"], snowpea_data["img_width"], snowpea_data["img_height"],
                                 snowpea_data["game_width"], snowpea_data["game_height"],
                                 snowpea_data["x_alias"], snowpea_data["y_alias"]);

extern int game_state;
extern Map cells;
extern window win;

/*
Snow Pea constructor
*/
Snowpea::Snowpea(const int &_row, const int &_col)
{
    type = SNOWPEA_TYPE;
    row = _row;
    col = _col;
    health = PLANT_HEALTH_LIMIT[SNOWPEA_TYPE];
    sec_for_prepare = 1;
    directory_num = SNOWPEA_SHEET_DIRECTORY;
    frame = 0;
    attacked_time = 0;
}

/*
Snow Pea destructor
*/
Snowpea::~Snowpea()
{
}

/*
Create new pea from a snow pea.
*/
void Snowpea::fire_pea(vector<Zombie *> &zombies, vector<Pea> &peas)
{
    if (directory_num == SNOWPEA_ATTACK_DIRECTORY && frame == 17 * SNOWPEA_ASSET.frame)
    {
        play_sound_effect(FIRE_PEA_MUSIC_DIRECTORY);
        Pea temp(2, row, cells[row][col].x2 - 10);
        peas.push_back(temp);
    }
    determine_appearance(zombies);
}

/*
Change snow pea's appearance if a snow pea need to attack or not.
(Snow Pea is attack only if there are some zombies in the row.)
Updated: Zombie position > snow pea position
*/
void Snowpea::determine_appearance(vector<Zombie *> &zombies)
{
    // Attack
    for (const auto &zombie : zombies)
        if (row == zombie->row &&
            is_in(cells[0][col].x2 - 140, zombie->x_location, ZOMBIE_INIT_X - ZOMBIE_EXACT_LOCATION - 20))
        {
            if (directory_num == SNOWPEA_SHEET_DIRECTORY)
            {
                if (frame == 0)
                {
                    directory_num = SNOWPEA_ATTACK_DIRECTORY;
                }
            }
            return;
        }
    // No attack
    if (directory_num == SNOWPEA_ATTACK_DIRECTORY)
    {
        if (frame == 0)
        {
            directory_num = SNOWPEA_SHEET_DIRECTORY;
        }
    }
}

/*
Display snow pea in row
*/
void Snowpea::display(const int &_row)
{
    if (row == _row)
        Plants::display(SNOWPEA_ASSET);
}
