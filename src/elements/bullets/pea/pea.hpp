#pragma once
#include "music/music.hpp"
#include "draw/rsdl.hpp"
#include "zombies/zombie_struct.hpp"

class Pea
{
private:
    int type;
    int row;
    int x_location;
    int directory_num;
    int explode;

    bool has_reached_zombie(Zombie &zombie);

public:
    Pea();
    Pea(int _type, int _row, int _x);

    void move();
    bool apply_hitting_zombie(vector<Zombie> &zombies, vector<DeadZombie> &dead_zombies, const int &z_ind);
    bool has_exploded();
    bool is_disappeared();
    void display();
    void display_shadow();
};

void handle_pea_zombie_encounter(vector<Pea> &peas, vector<Zombie> &zombies, vector<DeadZombie> &dead_zombies);
void move_peas(vector<Pea> &peas);