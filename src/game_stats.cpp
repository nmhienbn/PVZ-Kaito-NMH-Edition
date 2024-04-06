#include <game_stats.hpp>
#include <nlohmann/json.hpp>
using namespace nlohmann;

static ifstream f("./dat/stat.json");
json j = json::parse(f);

const int WINDOW_WIDTH = j["WINDOW_WIDTH"];
const int WINDOW_HEIGHT = j["WINDOW_HEIGHT"];

const int BLOCK_WIDTH = j["BLOCK_WIDTH"];
const int BLOCK_HEIGHT = j["BLOCK_HEIGHT"];
const int HORIZ_BLOCK_COUNT = j["HORIZ_BLOCK_COUNT"];
const int VERT_BLOCK_COUNT = j["VERT_BLOCK_COUNT"];

const int X_UPPER_LEFT = j["X_UPPER_LEFT"];
const int Y_UPPER_LEFT = j["Y_UPPER_LEFT"];

const int ELEMENT_WIDTH = j["ELEMENT_WIDTH"];
const int ELEMENT_HEIGHT = j["ELEMENT_HEIGHT"];

const int FPS = j["FPS"];
const int TICKS_PER_FRAME = 1000 / FPS;
const int SUN_GEN_SKY_CLK_COUNT = j["SUN_GEN_SKY_CLK_COUNT"];
const int SUN_GEN_SUNFLOWER_CLK_COUNT = j["SUN_GEN_SUNFLOWER_CLK_COUNT"];
const int FIRST_WAVE_CLK_COUNT = j["FIRST_WAVE_CLK_COUNT"];
const int ZOMBIE_CREATE_CLK_COUNT = j["ZOMBIE_CREATE_CLK_COUNT"];
const int BITE_CLK_COUNT = j["BITE_CLK_COUNT"];
const int ZOMBIE_CLK_COUNT = j["ZOMBIE_CLK_COUNT"];

const int ANNOUNCER_CLK_COUNT = j["ANNOUNCER_CLK_COUNT"];
const int SUN_CHANGE_COLOR_CLK_COUNT = j["SUN_CHANGE_COLOR_CLK_COUNT"];
const int MAX_TIME_BLINK = j["MAX_TIME_BLINK"];
const int MAX_COLD_TIME = j["MAX_COLD_TIME"];
const int SUN_APPEAR_LIMIT = j["SUN_APPEAR_LIMIT"];

const int INIT_SUN_COUNT = j["INIT_SUN_COUNT"];
const int NORMAL_SUN_VALUE = j["NORMAL_SUN_VALUE"];
const int INF = j["INF"];

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