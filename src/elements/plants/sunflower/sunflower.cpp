#include "sunflower.hpp"
#include "elements/Map/Map.hpp"
#include "nlohmann/json.hpp"
#include "support.hpp"
#include "elements/Level/Level.hpp" // roof_tile_dy
using namespace nlohmann;

static ifstream sunflower("./resources/images/json/sunflower.json");
json sunflower_data = json::parse(sunflower);

auto SUNFLOWER_ASSET = PlantAssets(sunflower_data["frame"], sunflower_data["img_width"], sunflower_data["img_height"],
                                   sunflower_data["game_width"], sunflower_data["game_height"],
                                   sunflower_data["x_alias"], sunflower_data["y_alias"]);

extern int game_state;
extern Map cells;
extern Window win;

/*
Sunflower constructor
*/
Sunflower::Sunflower(const int &_row, const int &_col) : Plants(_row, _col)
{
    type = SUNFLOWER_TYPE;
    health = PLANT_HEALTH_LIMIT[SUNFLOWER_TYPE];
    // First sun 3-12.5 s after planting (original PvZ timing).
    sec_for_prepare = 180 + rand(0, 569);
    directory_num = SUNFLOWER_SHEET_DIRECTORY;
}

/*
Sunflower destructor
*/
Sunflower::~Sunflower()
{
}

/*
Change sunflower appearance: the glow-with-sun sheet leads the drop by
~2 s (one sheet cycle, switched under frame==0 wraps) so the flash plays as
the sun pops. The drop itself is timer-authoritative like the original's
mLaunchCounter (repvz Plant.cpp:1027) -- not gated on a sheet frame, which
used to stretch the interval to ~26 s instead of 23.5-25 s.
*/
const int SUNFLOWER_GLOW_LEAD = 120;

void Sunflower::determine_appearance()
{
    if (frame == 0)
        directory_num = sec_for_prepare <= SUNFLOWER_GLOW_LEAD ? SUNFLOWER_HAPPY_DIRECTORY
                                                               : SUNFLOWER_SHEET_DIRECTORY;
    if (frame >= SUNFLOWER_ASSET.frame * all_img[SUNFLOWER_SHEET_DIRECTORY].n_sheet)
    {
        frame = 0;
    }
}

/*Generate sun from a sunflower.
Update time to have next sun.
Init a new sun.
Then add it into vector<Sun>.
Re-arm the moment the counter runs out (repvz Plant.cpp:1027:
mLaunchCounter = RandRangeInt(2350, 2500) cs = 1410-1500 ticks at 60 tps).
*/
void Sunflower::gen_sun_from_a_sunflower(vector<Sun> &suns)
{
    // Update time to have next sun.
    --sec_for_prepare;
    determine_appearance();
    if (sec_for_prepare <= 0)
    {
        // Reset time to have next sun: 23.5-25 s.
        sec_for_prepare = 1410 + rand(0, 89);
        // Pop a sun out of the body (repvz COIN_MOTION_FROM_PLANT).
        spawn_sun_from_plant(suns, row, col, NORMAL_SUN_VALUE);
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
