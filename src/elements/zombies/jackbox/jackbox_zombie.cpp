#include "jackbox_zombie.hpp"
#include "draw/rsdl.hpp"
#include "draw/world_scale.hpp"
#include "elements/Level/Level.hpp"
#include "elements/Map/Map.hpp"
#include "elements/elements.hpp"
#include "elements/plants/plants.hpp"
#include "music/music.hpp"
#include "reanim/reanim_zombie.hpp"

extern Elements game_characters;
extern Map cells;
extern Level level;

// Pop beat: 110 ticks like the original mPhaseCounter (220 reanim-side);
// the surprise stinger fires with ~30 ticks to go (counter == 80 -> here 30).
const int JACK_POP_TICKS = 110;
const int JACK_SURPRISE_AT = 30;
// jackinthebox.ogg is 6.0 s; re-arm the loop a little early.
const int JACK_MUSIC_CLK = 350;
// Blast kills every plant in the 3x3 tiles centered on the zombie, plus
// zombies inside the original's 90-px blast circle, one row up/down.
const int JACK_BLAST_TILES = 1;
const int JACK_BLAST_ZOMBIE_PX = 90;

JackboxZombie::~JackboxZombie()
{
}

void JackboxZombie::init()
{
    dir_width = 166;
    dir_height = 144;
    health = *ZOMBIE_HEALTH_LIMIT[JACKBOX_TYPE].rbegin();
    directory_num = JACKBOX_WALK_DIRECTORY;
    // Original: pops after 450 + Rand(300) px of walking; 1/20 of them
    // are early poppers (3x sooner). Vasebreaker pots pop almost at once
    // (the original sets mPhaseCounter = 10 for scary potter).
    fuse_distance = 450 + rand(0, 300);
    if (rand(0, 19) == 0)
        fuse_distance /= 3;
    if (level.is_special(LevelSpecial::Vasebreaker))
        fuse_distance = 80;
    last_x = x_location; // fuse observer baseline (init_zombie placed us already)
}

void JackboxZombie::make_credit()
{
    x_location = rand(WINDOW_WIDTH, 1300) - 50;
    y_location = rand(0, WINDOW_HEIGHT - dir_height);
    directory_num = JACKBOX_WALK_DIRECTORY;
    state = RUNNING;
}

/*Crank music loop: only audible once the zombie shows on the board.*/
static void jack_music_tick(JackboxZombie &z, int &music_clk, bool popping)
{
    if (popping)
        return;
    if (z.x_location >= WINDOW_WIDTH - 10)
        return; // still off-screen
    if (--music_clk <= 0)
    {
        play_sound_effect(JACKINTHEBOX_MUSIC_DIRECTORY);
        music_clk = JACK_MUSIC_CLK;
    }
}

bool JackboxZombie::on_move_tick()
{
    if (state == POPPING)
    {
        pop_ticks--;
        if (pop_ticks == JACK_SURPRISE_AT)
            play_sound_effect(JACK_SURPRISE_SND_MUSIC_DIRECTORY);
        if (pop_ticks <= 0)
            explode(game_characters.zombie_parts);
        return true;
    }
    if (state == STOLEN)
        return false; // box gone: plain walker, fuse and music died with it
    // Fuse observer: the base stepper moves the zombie; the box only arms
    // while it actually walks (eating pauses the fuse), and the pop only
    // fires on the board.
    if (last_x >= 0)
    {
        int walked = last_x - x_location;
        if (walked > 0 && is_moving)
        {
            fuse_distance -= walked;
            if (fuse_distance <= 0 && x_location < WINDOW_WIDTH - 10)
            {
                state = POPPING;
                pop_ticks = JACK_POP_TICKS;
                frame = 0;
                reanim_time = 0.0f;
                float len = reanimz::skinSegmentLength(reanimz::SKIN_JACKBOX, "anim_pop");
                reanim_rate = len > 0 ? len / JACK_POP_TICKS : 0.2f;
            }
        }
    }
    last_x = x_location;
    jack_music_tick(*this, music_clk, false);
    return false;
}

/*Popping zombies stand still on top of plants; they never start eating again.*/
bool JackboxZombie::on_reach_plant(const int &col)
{
    return state == POPPING;
}

bool JackboxZombie::can_eat_plant() const
{
    return state != POPPING;
}

void JackboxZombie::steal_jackbox()
{
    // repvz MagnetShroomAttactItem: stealing is damage-free; the phase drops
    // to PHASE_ZOMBIE_NORMAL (box/handle tracks hidden, music stopped).
    if (state == RUNNING)
    {
        state = STOLEN;
        music_clk = 0; // the crank loop re-arms only for RUNNING zombies anyway
    }
}

