#pragma once
#include "music/music.hpp"
#include "draw/rsdl.hpp"
#include "elements/zombies/zombie_struct.hpp"

class Pea
{
private:
    int type;
    int row;
    int x_location;
    int directory_num;
    int explode;
    int pea_width, pea_height;

    bool has_reached_zombie(const Zombie &zombie);

public:
    Pea();
    Pea(int _type, int _row, int _x);
    bool operator<(const Pea &other) const;
    void move();
    bool apply_hitting_zombie(Zombie &zombie, vector<ZombiePart> &zombie_parts);
    bool has_exploded();
    bool is_disappeared();
    void display();
    void display_shadow();
};

void handle_pea_zombie_encounter(vector<Pea> &peas, vector<Zombie *> &zombies, vector<ZombiePart> &zombie_parts);
void move_peas(vector<Pea> &peas);