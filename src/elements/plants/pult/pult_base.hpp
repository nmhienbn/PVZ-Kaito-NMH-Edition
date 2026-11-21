#pragma once
#include "../plants.hpp"
#include "elements/bullets/lobbed/lobbed.hpp"

/*
Shared catapult-plant (pult) frame: idle until a zombie shows up down the
lane, then loop the lob animation. The projectile leaves the basket at the
whip point of anim_shooting, i.e. the tick the ammo track's art flips to
f=-1 (subclass fire_tick() is set to that tick). The lob's ammo is rolled
once per attack loop at the loop head (armed_ammo), like the original
assigning the kernel-pult's butter render group when the shooting anim
starts (repvz Plant.cpp:784-792), so the basket shows what it is about to
throw through the whole windup. Subclasses fill in the art slots (idle =
icon dir, attack = reanim state slot), the fire tick and pick the ammo via
pick_ammo().
*/
class PultBase : public Plants
{
public:
    PultBase(const int &_row, const int &_col);
    virtual ~PultBase();
    void action() override;
    void display(const int &_row) override;
    void next_frame(const PlantAssets &assets) override;

protected:
    virtual int idle_dir() const = 0;
    virtual int attack_dir() const = 0;
    virtual int fire_tick() const = 0;       // `frame` value that releases the lob
    virtual int lobbed_kind() const = 0;     // LobbedKind for this pult
    virtual const PlantAssets &idle_assets() const = 0;
    virtual const PlantAssets &attack_assets() const = 0;

    virtual int pick_ammo() const { return lobbed_kind(); } // kernel-pult rolls butter
    void determine_appearance(bool zombie_ahead);
    void fire_lobbed(vector<Bullet *> &bullets);
    bool attacking() const { return directory_num == attack_dir(); }
    // Ammo rolled for the current attack loop (-1 = basket empty: idle, or
    // fired and not yet reloaded). The reanim data itself re-shows the ammo
    // sprite late in the shooting segment (the reload pose).
    int armed_ammo = -1;
};
