#include "plants.hpp"
#include "draw/rsdl.hpp"
#include "elements/Map/Map.hpp"
#include "elements/zombies/zombie_struct.hpp"
#include "music/music.hpp"

extern int game_state;
extern Map cells;
extern Window win;

/*
Plant constructor
*/
Plants::Plants()
{
}

/*
Plant destructor
*/
Plants::~Plants()
{
}

Plants::Plants(const int &_row, const int &_col)
{
    row = _row;
    col = _col;
    attacked_time = 0;
    frame = 0;
    attack_range = NO_ATTACK;
}

/*
Plant display: nothing
*/
void Plants::display(const int &_row)
{
}

/*
Plant geter
@return plant's type
*/
int Plants::get_type() const
{
    return type;
}

/*
Plant action virtual
*/
void Plants::action()
{
}

AttackRange Plants::get_attack_range() const
{
    return attack_range;
}

/*
Plant geter
@return plant's row
*/
int Plants::get_row() const
{
    return row;
}

/*
Plant geter
@return plant's column
*/
int Plants::get_col() const
{
    return col;
}

/*
Plant seter
Set plant's attacked time
*/
void Plants::set_attacked_time(const int &_attacked_time)
{
    attacked_time = _attacked_time;
}

/*
Decrease plant's health
*/
void Plants::decrease_health()
{
    --health;
}

/*
@return true if plant died (health <= 0)
*/
bool Plants::is_died()
{
    return health <= 0;
}

PlantAssets::PlantAssets(int _frame, int img_width, int img_height, int game_width, int game_height, int x_alias, int y_alias)
{
    this->frame = _frame;
    this->img_width = img_width;
    this->img_height = img_height;
    this->game_width = game_width;
    this->game_height = game_height;
    this->x_alias = x_alias;
    this->y_alias = y_alias;
}

void Plants::display(const PlantAssets &assets)
{
    // current frame
    int sframe = frame / assets.frame;
    int x_loc = cells[row][col].x1 + assets.x_alias;
    int y_loc = cells[row][col].y1 + assets.y_alias;
    win.draw_nth_frame(directory_num, sframe, assets.img_width, assets.img_height,
                       x_loc, y_loc, assets.game_width, assets.game_height);
    // Blink
    if (attacked_time)
    {
        win.draw_nth_frame(blink_of(directory_num), sframe, assets.img_width, assets.img_height,
                           x_loc, y_loc, assets.game_width, assets.game_height);
        if (check_status(game_state, IS_PAUSED) == false)
            attacked_time--;
    }
    // Next frame
    next_frame(assets);
}

void Plants::next_frame(const PlantAssets &assets)
{
    if (check_status(game_state, IS_PAUSED) == false)
        if (++frame >= assets.frame * all_img[directory_num].n_sheet)
        {
            frame = 0;
        }
}