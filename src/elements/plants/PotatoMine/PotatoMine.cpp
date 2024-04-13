#include "PotatoMine.hpp"
#define POTATO_MINE_ARMED_TIME 900

extern const json plant_json;
static ifstream potatomine("./image/json/potatomine.json");
static ifstream potatomine_explosion("./image/json/potatomine_explosion.json");

json potatomine_data = json::parse(potatomine);
json potatomine_explosion_data = json::parse(potatomine_explosion);

auto POTATOMINE_ASSET = PlantAssets(potatomine_data["frame"], potatomine_data["img_width"], potatomine_data["img_height"],
                                    potatomine_data["game_width"], potatomine_data["game_height"],
                                    potatomine_data["x_alias"], potatomine_data["y_alias"]);

auto POTATOMINE_EXPLOSION_ASSET = PlantAssets(potatomine_explosion_data["frame"],
                                              potatomine_explosion_data["img_width"], potatomine_explosion_data["img_height"],
                                              potatomine_explosion_data["game_width"], potatomine_explosion_data["game_height"],
                                              potatomine_explosion_data["x_alias"], potatomine_explosion_data["y_alias"]);

extern int game_state;
extern Window win;

/*
Cherry Bomb constructor
*/
PotatoMine::PotatoMine(const int &_row, const int &_col) : Plants(_row, _col)
{
    type = POTATOMINE_TYPE;
    health = PLANT_HEALTH_LIMIT[POTATOMINE_TYPE];
    directory_num = POTATOMINE_UNARMED_DIRECTORY;
    attack_range = ONE_X_ONE;
    state = PLANTED;
    arm_count_down = POTATO_MINE_ARMED_TIME;
}

/*
Cherry Bomb Destructor
*/
PotatoMine::~PotatoMine()
{
}

/*
@return true if current frame cherry bomb can explode
*/
bool PotatoMine::is_blow()
{
    if (state == ATTACK && frame >= POTATOMINE_ASSET.frame * all_img[POTATOMINE_ATTACK_DIRECTORY].n_sheet)
    {
        directory_num = POTATOMINE_EXPLOSION_DIRECTORY;
        state = EXPLOSION;
        frame = 0;
        return true;
    }
    return false;
}

/*
@return if cherry bomb can now disappear
*/
void PotatoMine::disappear()
{
    if (state == EXPLOSION && frame >= POTATOMINE_EXPLOSION_ASSET.frame * all_img[POTATOMINE_EXPLOSION_DIRECTORY].n_sheet)
    {
        health = 0;
    }
}

/*
Display cherry bomb in row
*/
void PotatoMine::display(const int &_row)
{
    if (row == _row)
    {
        if (state != EXPLOSION)
            Plants::display(POTATOMINE_ASSET);
        else
            Plants::display(POTATOMINE_EXPLOSION_ASSET);
    }
}

void PotatoMine::next_frame(const PlantAssets &assets)
{
    if (check_status(game_state, IS_PAUSED) == false)
    {
        switch (state)
        {
        case PLANTED:
        {
            if (++frame >= assets.frame * all_img[directory_num].n_sheet)
            {
                --frame;
            }
            if (--arm_count_down <= 0)
            {
                state = READY_TO_ARM;
                health = INF;
                directory_num = POTATOMINE_RECOVER_DIRECTORY;
                frame = 0;
            }
            break;
        }
        case READY_TO_ARM:
        {
            if (++frame >= assets.frame * all_img[directory_num].n_sheet)
            {
                state = ARMED;
                frame = 0;
                directory_num = POTATOMINE_IDLE_DIRECTORY;
            }
            break;
        }
        case ARMED:
        {
            if (++frame >= assets.frame * all_img[directory_num].n_sheet)
            {
                frame = 0;
            }
            break;
        }
        case ATTACK:
        {
            ++frame;
            break;
        }
        case EXPLOSION:
        {
            ++frame;
            break;
        }

        default:
            break;
        }
    }
}