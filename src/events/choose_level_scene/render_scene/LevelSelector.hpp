#pragma once
#include "draw/rsdl.hpp"
#include <iostream>
#include <string>
#include <vector>
using namespace std;

enum LevelStatus
{
    MOUSE_IN,
    MOUSE_OUT,
    LOCKED
};

class LevelSelector
{
private:
    int bg_dir;               // the level's OWN backdrop art (per-level crop)
    vector<int> debut;        // cell preview zombies: the types debuting at
                              // this level, or -- when nothing debuts -- the
                              // roster's strongest (level_roster_strongest_zombie)
    int display_plant;
    string level_name;

public:
    static int BACKGROUND_CHOOSE_LEVEL;
    static int TYPE_LEVEL;
    LevelSelector(int level_id);
    void display(int x, int y, RGB color);
    void display_locked(int x, int y);
    void display_blink(int x, int y);
    void display_complete(int x, int y);
    bool is_mouse_in(int mouse_x, int mouse_y, int x, int y);

    // Preview geometry of a card at screen (x, y), shared by display() and
    // selector_preview_check so the two cannot drift. The cell shows a crop
    // of the level's own backdrop (backdrop_src_rect: a 4:3 window placed
    // per backdrop class -- zoomed approach strip flush right for the yard
    // stages, the pool's water rectangle for pool, dead-center for fog, the
    // flush bottom-right deck edge for the roof stages) with the level's
    // debut zombie icon(s) on top -- or, for a level where nothing debuts,
    // the single strongest zombie of its roster: the cell width is split into one slot
    // per debutting type, each icon fitted INSIDE its slot (aspect kept, never upscaled past the legacy tuned height or the art's
    // natural size), centered in the slot, lifted a few px above the cell
    // bottom. The reward plant in its box obeys the same rule -- except the
    // lily pad, which stays bottom-flush. Returned rects are always inside
    // the cell.
    static SDL_Rect zombie_cell(int x, int y);
    static SDL_Rect plant_cell(int x, int y);
    static SDL_Point backdrop_art_size(int dir);
    static SDL_Rect backdrop_src_rect(int dir);
    static SDL_Rect debut_preview_rect(int dir, int index, int count, int x, int y);
    static SDL_Rect plant_preview_rect(int dir, int x, int y);
    // Selector icon art of a zombie type, with the closest-art fallback.
    static int zombie_icon_dir(int type);
    int backdrop_dir() const { return bg_dir; }
    const vector<int> &debut_types() const { return debut; }
    int plant_icon() const { return display_plant; }
};

struct Point
{
    int x, y;
};

// 5 cards at 150px pitch, 139px wide -> group spans 739px; x0 centers the
// group in the 1050px window ((1050-739)/2 = 155).
const Point LEVEL_BUTTON[] = {
    {155, 40}, {305, 40}, {455, 40}, {605, 40}, {755, 40}};
