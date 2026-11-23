#include "AttackPlantVsZombie.hpp"
#include "elements/Map/Map.hpp"
#include <algorithm>

extern Map cells;

/*repvz EffectedByDamage's DAMAGAGES_UNDERGROUND bit (Zombie.cpp:8241-8245):
only the TUNNELING digger sits underground. The instant-kill blasts all carry
that bit (cherry bomb / doom-shroom / jalapeno flags 127, potato mine flags
77 in GetDamageRangeFlags, Plant.cpp:587-621), so a digging digger IS blown
up (the blast reaches the tunnel). BoulderZombie / the boss only borrow is_underground() as a hit
immunity flag (boss_zombie.hpp) -- they are not diggers, so they keep theirs.
Straight-shot targeting (peas, fumes) uses ground flags only and still skips
anything underground.*/
static bool underground_blocks(const Plants &plant, const Zombie &zombie)
{
    if (!zombie.is_underground())
        return false;
    switch (plant.get_attack_range())
    {
    case THREE_X_FIVE:
    case THREE_X_THREE:
    case ONE_X_ONE:
        return zombie.type != DIGGER_TYPE; // blast flags include UNDERGROUND
    default:
        return true; // pea/fume targeting: ground shots never target a tunneler
    }
}

/*repvz GetCircleRectOverlap (2D): closest point of the rect to the circle
center inside the radius counts as hit.*/
static bool circle_rect_overlap(const int &cx, const int &cy, const int &r,
                                const int &rx1, const int &ry1, const int &rx2, const int &ry2)
{
    int nx = std::max(rx1, std::min(cx, rx2));
    int ny = std::max(ry1, std::min(cy, ry2));
    int dx = cx - nx, dy = cy - ny;
    return dx * dx + dy * dy <= r * r;
}

bool check_in_attack_range(const Plants &plant, const Zombie &zombie)
{
    if (zombie.is_charmed)
        return false; // hypnotized zombies are on the player's side
    if (underground_blocks(plant, zombie))
        return false;
    switch (plant.get_attack_range())
    {
    case ONE_ROW_AHEAD:
    {
        // Upper bound: repvz EffectedByDamage (Zombie.cpp:8236-8239) only
        // stops effecting "off-board" zombies (rect mX > WIDE_BOARD_WIDTH =
        // 800; clone +220) -- 1020 here. A fresh spawn draws fire right away.
        if (plant.get_row() == zombie.get_row() &&
            is_in(cells[0][plant.get_col()].x2 - 140, zombie.x_location, X_UPPER_LEFT - 40 + 800))
            return true;
        else
            return false;
    }
    case ONE_ROW_AHEAD_FOUR:
    {
        // Fume-shroom: PvZ1 fumes soak mX+60 .. mX+60+4*80 (about 4 tiles).
        int left_limit = cells[plant.get_row()][plant.get_col()].x1 + 60;
        int right_limit = left_limit + 340;
        if (plant.get_row() == zombie.get_row() &&
            is_in(left_limit, zombie.x_location + 40, right_limit))
            return true;
        return false;
    }
    case THREE_X_FIVE:
    {
        // Doom-shroom (repvz Plant.cpp:4388-4400 -> KillAllZombiesInRadius,
        // Board.cpp:9838-9867): a 250-px blast circle on the tile center
        // against the zombie body rect, row distance filtered to +-3 -- on
        // 100-px rows the circle reaches about +-2 rows in practice, so a
        // mid-lawn doom covers 5 rows, not the old clone 3.
        // Doom-shroom: radius 250, rowRange 3, circle at the plant origin
        // (repvz Plant::DoSpecial aPosX/aPosY = mX+40, mY+40 -> Plant.cpp:4326;
        // Board::KillAllZombiesInRadius Board.cpp:9301-9317).
        const Block &cell = cells[plant.get_row()][plant.get_col()];
        if (abs(plant.get_row() - zombie.row) > 3)
            return false;
        return circle_rect_overlap(cell.x1 + 40, cell.y1 + 40, 250,
                                   zombie.zrect_left(), zombie.zrect_top(),
                                   zombie.zrect_right(), zombie.zrect_bottom());
    }
    case THREE_X_THREE:
    {
        // Cherry bomb: radius 115, rowRange 1, circle centred on the plant
        // origin (repvz Plant.cpp:4313 -> KillAllZombiesInRadius).
        const Block &cell = cells[plant.get_row()][plant.get_col()];
        if (!is_in(plant.get_row() - 1, zombie.row, plant.get_row() + 1))
            return false;
        return circle_rect_overlap(cell.x1 + 40, cell.y1 + 40, 115,
                                   zombie.zrect_left(), zombie.zrect_top(),
                                   zombie.zrect_right(), zombie.zrect_bottom());
    }
    case ONE_X_ONE:
    {
        // Potato mine: radius 60, rowRange 0, centre (mX+20, mY+40)
        // (repvz Plant.cpp:4373-4377).
        const Block &cell = cells[plant.get_row()][plant.get_col()];
        if (plant.get_row() != zombie.get_row())
            return false;
        return circle_rect_overlap(cell.x1 + 20, cell.y1 + 40, 60,
                                   zombie.zrect_left(), zombie.zrect_top(),
                                   zombie.zrect_right(), zombie.zrect_bottom());
    }
    default:
        return false;
    }
    return false;
}

bool check_in_attack_range(const Plants &plant, const vector<Zombie *> &zombies)
{
    for (const auto &zombie : zombies)
        if (check_in_attack_range(plant, *zombie))
            return true;
    return false;
}
