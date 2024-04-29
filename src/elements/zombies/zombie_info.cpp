#include "zombie_struct.hpp"
#include <fstream>
#include <nlohmann/json.hpp>
using namespace std;
using namespace nlohmann;

static ifstream fzombie("./resources/images/json/zombie.json");
static json zombie = json::parse(fzombie);

const int ZOMBIE_FRAME = zombie["zombie_frame"];
const int ZOMBIE_EXACT_LOCATION = zombie["zombie_exact_location"];
const int ZOMBIE_WIDTH = zombie["zombie_width"];
const int ZOMBIE_HEIGHT = zombie["zombie_height"];

const int ZOMBIE_EATING_FRAME = zombie["zombie_eating_frame"];
const int ZOMBIE_DIE_FRAME = zombie["zombie_die_frame"];
const int ZOMBIE_BURNT_FRAME = zombie["zombie_burnt_frame"];

const int HEAD_ZOMBIE_FRAME = zombie["head_zombie_frame"];
const int HEAD_ZOMBIE_WIDTH = zombie["head_zombie_width"];
const int HEAD_ZOMBIE_HEIGHT = zombie["head_zombie_height"];