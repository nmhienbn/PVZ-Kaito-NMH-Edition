#include "walnut.hpp"

static ifstream f("./image/json/walnut.json");
json walnut_data = json::parse(f);

auto WALNUT_ASSET = PlantAssets(walnut_data["frame"], walnut_data["img_width"], walnut_data["img_height"],
                                walnut_data["game_width"], walnut_data["game_height"],
                                walnut_data["x_alias"], walnut_data["y_alias"]);

extern int game_state;
extern Map cells;
extern window win;

/*
Wall-nut constructor
*/
Walnut::Walnut(const int &_row, const int &_col)
{
    type = WALNUT_TYPE;
    row = _row;
    col = _col;
    health = PLANT_HEALTH_LIMIT[WALNUT_TYPE];
    directory_num = WALNUT_1_DIRECTORY;
    frame = 0;
    attacked_time = 0;
}

/*
Wall-nut destructor
*/
Walnut::~Walnut()
{
}

/* Change wall-nut appearance
health <= (1/5) * health: 5
health <= (2/5) * health: 4
health <= (3/5) * health: 3
health <= (4/5) * health: 2
health <= (5/5) * health: 1
*/
void Walnut::determine_appearance()
{
    for (int i = 1; i <= 5; i++)
    {
        if (health <= PLANT_HEALTH_LIMIT[WALNUT_TYPE] * i / 5)
        {
            directory_num = WALNUT_1_DIRECTORY + 5 - i;
            if (frame >= WALNUT_ASSET.frame * all_img[directory_num].n_sheet)
            {
                frame = 0;
            }
            return;
        }
    }
}

/*
Display walnut in row
*/
void Walnut::display(const int &_row)
{
    if (row == _row)
    {
        determine_appearance();
        Plants::display(WALNUT_ASSET);
    }
}
