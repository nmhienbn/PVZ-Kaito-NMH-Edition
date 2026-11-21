#pragma once
#include "../zombie_struct.hpp"

/*
Zomboni (pool dry lanes -- it never drives on water): rides the ice
resurfacer in, flattens every plant it touches (crushes_plants, handled in
ZombieVsPlant), and leaves an ice trail behind the wheels that the bobsled
team slides on.

  + Speed decays along the lane like the original (UpdateZamboni, repvz
    Zombie.cpp:4015: velX 0.25 px/cs -> 0.05 between x=700 and x=300) --
    speed_px_per_tick() replays that curve.
  + Ice trail: level.ice_min_x[row] -- leftmost x the trail reaches back to
    the street; level.ice_timer[row] holds the trail's remaining life
    (~30 s, refreshed while driving; jalapeno burns it off early; the trail
    melts when the timer hits 0, original Board::UpdateIce).
  + The zamboni doc ships no death animation: a killed zomboni disappears
    with the falling sound, no corpse part (approximation).
*/
class ZomboniZombie : public Zombie
{
public:
    ~ZomboniZombie() override;
    void init() override;
    void make_credit() override;
    bool on_move_tick() override;
    bool on_reach_plant(const int &col) override;
    bool can_eat_plant() const override;
    bool crushes_plants() const override;
    float speed_px_per_tick() const override;
    void determine_appearance(vector<ZombiePart> &zombie_parts) override;
    void add_zombie_die(vector<ZombiePart> &zombie_parts) override;
    using Zombie::Zombie;

    /*The zomboni's ice trail: leftmost covered x of the lane (or -1).
    Lazily sized to the current grid.*/
    static int ice_front_x(int row);

private:
    int engine_sound_clk = 0; // retrigger the zamboni hum while it drives
};
