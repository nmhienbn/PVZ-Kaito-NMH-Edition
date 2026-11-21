#pragma once
// STUB-AREA-AGENT
// Generic inert plant for the area 3-5 roster (LILYPAD_TYPE..MELONPULT_TYPE).
// It renders via the normal Plants::display pipeline: the reanim skin idles on
// anim_idle (reanim_plant.cpp), with the packet icon (draw/plant_icon_dir.hpp
// PLANT_ICON_DIR[type]) as the legacy fallback.
// Behavior (attack/produce/instant effects) is intentionally absent — each
// area agent replaces its plants with real classes and re-registers them in
// plant_type.cpp.
#include "../plants.hpp"
#include "draw/plant_icon_dir.hpp"
#include "game_param.hpp"

class ReanimStubPlant : public Plants
{
public:
    ReanimStubPlant(const int &ptype, const int &_row, const int &_col) : Plants(_row, _col)
    {
        type = ptype;
        health = PLANT_HEALTH_LIMIT[ptype];
        sec_for_prepare = 1;
        directory_num = PLANT_ICON_DIR[ptype];
        attack_range = NO_ATTACK;
    }

    void display(const int &_row) override
    {
        if (row == _row)
        {
            static const PlantAssets STUB_ASSETS(2, 85, 81, 85, 81, 5, 5);
            Plants::display(STUB_ASSETS);
        }
    }

    void action() override {} // STUB-AREA-AGENT: no behavior yet
};
