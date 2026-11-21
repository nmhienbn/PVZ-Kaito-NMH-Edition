#pragma once
#include "../plants.hpp"

/*
Umbrella Leaf: covers a 3x3 neighbourhood. Bungee zombies picking a protected
tile bounce off empty-handed, and catapult basketballs landing under an
umbrella are deflected. Both zombie classes call these (their files got a
two-line hook each).
*/
bool umbrella_covers(const int &row, const int &col);   // pure range query
bool umbrella_protects(const int &row, const int &col);  // query + block anim flash

class UmbrellaLeaf : public Plants
{
public:
    UmbrellaLeaf(const int &_row, const int &_col);
    ~UmbrellaLeaf();
    void display(const int &_row) override;
    void action() override;
    void flash_block(); // a protected deflect just happened over me

private:
    int block_anim = 0; // >0 while the anim_block reel is playing
};

// Ticks of one anim_block pass (16 reanim frames @ 12 fps, 5 tpf).
#define UMBRELLA_BLOCK_TICKS 80
