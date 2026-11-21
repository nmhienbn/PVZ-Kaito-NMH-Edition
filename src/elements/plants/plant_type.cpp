#include "plant_type.hpp"
#include "elements/Level/Level.hpp"
#include "elements/elements.hpp"
#include "elements/plants/support_plant.hpp"

extern Level level;
extern Elements game_characters;

static Plants *create_plant(const int &type, const int &row, const int &col)
{
    switch (type)
    {
    case PEASHOOTER_TYPE:
        return new Peashooter(row, col);
    case SUNFLOWER_TYPE:
        return new Sunflower(row, col);
    case WALLNUT_TYPE:
        return new WallNut(row, col);
    case SNOWPEA_TYPE:
        return new SnowPea(row, col);
    case POTATOMINE_TYPE:
        return new PotatoMine(row, col);
    case CHERRYBOMB_TYPE:
        return new CherryBomb(row, col);
    case REPEATER_TYPE:
        return new Repeater(row, col);
    case CHOMPER_TYPE:
        return new Chomper(row, col);
    case PUFFSHROOM_TYPE:
        return new PuffShroom(row, col);
    case SUNSHROOM_TYPE:
        return new SunShroom(row, col);
    case FUMESHROOM_TYPE:
        return new FumeShroom(row, col);
    case GRAVEBUSTER_TYPE:
        return new GraveBuster(row, col);
    case HYPNOSHROOM_TYPE:
        return new HypnoShroom(row, col);
    case SCAREDYSHROOM_TYPE:
        return new ScaredyShroom(row, col);
    case ICESHROOM_TYPE:
        return new IceShroom(row, col);
    case DOOMSHROOM_TYPE:
        return new DoomShroom(row, col);

    // Pool plants (area 3).
    case LILYPAD_TYPE:
        return new LilyPad(row, col);
    case SQUASH_TYPE:
        return new Squash(row, col);
    case THREEPEATER_TYPE:
        return new ThreePeater(row, col);
    case TANGLEKELP_TYPE:
        return new TangleKelp(row, col);
    case JALAPENO_TYPE:
        return new Jalapeno(row, col);
    case SPIKEWEED_TYPE:
        return new SpikeWeed(row, col);
    case TORCHWOOD_TYPE:
        return new Torchwood(row, col);
    case TALLNUT_TYPE:
        return new TallNut(row, col);

    // Fog plants (area 4).
    case SEASHROOM_TYPE:
        return new SeaShroom(row, col);
    case PLANTERN_TYPE:
        return new Plantern(row, col);
    case CACTUS_TYPE:
        return new Cactus(row, col);
    case BLOVER_TYPE:
        return new Blover(row, col);
    case SPLITPEA_TYPE:
        return new SplitPea(row, col);
    case STARFRUIT_TYPE:
        return new Starfruit(row, col);
    case PUMPKIN_TYPE:
        return new Pumpkin(row, col);
    case MAGNETSHROOM_TYPE:
        return new MagnetShroom(row, col);

    // Roof plants (area 5).
    case CABBAGEPULT_TYPE:
        return new CabbagePult(row, col);
    case FLOWERPOT_TYPE:
        return new FlowerPot(row, col);
    case KERNELPULT_TYPE:
        return new KernelPult(row, col);
    case COFFEEBEAN_TYPE:
        return new CoffeeBean(row, col);
    case GARLIC_TYPE:
        return new Garlic(row, col);
    case UMBRELLALEAF_TYPE:
        return new UmbrellaLeaf(row, col);
    case MARIGOLD_TYPE:
        return new Marigold(row, col);
    case MELONPULT_TYPE:
        return new MelonPult(row, col);

    default:
        return nullptr;
    }
}

/*The spawn funnel every path goes through (level pre-planted pots, hand-planted
seeds, conveyor/bowling drops). Roof maps bake the designer's F2-tuned render
offsets as spawn defaults: a flower pot takes its per-(row, col) table entry,
and any plant stacked onto a same-cell pot rides it at the pot offset plus the
passenger delta. Non-roof maps untouched (offsets stay 0).*/
Plants *init_plant(const int &type, const int &row, const int &col)
{
    Plants *plant = create_plant(type, row, col);
    if (plant == nullptr || !level.is_roof())
        return plant;
    if (type == FLOWERPOT_TYPE)
        roof_pot_layout_default(row, col, plant->layout_dx, plant->layout_dy);
    else
    {
        Plants *pot = find_support_plant(game_characters.plants, row, col);
        if (pot != nullptr && pot->get_type() == FLOWERPOT_TYPE)
            roof_passenger_layout_default(row, col, plant->layout_dx, plant->layout_dy);
    }
    return plant;
}