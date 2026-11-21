#pragma once
#include "../plants.hpp"

class Zombie;
class ZombiePart;

// Magnet-shroom: pulls the nearest metal gear off a zombie within ~3 tiles
// (bucket/football helmets, door/ladder shields, jack-in-the-box, pogo stick,
// digger pickaxe), then needs ~15 s to recharge. Sleeps by day (mushroom).
class MagnetShroom : public Plants
{
public:
    MagnetShroom(const int &_row, const int &_col);
    ~MagnetShroom();
    void display(const int &_row) override;
    void action() override;
    bool is_night() const;
    bool try_suck(vector<Zombie *> &zombies, vector<ZombiePart> &zombie_parts);

private:
    int charge = 0;   // recharge ticks after a suck (~15 s)
    int suck_time = 0; // brief suck flourish (anim_shooting one pass)
};
