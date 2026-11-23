#pragma once
#include "../zombie_struct.hpp"
#include "draw/ImageType.hpp"
#include "game_param.hpp"
#include "reanim/reanim_zombie.hpp"

/*
STUB-AREA-AGENT: generic reanim-skinned zombie standing in for every
area 3-5 zombie (ducky tube, snorkel, zomboni, bobsled, dolphin rider,
jack-in-the-box, balloon, digger, pogo, bungee, ladder, catapult,
gargantuar, imp, zomboss) until the area agents write the real classes.

Behavior = plain zombie: walks, eats, degrades with a flash (no torn-arm
part yet), dies with the skin's anim_death corpse. rendering goes through
the reanim skin mapped from its walk/eat state directory numbers
(reanim_zombie.cpp STUB_STATES); the sprite-sheet fallback draws the
plain-zombie sheets the state slots point at.
*/
class StubZombie : public Zombie
{
public:
    StubZombie(int walk_dir, int eat_dir, int skin)
        : m_walk_dir(walk_dir), m_eat_dir(eat_dir), m_skin(skin) {}
    ~StubZombie() override = default;

    void init() override
    {
        dir_width = 166;
        dir_height = 144;
        health = *ZOMBIE_HEALTH_LIMIT[type].rbegin();
        directory_num = m_walk_dir;
    }

    void make_credit() override
    {
        x_location = rand(WINDOW_WIDTH, 1300) - 50;
        y_location = rand(0, WINDOW_HEIGHT - dir_height);
        directory_num = m_walk_dir;
    }

    void determine_appearance(vector<ZombiePart> &) override
    {
        // No armor/arm visuals on the stubs: the attack flash is the cue.
    }

    void add_zombie_die(vector<ZombiePart> &zombie_parts) override
    {
        if (reanimz::ready() && m_skin >= 0)
        {
            zombie_parts.push_back(ZombiePart(reanimz::DEATH_PART_IMG, 1, row, x_location,
                                              ZOMBIE_WIDTH, ZOMBIE_HEIGHT, cold_time > 0,
                                              m_skin, false, head_dropped));
            return;
        }
        Zombie::add_zombie_die(zombie_parts);
    }

private:
    int m_walk_dir, m_eat_dir, m_skin;
};