/*The blast (repvz PHASE_JACK_IN_THE_BOX_POPPING, Zombie.cpp:2070-2087):
plants in the 3x3 tiles around the zombie die AND every zombie whose body
overlaps the 90-px blast circle (row range 1) burns
(KillAllZombiesInRadius, Board.cpp:9838, theBurn -> ApplyBurn) -- the popper
included, and, in the original, the ladders inside it too (no ladder griditem
support here). The box itself goes with the zombie (pending_removal: no
corpse left).

explode() runs from on_move_tick, i.e. INSIDE move_zombies' iteration over
the zombie vector, so blast victims cannot be erased here; they are queued
and drained at the top of the next update_zombies_status
(drain_jack_explosion_kills).*/
static vector<Zombie *> g_blast_kills;

void drain_jack_explosion_kills(vector<Zombie *> &zombies)
{
    if (g_blast_kills.empty())
        return;
    for (auto *victim : g_blast_kills)
        for (int i = 0; i < (int)zombies.size(); i++)
            if (zombies[i] == victim)
            {
                delete victim;
                zombies.erase(zombies.begin() + i);
                break;
            }
    g_blast_kills.clear();
}

void JackboxZombie::explode(vector<ZombiePart> &zombie_parts)
{
    play_sound_effect(CHERRYBOMB_MUSIC_DIRECTORY);
    int zcol = get_block_col(x_location + 80);
    vector<Plants *> &plants = game_characters.plants;
    for (int i = 0; i < (int)plants.size(); i++)
    {
        int dr = plants[i]->get_row() - row;
        int dc = plants[i]->get_col() - zcol;
        if (dr >= -JACK_BLAST_TILES && dr <= JACK_BLAST_TILES &&
            dc >= -JACK_BLAST_TILES && dc <= JACK_BLAST_TILES)
            delete_plant(plants, i);
    }
    // Zombie half of the blast: 90-px radius, one row up/down. Queued, not
    // erased (see the comment above).
    for (auto *z : game_characters.zombies)
    {
        if (z == this || z->is_underground())
            continue;
        if (z->row < row - 1 || z->row > row + 1)
            continue;
        if (abs(z->x_location - x_location) > JACK_BLAST_ZOMBIE_PX)
            continue;
        if (!z->take_hit(90, zombie_parts)) // 90 peas ~= the 1800 raw burn
            continue;                       // survived (boss-class)
        play_sound_effect(ZOMBIE_FALLING_MUSIC_DIRECTORY);
        push_burned_corpse(zombie_parts, z);
        g_blast_kills.push_back(z);
    }
    // Blast flash: reuse the cherry-bomb explosion sheet as a big one-shot.
    zombie_parts.push_back(ZombiePart(CHERRYBOMB_EXPLOSION_DIRECTORY, 3, row,
                                      x_location - 70, 300, 300));
    blown_up = true; // DieNoLoot: no corpse; the sweeper removes the body
}

/*Degrade is a plain flash here (PvZ1 only bares the arm stump; the skin has
no armless variant wired up). The box counts as armor: keep walk art until
the zombie dies.*/
void JackboxZombie::determine_appearance(vector<ZombiePart> &zombie_parts)
{
}

void JackboxZombie::add_zombie_die(vector<ZombiePart> &zombie_parts)
{
    if (reanimz::ready())
    {
        zombie_parts.push_back(ZombiePart(reanimz::DEATH_PART_IMG, 1, row, x_location,
                                          ZOMBIE_WIDTH, ZOMBIE_HEIGHT, cold_time > 0,
                                          reanimz::SKIN_JACKBOX));
        return;
    }
    Zombie::add_zombie_die(zombie_parts);
}

void JackboxZombie::render_zombie()
{
    if (state == POPPING)
    {
        if (reanimz::drawSkinSegment(reanimz::SKIN_JACKBOX, "anim_pop", reanim_time,
                                     x_location, y_location, cold_time > 0, attacked_time > 0,
                                     0, 0, WORLD_SCALE))
            return;
    }
    if (state == STOLEN)
    {
        // Box magneted off: plain body walk, no box art.
        static const std::unordered_set<std::string> noBox = {
            "zombie_jackbox_box", "zombie_jackbox_box2"};
        if (reanimz::drawZombie(directory_num, armor_dir, reanim_time,
                                x_location, y_location, cold_time > 0, attacked_time > 0,
                                &noBox))
            return;
    }
    Zombie::render_zombie();
}
