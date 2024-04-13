#include "wallnut.hpp"

static ifstream wallnut("./image/json/wallnut.json");
json wallnut_data = json::parse(wallnut);

auto WALLNUT_ASSET = PlantAssets(wallnut_data["frame"], wallnut_data["img_width"], wallnut_data["img_height"],
                                 wallnut_data["game_width"], wallnut_data["game_height"],
                                 wallnut_data["x_alias"], wallnut_data["y_alias"]);

extern int game_state;
extern Window win;

/*
Wall-nut constructor
*/
WallNut::WallNut(const int &_row, const int &_col) : Plants(_row, _col)
{
    type = WALLNUT_TYPE;
    health = PLANT_HEALTH_LIMIT[WALLNUT_TYPE];
    directory_num = WALLNUT_1_DIRECTORY;
}

/*
Wall-nut destructor
*/
WallNut::~WallNut()
{
}

/* Change wall-nut appearance
health <= (1/5) * health: 5
health <= (2/5) * health: 4
health <= (3/5) * health: 3
health <= (4/5) * health: 2
health <= (5/5) * health: 1
*/
void WallNut::determine_appearance()
{
    for (int i = 1; i <= 5; i++)
    {
        if (health <= PLANT_HEALTH_LIMIT[WALLNUT_TYPE] * i / 5)
        {
            directory_num = WALLNUT_1_DIRECTORY + 5 - i;
            frame = 0;
            return;
        }
    }
}

/*
Display wallnut in row
*/
void WallNut::display(const int &_row)
{
    if (row == _row)
    {
        determine_appearance();
        Plants::display(WALLNUT_ASSET);
    }
}
