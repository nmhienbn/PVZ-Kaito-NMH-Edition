#pragma once
#include "../plants.hpp"

// Blast freeze: repvz HitIceTrap (Zombie.cpp:8472) arms the ice trap at
// RandRangeInt(400,600)cs for a fresh zombie -> 240..360 ticks at 60 tps.
// The chill that outlasts it is ICE_SHROOM_COLD_TIME (game_param.hpp).
#define ICESHROOM_FREEZE_MIN_TICKS 240
#define ICESHROOM_FREEZE_MAX_TICKS 360

class IceShroom : public Plants
{
public:
    IceShroom(const int &_row, const int &_col);
    ~IceShroom();
    bool is_night() const;
    /*Tick the fuse; when it fires, sets the EXPLODE state and returns true so
    the caller can freeze every zombie it knows about.*/
    bool fuse_step();
    /*Tick the blast fade; the shroom melts away once the overlay is gone.*/
    void melt_step();
    bool exploded() const { return has_exploded; }
    void display(const int &_row) override;
    void action() override;

private:
    bool has_exploded = false;
    int blast_left; // the icetrap overlay (and plant) linger this long fading
};
