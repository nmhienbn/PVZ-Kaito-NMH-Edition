#pragma once
#include "../bullet.hpp"

class Pea : public Bullet
{
public:
    // dir=-1 flies left (Split Pea's rear head).
    Pea(int _row, int _x, int _dir = 1);
    void move();
    void display();
    void crash_effect();
    void make_explode();
    void apply_torchwood(const int &prev_x);
    // Test hook: lit by torchwood + the fire reanim frame/rate state.
    bool fire_state_for_test(float &fire_frame, float &fire_fps) const;

private:
    // Authoritative float position (the original keeps projectiles at float
    // mPosX; repvz Projectile::UpdateNormalMotion). x_location is the
    // lround()'d copy collision/render consume.
    float x_pos;
    // Fire-pea reanim state (valid while directory_num == FIRE_PEA_DIRECTORY):
    // doc-local frame and this shot's orbit rate in fps (repvz
    // RandRangeFloat(50, 80), Projectile.cpp:1226).
    float fire_t = 0.0f;
    float fire_rate = 60.0f;
};

/*Torchwood crossing test shared by Pea (ignite) and SnowzPea (melt).*/
bool crosses_torchwood(const int &row, const int &prev_x, const int &cur_x);