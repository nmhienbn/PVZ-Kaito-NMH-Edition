#include "cherrybomb.hpp"
static ifstream cherrybomb("./image/json/cherrybomb.json");
json cherrybomb_data = json::parse(cherrybomb);

auto CHERRYBOMB_ASSET = PlantAssets(cherrybomb_data["frame"], cherrybomb_data["img_width"], cherrybomb_data["img_height"],
                                    cherrybomb_data["game_width"], cherrybomb_data["game_height"],
                                    cherrybomb_data["x_alias"], cherrybomb_data["y_alias"]);

extern int game_state;
extern Map cells;
extern window win;

/*
Cherry Bomb constructor
*/
CherryBomb::CherryBomb(const int &_row, const int &_col)
{
    type = CHERRYBOMB_TYPE;
    row = _row;
    col = _col;
    health = PLANT_HEALTH_LIMIT[CHERRYBOMB_TYPE];
    directory_num = CHERRYBOMB_SHEET_DIRECTORY;
    frame = 0;
    attacked_time = 0;
}

/*
Cherry Bomb Destructor
*/
CherryBomb::~CherryBomb()
{
}
/*
@return 'true' if cherrybomb hit the zombie
*/

/*
@return true if Cherry Bomb can hit the zombie
*/
bool CherryBomb::is_hit_zombie(Zombie &zombie)
{
    int right_limit = cells[row][col].x2 + BLOCK_WIDTH;
    int left_limit = cells[row][col].x1 - BLOCK_WIDTH;
    if (is_in(row - 1, zombie.row, row + 1) &&
        is_in(left_limit, zombie.x_location + ZOMBIE_EXACT_LOCATION + 15, right_limit))
        return true;
    return false;
}

/*
@return true if current frame cherry bomb can explode
*/
bool CherryBomb::is_blow()
{
    return frame == CHERRYBOMB_ASSET.frame * 22;
}

/*
@return if cherry bomb can now disappear
*/
void CherryBomb::disappear()
{
    if (frame >= CHERRYBOMB_ASSET.frame * all_img[CHERRYBOMB_SHEET_DIRECTORY].n_sheet)
    {
        health = 0;
    }
}

/*
Make cherry bomb hit all the zombies of area 3x3
*/
void CherryBomb::hit_all_zombies(vector<Zombie *> &zombies,
                                 vector<ZombiePart> &zombie_parts)
{
    if (is_blow())
    {
        play_sound_effect(CHERRYBOMB_MUSIC_DIRECTORY);
        for (int j = 0; j < (int)zombies.size();)
            if (!apply_hitting_zombie(zombies, j, zombie_parts))
            {
                j++;
            }
    }
}

/*
Apply cherrybomb explode the zombie. (change zombie into burnt one)
*/
bool CherryBomb::apply_hitting_zombie(vector<Zombie *> &zombies, const int &z_ind,
                                      vector<ZombiePart> &zombie_parts)
{
    if (is_hit_zombie(*zombies[z_ind]))
    {
        zombie_parts.push_back(ZombiePart(ZOMBIE_BURNT_DIRECTORY, ZOMBIE_DIE_FRAME,
                                          zombies[z_ind]->row, zombies[z_ind]->x_location,
                                          ZOMBIE_WIDTH, ZOMBIE_HEIGHT));
        delete zombies[z_ind];
        zombies.erase(zombies.begin() + z_ind);
        return true;
    }
    return false;
}

/*
Display cherry bomb in row
*/
void CherryBomb::display(const int &_row)
{
    if (row == _row)
        Plants::display(CHERRYBOMB_ASSET);
}

void CherryBomb::next_frame(const PlantAssets &assets)
{
    if (check_status(game_state, IS_PAUSED) == false)
        ++frame;
}