#include "yeti_zombie.hpp"
#include "draw/rsdl.hpp"
#include "elements/elements.hpp" // game_characters.suns
#include "reanim/reanim_zombie.hpp"

// Fixed paces (repvz PickRandomSpeed, Zombie.cpp:1188-1195; the yeti does
// not roll the generic band), HEAD-speed-scaled.
const float YETI_WALK_PX = CS_VEL(0.4f) * HEAD_SPEED_SCALE; // 21.8 px/s walk-in
const float YETI_FLEE_PX = CS_VEL(0.8f) * HEAD_SPEED_SCALE; // 43.6 px/s PHASE_YETI_RUNNING

/*DropLoot payout (repvz Zombie.cpp:7362-7368): the original yeti drops
diamonds into the money counter. With no money HUD this clone pays the kill
as sun through the coin-sun pipeline; 50 keeps it a small jackpot (a strict
$1 -> 1 sun mapping of the retail $1000 diamond would be absurd).*/
#define YETI_DIAMOND_SUN 50

extern Elements game_characters;

YetiZombie::~YetiZombie()
{
}

void YetiZombie::init()
{
    dir_width = 166;
    dir_height = 144;
    health = *ZOMBIE_HEALTH_LIMIT[YETI_TYPE].rbegin(); // 68 (zombies.json)
    directory_num = YETI_WALK_DIRECTORY;
    // Turn-tail timer: RandRangeInt(1500, 2000) centiseconds in the original
    // (100 cs/s) -> 900..1200 game ticks (60 tps).
    flee_clk = rand(900, 1200);
}

void YetiZombie::make_credit()
{
    x_location = rand(WINDOW_WIDTH, 1300) - 50;
    y_location = rand(0, WINDOW_HEIGHT - dir_height);
    directory_num = YETI_WALK_DIRECTORY;
}

float YetiZombie::speed_px_per_tick() const
{
    return fleeing ? YETI_FLEE_PX : YETI_WALK_PX;
}

bool YetiZombie::on_move_tick()
{
    if (!fleeing)
    {
        // UpdateYeti (Zombie.cpp:4050-4054): once the phase counter runs out
        // the yeti turns and runs. Mid-chew it just gets up and goes.
        if (--flee_clk <= 0)
        {
            fleeing = true;
            if (!is_moving)
            {
                is_moving = true;
                directory_num = YETI_WALK_DIRECTORY;
                frame = 0;
                reanim_time = 0.0f;
            }
        }
        return false; // walk-in/chew uses the base stepper
    }
    // Ran off the right edge: leaves the lawn for good (digger pattern).
    if (x_location > WINDOW_WIDTH + 20)
    {
        fled_off_screen = true;
        return true;
    }
    float v = YETI_FLEE_PX;
    if (cold_time)
        v *= CHILLED_SPEED_FACTOR; // chilled flee slows like any movement
    walk_carry += v;
    int step = (int)walk_carry;
    walk_carry -= step;
    x_location += step;
    return true;
}

bool YetiZombie::on_reach_plant(const int &col)
{
    return fleeing; // fleeing yetis pass plants by (no stop-and-eat)
}

void YetiZombie::add_zombie_die(vector<ZombiePart> &zombie_parts)
{
    // DropLoot on kill: one diamond pickup hops out of the body (the same
    // coin-sun arc/collect pipeline, diamond art, YETI_DIAMOND_SUN sun).
    spawn_money_pickup(game_characters.suns, row,
                       x_location + dir_width / 2 - DIAMOND_DRAW_WIDTH / 2,
                       y_location + dir_height / 3,
                       YETI_DIAMOND_SUN, MONEY_DIAMOND);
    if (reanimz::ready())
    {
        zombie_parts.push_back(ZombiePart(reanimz::DEATH_PART_IMG, 1, row, x_location,
                                          ZOMBIE_WIDTH, ZOMBIE_HEIGHT, cold_time > 0,
                                          reanimz::SKIN_YETI, false, head_dropped));
        return;
    }
    Zombie::add_zombie_die(zombie_parts);
}
