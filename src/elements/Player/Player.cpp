#include "Player.hpp"

bool Player::is_choosing_a_plant()
{
    return SeedPacket::chosen_plant != PLANT_COUNT;
}
