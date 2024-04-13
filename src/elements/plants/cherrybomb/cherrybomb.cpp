#include "cherrybomb.hpp"

static ifstream cherrybomb("./image/json/cherrybomb.json");
static ifstream cherrybomb_explosion("./image/json/cherrybomb_explosion.json");

json cherrybomb_data = json::parse(cherrybomb);
json cherrybomb_explosion_data = json::parse(cherrybomb_explosion);

auto CHERRYBOMB_ASSET = PlantAssets(cherrybomb_data["frame"], cherrybomb_data["img_width"], cherrybomb_data["img_height"],
                                    cherrybomb_data["game_width"], cherrybomb_data["game_height"],
                                    cherrybomb_data["x_alias"], cherrybomb_data["y_alias"]);

auto CHERRYBOMB_EXPLOSION_ASSET = PlantAssets(cherrybomb_explosion_data["frame"],
                                              cherrybomb_explosion_data["img_width"], cherrybomb_explosion_data["img_height"],
                                              cherrybomb_explosion_data["game_width"], cherrybomb_explosion_data["game_height"],
                                              cherrybomb_explosion_data["x_alias"], cherrybomb_explosion_data["y_alias"]);

extern int game_state;
extern Window win;

/*
Cherry Bomb constructor
*/
CherryBomb::CherryBomb(const int &_row, const int &_col) : Plants(_row, _col)
{
    type = CHERRYBOMB_TYPE;
    health = PLANT_HEALTH_LIMIT[CHERRYBOMB_TYPE];
    directory_num = CHERRYBOMB_SHEET_DIRECTORY;
    attack_range = THREE_X_THREE;
    has_exploded = false;
}

/*
Cherry Bomb Destructor
*/
CherryBomb::~CherryBomb()
{
}

/*
@return true if current frame cherry bomb can explode
*/
bool CherryBomb::is_blow()
{
    if (!has_exploded && frame >= CHERRYBOMB_ASSET.frame * all_img[CHERRYBOMB_SHEET_DIRECTORY].n_sheet)
    {
        directory_num = CHERRYBOMB_EXPLOSION_DIRECTORY;
        has_exploded = true;
        frame = 0;
        return true;
    }
    return false;
}

/*
@return if cherry bomb can now disappear
*/
void CherryBomb::disappear()
{
    if (has_exploded && frame >= CHERRYBOMB_EXPLOSION_ASSET.frame * all_img[CHERRYBOMB_EXPLOSION_DIRECTORY].n_sheet)
    {
        health = 0;
    }
}

/*
Display cherry bomb in row
*/
void CherryBomb::display(const int &_row)
{
    if (row == _row)
    {
        if (!has_exploded)
            Plants::display(CHERRYBOMB_ASSET);
        else
            Plants::display(CHERRYBOMB_EXPLOSION_ASSET);
    }
}

void CherryBomb::next_frame(const PlantAssets &assets)
{
    if (check_status(game_state, IS_PAUSED) == false)
        ++frame;
}