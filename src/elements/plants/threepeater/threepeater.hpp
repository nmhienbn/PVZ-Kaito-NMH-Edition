#pragma once
#include "../plants.hpp"
#include "elements/bullets/pea/pea.hpp"

class ThreePeater : public Plants
{
private:
    bool has_zombie_ahead(Plants &plant, const int &row);

public:
    ThreePeater(const int &_row, const int &_col);
    ~ThreePeater();
    void determine_appearance(bool check_zombie_in_attack_range);
    void display(const int &_row) override;
    void action() override;
    void fire_peas(vector<Bullet *> &bullets);
    void next_frame(const PlantAssets &assets) override;
};
