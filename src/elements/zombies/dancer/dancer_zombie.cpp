#include "dancer_zombie.hpp"
#include "../zombie_type.hpp"
#include "draw/rsdl.hpp"
#include "elements/Level/Level.hpp"
#include "elements/Map/Map.hpp"
#include "elements/elements.hpp"
#include "music/music.hpp"
#include "reanim/reanim_zombie.hpp"

extern Map cells;
extern Level level;
extern Elements game_characters; // read-only peek at the live zombie crowd

// Backup dancers summoned mid-tick wait here until the zombie vector is not
// being iterated (drain happens at the top of handle_changes).
static vector<Zombie *> g_pending_backups;

void queue_spawned_zombie(Zombie *z)
{
    g_pending_backups.push_back(z);
}

void drain_spawning_zombies(vector<Zombie *> &zombies)
{
    for (auto *z : g_pending_backups)
        zombies.push_back(z);
    g_pending_backups.clear();
}

/*Spawn one backup dancer at an exact formation slot.*/
static Zombie *make_backup_at(const int &row, const int &x)
{
    Zombie *z = init_zombie(BACKUP_TYPE, level.level_num);
    z->row = row;
    z->x_location = x;
    z->y_location = cells[row][0].y1 - 50;
    return z;
}

/*A slot is filled if a living backup dancer stands near it.*/
static bool slot_filled(const vector<Zombie *> &zs, const int &row, const int &x)
{
    for (const auto *z : zs)
        if (z->type == BACKUP_TYPE && z->row == row &&
            abs(z->x_location - x) < 60)
            return true;
    for (const auto *z : g_pending_backups)
        if (z->type == BACKUP_TYPE && z->row == row &&
            abs(z->x_location - x) < 60)
            return true;
    return false;
}

DancingZombie::~DancingZombie()
{
}

float DancingZombie::speed_px_per_tick() const
{
    return DANCER_WALK_PX;
}

void DancingZombie::init()
{
    dir_width = 166;
    dir_height = 144;
    health = DANCER_TOTAL_HEALTH;
    directory_num = DANCER_WALK_DIRECTORY;
}

void DancingZombie::make_credit()
{
    x_location = rand(WINDOW_WIDTH, 1300) - 50;
    y_location = rand(0, WINDOW_HEIGHT - dir_height);
    directory_num = DANCER_WALK_DIRECTORY;
}

void DancingZombie::start_raise()
{
    raise_left = DANCER_RAISE_TICKS;
    is_moving = false;
    directory_num = DANCER_RAISE_DIRECTORY;
    frame = 0;
    reanim_time = 0.0f;
    play_sound_effect(DANCER_MUSIC_DIRECTORY);
}

/*
Fill any empty formation slot: (row, x +/- 100) and (row +/- 1, x), skipping
rows outside the playable lanes.
*/
void DancingZombie::queue_missing_backups()
{
    bool row_ok[5] = {false, false, false, false, false};
    for (const int r : level.playable_rows)
        if (r >= 0 && r < 5)
            row_ok[r] = true;
    struct
    {
        int row, x;
    } slots[4] = {
        {row, x_location - 100}, {row, x_location + 100}, {row - 1, x_location}, {row + 1, x_location}};
    for (const auto &slot : slots)
    {
        if (slot.row < 0 || slot.row >= 5 || !row_ok[slot.row])
            continue;
        if (slot_filled(game_characters.zombies, slot.row, slot.x))
            continue;
        queue_spawned_zombie(make_backup_at(slot.row, slot.x));
    }
}

