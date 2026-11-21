#include "almanac_info.hpp"
#include "game_param.hpp"
#include "level/plants_registry.hpp"
#include "level/zombies_registry.hpp"

// Display names come from the registries (level/plants_registry.hpp /
// level/zombies_registry.hpp): single source for name <-> type, indexed by
// PlantType/ZombieType (PEASHOOTER_TYPE=0 .. MELONPULT_TYPE=PLANT_COUNT-1;
// NORMAL_TYPE=0 .. YETI_TYPE=COUNT_ZOMBIE_TYPE-1).
static const char *PLANT_NAMES[PLANT_COUNT];
static const char *ZOMBIE_NAMES[COUNT_ZOMBIE_TYPE];

namespace
{
struct AlmanacTableInit
{
    AlmanacTableInit()
    {
        for (const auto &def : PLANT_DEFS)
            PLANT_NAMES[def.type] = def.display;
        for (const auto &def : ZOMBIE_DEFS)
            ZOMBIE_NAMES[def.type] = def.display;
    }
} almanac_table_init; // same-TU static init: runs before any almanac_* call
} // namespace

static const char *PLANT_DESCS[PLANT_COUNT] = {
    "Peashooters are your first line of defense. They shoot peas at attacking zombies.",
    "Sunflowers are essential for you to produce extra sun. Try planting as many as you can!",
    "Wall-nuts have hard shells which you can use to protect your other plants.",
    "Snow Peas fire frozen peas making it hard for zombies to advance.",
    "Potato Mines pack a powerful punch, but they need a while to arm themselves.",
    "Cherry Bombs can blow up all zombies in an area. They have a short fuse, so plant them near zombies.",
    "Repeaters fire two peas at one time.",
    "Chompers can devour a zombie whole, but they are vulnerable while chewing.",
    "Puff-shrooms are cheap, but can only fire a short distance. Puff-shrooms sleep during the day.",
    "Sun-shrooms give small sun at first and normal sun later. They sleep during the day.",
    "Fume-shrooms shoot fumes that can pass through screen doors. They sleep during the day.",
    "Plant Grave Busters on graves to remove the graves.",
    "When zombies eat Hypno-shrooms, they will turn around and fight for you. Sleeps during the day.",
    "Scaredy-shrooms are long-ranged shooters that hide when enemies get near them.",
    "Ice-shrooms temporarily immobilize all zombies on the screen.",
    "Doom-shrooms destroy everything in a large area and leave a crater that can't be planted on.",
    "Lily Pads let you plant non-aquatic plants on top of them in the pool.",
    "Squashes will smash the first zombie that gets close to them.",
    "Threepeaters shoot peas in three lanes.",
    "Tangle Kelp are aquatic plants that pull the first zombie that nears them underwater.",
    "Jalapenos destroy an entire lane of zombies.",
    "Spikeweeds pop tires and hurt any zombies that step on them.",
    "Torchwoods turn peas that pass through them into fireballs for double damage.",
    "Tall-nuts are heavy-duty wall plants that can't be vaulted over.",
    "Sea-shrooms are aquatic plants that shoot short ranged spores. They sleep during the day.",
    "Planterns light up an area, letting you see through fog.",
    "Cactuses shoot spikes that can hit both ground and air targets.",
    "Blovers blow away all balloon zombies and fog.",
    "Split Peas shoot peas forward and backwards.",
    "Starfruits shoot stars in 5 directions.",
    "Pumpkins protect plants that are within their shells.",
    "Magnet-shrooms remove helmets and other metal objects from zombies. They sleep during the day.",
    "Cabbage-pults hurl cabbages at the enemy, lobbing them over obstacles.",
    "Flower Pots let you plant on the roof.",
    "Kernel-pults fling corn kernels and spot-butter at zombies.",
    "Use Coffee Beans to wake up sleeping mushrooms.",
    "Garlic diverts zombies into other lanes.",
    "Umbrella Leafs protect nearby plants from bungees and catapults.",
    "Marigolds give you silver and gold coins.",
    "Melon-pults do heavy damage to groups of zombies."};

// Descriptions indexed by ZombieType (NORMAL_TYPE = 0 .. YETI_TYPE =
// COUNT_ZOMBIE_TYPE-1; the clone appends YETI after Zomboss, see
// game_param.hpp). Names come from the registry table above.
static const char *ZOMBIE_DESCS[COUNT_ZOMBIE_TYPE] = {
    "Toughness: low. This zombie loves brains. Can't get enough. Regular garden-variety zombie.",
    "Toughness: low. Flag Zombie marks the arrival of a huge pile or 'wave' of zombies.",
    "Toughness: medium. His traffic cone headpiece makes him twice as tough as normal zombies.",
    "Toughness: high. His bucket hat makes him extremely resistant to damage.",
    "Toughness: medium. His screen door is an effective shield.",
    "Toughness: medium, fast. The pole vaulting zombie vaults over the first plant it runs into with a single bound.",
    "Toughness: medium. His newspaper provides limited defense. He gets furious when it shreds.",
    "Toughness: very high, fast. Football Zombie is a big, fast player on a bloody field.",
    "Toughness: medium. This zombie loves to boogie, calling backup dancers from the ground.",
    "Toughness: low. Backup Dancers appear in sets of four whenever the Dancing Zombie rocks out.",
    "Toughness: same as a normal zombie. The ducky tube lets this zombie float on water.",
    "Toughness: low. Snorkel Zombies can swim underwater until they reach your plants.",
    "Toughness: high. The zomboni resurfaces your pool and leaves an ice trail behind.",
    "Toughness: low each. A bobsled team slides in a group of four down the zomboni's ice trail.",
    "Toughness: medium. Dolphin Rider Zombies use dolphins to exploit weaknesses in your pool defense.",
    "Toughness: medium. This zombie shivers because of the bang of its exploding box.",
    "Toughness: medium. Balloon Zombie floats above the fray, immune to ground attacks.",
    "Toughness: medium. Digger Zombie tunnels under your defenses and pops up behind them.",
    "Toughness: medium. Pogo Zombie bounces right over your plants.",
    "Toughness: medium. Bungee Zombie descends from above and loves taking risks.",
    "Toughness: medium. Ladder Zombie's ladder lets other zombies climb over wall plants.",
    "Toughness: high. Catapult Zombie lobs basketballs at your plants from afar.",
    "Toughness: extremely high. Gargantuar crushes plants with a telephone pole and throws an imp when hurt.",
    "Toughness: low. Imp is a tiny zombie hurled by Gargantuar deep into your defenses.",
    "Toughness: extreme. Dr. Zomboss rules all zombies, from behind the safety of his giant robot.",
    "Toughness: high. A rare and curious creature. Little is known about the Zombie Yeti other than his name, a birth date, social security number, educational history, past work experience, and sandwich preference (roast beef and Swiss)."};

const char *almanac_plant_name(int type)
{
    if (type < 0 || type >= PLANT_COUNT || PLANT_NAMES[type] == nullptr)
        return "???";
    return PLANT_NAMES[type];
}

const char *almanac_plant_desc(int type)
{
    if (type < 0 || type >= PLANT_COUNT)
        return "";
    return PLANT_DESCS[type];
}

const char *almanac_zombie_name(int type)
{
    if (type < 0 || type >= COUNT_ZOMBIE_TYPE || ZOMBIE_NAMES[type] == nullptr)
        return "???";
    return ZOMBIE_NAMES[type];
}

const char *almanac_zombie_desc(int type)
{
    if (type < 0 || type >= COUNT_ZOMBIE_TYPE)
        return "";
    return ZOMBIE_DESCS[type];
}
