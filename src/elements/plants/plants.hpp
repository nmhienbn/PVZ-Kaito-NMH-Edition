#pragma once
#include "draw/rsdl.hpp"
#include "music/music.hpp"

enum AttackRange
{
    NO_ATTACK,
    ONE_ROW_AHEAD,
    THREE_X_THREE,
    ONE_X_ONE,
    ONE_ROW_AHEAD_FOUR, // fume-shroom: its own row, ~4 tiles ahead
    THREE_X_FIVE        // doom-shroom: 3 rows x 5 tiles
};

struct PlantAssets
{
    int frame; // How many game ticks a state frame in the sprite sheet runs
    int img_width;
    int img_height;
    int game_width;
    int game_height;
    // Cell-top offsets to the sprite's top-left corner; Plants::display draws
    // the sheet with a bottom-left anchor, so the sprite's ground contact is
    // the invariant point at cell.y1 + y_alias + game_height (+ roof slope).
    int x_alias;
    int y_alias;
    PlantAssets(int _frame, int img_width, int img_height, int game_width, int game_height, int x_alias, int y_alias);
};

class Plants
{
protected:
    int type;
    int row, col;
    int health;
    int sec_for_prepare;
    int directory_num;
    int frame = 0;
    int attacked_time = 0;
    float pam_clock = 0.0f; // pam-local time (frames) for non-synced loops
    int pam_dir = -1;       // last sprite sheet dir rendered via PAM
    float reanim_clock = 0.0f; // reanim-local time (frames) for non-synced loops
    int reanim_dir = -1;       // last sprite sheet dir rendered via reanim
    AttackRange attack_range;

    // Frame advance with an attack-loop cap: one attack loop wraps after
    // cap_ticks game ticks so a shooter's launch interval matches the
    // original's mLaunchRate even when the sheet loop runs longer. Other
    // states (idle, sleep...) keep their full sheet wrap.
    void next_frame_capped(const PlantAssets &assets, const int &attack_dir, const int &cap_ticks);

public:
    Plants();
    Plants(const int &_row, const int &_col);

    AttackRange get_attack_range() const;
    // behavior checks: which sprite-sheet/reanim state the plant is showing
    int get_directory_num() const;
    int get_type() const;
    int get_row() const;
    int get_col() const;
    int get_health() const;
    void set_attacked_time(const int &_attacked_time);
    void decrease_health();
    bool is_died();
    void display(const PlantAssets &assets);
    // PvZ2 pam renderer; returns false when the state has no PAM mapping or
    // the assets are missing (caller keeps the sprite-sheet path).
    bool display_pam(const PlantAssets &assets);
    // PopCap reanim (per-part) renderer; returns false when the state has no
    // reanim mapping or the assets are missing (caller falls back to PAM,
    // then the sprite sheets).
    bool display_reanim(const PlantAssets &assets);

    virtual ~Plants();
    virtual void display(const int &_row);
    virtual void action();
    virtual void next_frame(const PlantAssets &assets);

    // Coffee Bean wake-up: a mushroom dozing in daylight flips this and its
    // is_night() checks start passing (the shroom's own code then runs as if
    // it were night). Set by CoffeeBean, read by the mushroom classes.
    bool coffee_awake = false;

    /*F2 layout-editor transient offsets (px): the render paths (sheet
    Plants::display + the reanim hook) add them to the drawn position, and the
    rsdl debug-scale registry scales the art about its bottom point. Render-
    only; the editor restores the spawn defaults (roof bake in init_plant, 0
    elsewhere) on close -- the F9 dump is the hand-off, they never persist
    into level data.*/
    int layout_dx = 0, layout_dy = 0;
};

void delete_plant(vector<Plants *> &plants, int &ind);
void update_plants_status(vector<Plants *> &plants);

/*Roof spawn defaults baked from the designer's hand-tuned F2 layout dump
(levels 41-50): the editor's transient layout_dx/layout_dy reset on close, so
the tuned pixels live here. roof_pot_layout_default is a per-row translation
for flower pots: the designer only dragged column 0, but that drag means the
WHOLE row shifts, so the dumped c0 offset applies to every column of the row
(sloped and flat alike). roof_passenger_layout_default is a plant riding a
same-cell pot, derived as pot offset + (+3,-22) (dump: sunflower on r0c1 over
pot (-2,-12) -> (+1,-34)) so a pot-offset change propagates. Non-roof maps
leave both at (0,0) -- callers gate on level.is_roof().*/
void roof_pot_layout_default(const int &row, const int &col, int &dx, int &dy);
void roof_passenger_layout_default(const int &row, const int &col, int &dx, int &dy);