/*
The raise pose freezes the dancer in place; the summon fires when it ends.
Between raises the dancer re-checks its formation every ~10 s.
*/
bool DancingZombie::on_move_tick()
{
    if (raise_left > 0)
    {
        if (--raise_left == 0)
        {
            directory_num = DANCER_WALK_DIRECTORY;
            frame = 0;
            reanim_time = 0.0f;
            is_moving = true;
            queue_missing_backups();
        }
        return true;
    }
    if (!is_moving)
        return false; // chewing a plant
    if (x_location > WINDOW_WIDTH - DANCER_SUMMON_ENTER_X)
        return false; // still marching in from off-screen
    if (!has_summoned)
    {
        has_summoned = true;
        start_raise();
        return true;
    }
    if (--resummon_clk <= 0)
    {
        resummon_clk = DANCER_RESUMMON_CLK;
        // Re-call only when the formation broke. Peek at counts first: cheap
        // check of whether any of the 4 slots is empty in the live crowd.
        int near = 0;
        for (const auto *z : game_characters.zombies)
            if (z->type == BACKUP_TYPE && abs(z->row - row) <= 1 &&
                abs(z->x_location - x_location) < 160)
                near++;
        if (near < 4)
        {
            start_raise();
            return true;
        }
    }
    return false;
}

/*Only the outer arm drops at one-third health. Threshold, not exact hit:
a melon (4 dmg) crosses the mark without landing on it.*/
void DancingZombie::determine_appearance(vector<ZombiePart> &zombie_parts)
{
    if (health <= DANCER_ARM_HEALTH * health_scale && armor_dir != reanimz::DANCER_ARMLESS_STATE)
    {
        armor_dir = reanimz::DANCER_ARMLESS_STATE;
        if (reanimz::ready())
            zombie_parts.push_back(ZombiePart(reanimz::DROP_ARM_IMG, 1, row, x_location,
                                              ZOMBIE_WIDTH, ZOMBIE_HEIGHT, cold_time > 0,
                                              reanimz::SKIN_DANCER));
        else
            zombie_parts.push_back(ZombiePart(ZOMBIE_ARM_DIRECTORY, 1, row, x_location,
                                              ZOMBIE_WIDTH, ZOMBIE_HEIGHT, cold_time > 0));
    }
}

void DancingZombie::add_zombie_die(vector<ZombiePart> &zombie_parts)
{
    if (reanimz::ready())
    {
        zombie_parts.push_back(ZombiePart(reanimz::DEATH_PART_IMG, 1, row, x_location,
                                          ZOMBIE_WIDTH, ZOMBIE_HEIGHT, cold_time > 0,
                                          reanimz::SKIN_DANCER, false, head_dropped));
        return;
    }
    Zombie::add_zombie_die(zombie_parts);
}

BackupDancerZombie::~BackupDancerZombie()
{
}

float BackupDancerZombie::speed_px_per_tick() const
{
    return DANCER_WALK_PX;
}

void BackupDancerZombie::init()
{
    dir_width = 166;
    dir_height = 144;
    health = BACKUP_TOTAL_HEALTH;
    directory_num = BACKUP_WALK_DIRECTORY;
}

void BackupDancerZombie::make_credit()
{
    x_location = rand(WINDOW_WIDTH, 1300) - 50;
    y_location = rand(0, WINDOW_HEIGHT - dir_height);
    directory_num = BACKUP_WALK_DIRECTORY;
}

void BackupDancerZombie::determine_appearance(vector<ZombiePart> &zombie_parts)
{
    if (health <= BACKUP_ARM_HEALTH * health_scale && armor_dir != reanimz::DANCER_ARMLESS_STATE)
    {
        armor_dir = reanimz::DANCER_ARMLESS_STATE;
        if (reanimz::ready())
            zombie_parts.push_back(ZombiePart(reanimz::DROP_ARM_IMG, 1, row, x_location,
                                              ZOMBIE_WIDTH, ZOMBIE_HEIGHT, cold_time > 0,
                                              reanimz::SKIN_BACKUP));
        else
            zombie_parts.push_back(ZombiePart(ZOMBIE_ARM_DIRECTORY, 1, row, x_location,
                                              ZOMBIE_WIDTH, ZOMBIE_HEIGHT, cold_time > 0));
    }
}

void BackupDancerZombie::add_zombie_die(vector<ZombiePart> &zombie_parts)
{
    if (reanimz::ready())
    {
        zombie_parts.push_back(ZombiePart(reanimz::DEATH_PART_IMG, 1, row, x_location,
                                          ZOMBIE_WIDTH, ZOMBIE_HEIGHT, cold_time > 0,
                                          reanimz::SKIN_BACKUP, false, head_dropped));
        return;
    }
    Zombie::add_zombie_die(zombie_parts);
}
