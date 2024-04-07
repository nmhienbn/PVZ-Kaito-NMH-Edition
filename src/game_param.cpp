#include "game_param.hpp"

#include <nlohmann/json.hpp>
using namespace nlohmann;

static ifstream f("./dat/parameters.json");
json j = json::parse(f);

//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

// Playground elements size
const int BLOCK_WIDTH = j["BLOCK_WIDTH"];
const int BLOCK_HEIGHT = j["BLOCK_HEIGHT"];
const int HORIZ_BLOCK_COUNT = j["HORIZ_BLOCK_COUNT"];
const int VERT_BLOCK_COUNT = j["VERT_BLOCK_COUNT"];

const int X_UPPER_LEFT = j["X_UPPER_LEFT"];
const int Y_UPPER_LEFT = j["Y_UPPER_LEFT"];

const int ELEMENT_WIDTH = j["ELEMENT_WIDTH"];
const int ELEMENT_HEIGHT = j["ELEMENT_HEIGHT"];

//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
// Speed properties: increase or decrease the following 9 define values to speed up or speed down entities in the game
const int FPS = j["FPS"];
// ticks per frame
const int TICKS_PER_FRAME = 1000 / FPS;
// Sun from sky frequency
const int SUN_GEN_SKY_CLK_COUNT = j["SUN_GEN_SKY_CLK_COUNT"];
// Sun from sunflower frequency
const int SUN_GEN_SUNFLOWER_CLK_COUNT = j["SUN_GEN_SUNFLOWER_CLK_COUNT"];
// Time appear of first wave of zombies
const int FIRST_WAVE_CLK_COUNT = j["FIRST_WAVE_CLK_COUNT"];
// Time between zombies' different groups
const int ZOMBIE_CREATE_CLK_COUNT = j["ZOMBIE_CREATE_CLK_COUNT"];
// Zombie bite plant frequency
const int BITE_CLK_COUNT = j["BITE_CLK_COUNT"];
// Zombie moving frequency
const int ZOMBIE_CLK_COUNT = j["ZOMBIE_CLK_COUNT"];

// Announcer appear time
const int ANNOUNCER_CLK_COUNT = j["ANNOUNCER_CLK_COUNT"];
// Sun change color frequency
const int SUN_CHANGE_COLOR_CLK_COUNT = j["SUN_CHANGE_COLOR_CLK_COUNT"];
// Max time blink when attack
const int MAX_TIME_BLINK = j["MAX_TIME_BLINK"];
// Max time being cold
const int MAX_COLD_TIME = j["MAX_COLD_TIME"];
// Max time that a sun can appear
const int SUN_APPEAR_LIMIT = j["SUN_APPEAR_LIMIT"];

//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
// Initial sun count and normal sun value
const int INIT_SUN_COUNT = j["INIT_SUN_COUNT"];
const int NORMAL_SUN_VALUE = j["NORMAL_SUN_VALUE"];
const int INF = j["INF"];

//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

// Moving speed of some elements
const int ZOMBIE_INIT_X = WINDOW_WIDTH;
const int ZOMBIE_DX = j["ZOMBIE_DX"];
const int ZOMBIE_COLD_DX = j["ZOMBIE_COLD_DX"];
const int PEA_DX = j["PEA_DX"];
const int SUN_DY = j["SUN_DY"];
const int CLICKED_SUN_D = j["CLICKED_SUN_D"];

ifstream plants_json(".\\dat\\plants.json");
auto plants = ordered_json::parse(plants_json);

const vector<int> PLANT_SUN_COST = [](ordered_json &plants)
{
    vector<int> plant_sun_cost;
    for (const auto &plant : plants.items())
    {
        plant_sun_cost.push_back(plant.value()["cost"]);
    }
    return plant_sun_cost;
}(plants);

const vector<int> PLANT_LOADING_TIME = [](ordered_json &plants)
{
    vector<int> plant_loading_time;
    for (const auto &plant : plants.items())
    {
        plant_loading_time.push_back(plant.value()["loading_time"]);
    }
    return plant_loading_time;
}(plants);

const vector<int> PLANT_HEALTH_LIMIT = [](ordered_json &plants)
{
    vector<int> plant_health_limit;
    for (const auto &plant : plants.items())
    {
        plant_health_limit.push_back(plant.value()["health"]);
    }
    return plant_health_limit;
}(plants);

ifstream zombies_json(".\\dat\\zombies.json");
auto zombies = ordered_json::parse(zombies_json);

const vector<set<int>> ZOMBIE_HEALTH_LIMIT = [](ordered_json &zombies)
{
    vector<set<int>> zombie_health_limit;
    for (const auto &zombie : zombies.items())
    {
        set<int> health_limit(zombie.value().begin(), zombie.value().end());
        zombie_health_limit.push_back(health_limit);
    }
    return zombie_health_limit;
}(zombies);