#include "peashooter.hpp"
static ifstream peashooter("./image/json/peashooter.json");
json peashooter_data = json::parse(peashooter);

auto PEASHOOTER_ASSET = PlantAssets(peashooter_data["frame"], peashooter_data["img_width"], peashooter_data["img_height"],
                                    peashooter_data["game_width"], peashooter_data["game_height"],
                                    peashooter_data["x_alias"], peashooter_data["y_alias"]);

extern int game_state;
extern Map cells;
extern window win;

/*
Peashooter constructor
*/
Peashooter::Peashooter(const int &_row, const int &_col)
{
    type = PEASHOOTER_TYPE;
    row = _row;
    col = _col;
    health = PLANT_HEALTH_LIMIT[PEASHOOTER_TYPE];
    sec_for_prepare = 1;
    directory_num = PEASHOOTER_SHEET_DIRECTORY;
    frame = 0;
    attacked_time = 0;
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
void Peashooter::fire_pea(vector<Zombie *> &zombies, vector<Bullet *> &bullets)
{
    if (directory_num == PEASHOOTER_ATTACK_DIRECTORY && frame == 32 * PEASHOOTER_ASSET.frame)
    {
        play_sound_effect(FIRE_PEA_MUSIC_DIRECTORY);
        bullets.push_back(new Pea(row, cells[row][col].x2 - 10));
    }
    determine_appearance(zombies);
}

/*
Change peashooter's appearance if a peashooter need to attack or not.
(Peashooter is attack only if there are some zombies in the row.)
Updated: Zombie position > peashooter position
*/
void Peashooter::determine_appearance(vector<Zombie *> &zombies)
{
    // Attack
    for (const auto &zombie : zombies)
        if (row == zombie->row &&
            is_in(cells[0][col].x2 - 140, zombie->x_location, ZOMBIE_INIT_X - ZOMBIE_EXACT_LOCATION - 20))
        {
            if (directory_num == PEASHOOTER_SHEET_DIRECTORY)
            {
                if (frame == 0)
                {
                    directory_num = PEASHOOTER_ATTACK_DIRECTORY;
                }
            }
            return;
        }
    // No attack
    if (directory_num == PEASHOOTER_ATTACK_DIRECTORY)
    {
        if (frame == 0)
        {
            directory_num = PEASHOOTER_SHEET_DIRECTORY;
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
