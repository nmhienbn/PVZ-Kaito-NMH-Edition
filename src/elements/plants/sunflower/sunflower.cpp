#include "sunflower.hpp"

static ifstream sunflower("./image/json/sunflower.json");
json sunflower_data = json::parse(sunflower);

auto SUNFLOWER_ASSET = PlantAssets(sunflower_data["frame"], sunflower_data["img_width"], sunflower_data["img_height"],
                                   sunflower_data["game_width"], sunflower_data["game_height"],
                                   sunflower_data["x_alias"], sunflower_data["y_alias"]);

extern int game_state;
extern Map cells;
extern window win;

/*
Sunflower constructor
*/
Sunflower::Sunflower(const int &_row, const int &_col)
{
    type = SUNFLOWER_TYPE;
    row = _row;
    col = _col;
    health = PLANT_HEALTH_LIMIT[SUNFLOWER_TYPE];
    sec_for_prepare = SUN_GEN_SUNFLOWER_CLK_COUNT;
    directory_num = SUNFLOWER_SHEET_DIRECTORY;
    frame = 0;
    attacked_time = 0;
}

/*
Sunflower destructor
*/
Sunflower::~Sunflower()
{
}

/*
@return true if sunflower gen a new sun.
*/
bool Sunflower::is_gen_sun()
{
    return directory_num == SUNFLOWER_HAPPY_DIRECTORY && frame == 30 * SUNFLOWER_ASSET.frame;
}

/*
Change sunflower appearance when gen a new sun or normal
*/
void Sunflower::determine_appearance()
{
    // Gen new sun
    if (sec_for_prepare <= 0)
    {
        if (directory_num == SUNFLOWER_SHEET_DIRECTORY)
        {
            if (frame == 0)
            {
                directory_num = SUNFLOWER_HAPPY_DIRECTORY;
            }
        }
    }
    // Normal
    else
    {
        if (directory_num == SUNFLOWER_HAPPY_DIRECTORY)
        {
            if (frame == 0)
            {
                directory_num = SUNFLOWER_SHEET_DIRECTORY;
            }
        }
    }
    if (frame >= SUNFLOWER_ASSET.frame * all_img[SUNFLOWER_SHEET_DIRECTORY].n_sheet)
    {
        frame = 0;
    }
}

/*Generate sun from a sunflower.
Update time to have next sun.
Init a new sun.
Then add it into vector<Sun>.
*/
void Sunflower::gen_sun_from_a_sunflower(vector<Sun> &suns)
{
    // Update time to have next sun.
    --sec_for_prepare;
    determine_appearance();
    if (sec_for_prepare <= 0)
    {
        if (is_gen_sun())
        {
            // Reset time to have next sun.
            sec_for_prepare = SUN_GEN_SUNFLOWER_CLK_COUNT;
            // Init a new sun.
            Sun temp;
            temp.final_col = col;
            temp.final_row = row;
            temp.y_location = cells[row][col].y1 - 20;
            temp.wait_seconds = 0;
            // Then add it into vector<Sun>.
            suns.push_back(temp);
        }
    }
}

/*
Display sunflower in row
*/
void Sunflower::display(const int &_row)
{
    if (row == _row)
        Plants::display(SUNFLOWER_ASSET);
}
