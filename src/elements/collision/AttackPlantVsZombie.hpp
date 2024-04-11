#include "../plants/plants.hpp"
#include "../zombies/zombie_struct.hpp"

bool check_in_attack_range(const Plants &plant, const Zombie &zombie);
bool check_in_attack_range(const Plants &plant, const vector<Zombie *> &zombies);