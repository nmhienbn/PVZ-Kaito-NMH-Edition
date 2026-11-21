#pragma once
#include <vector>

#define BULLET_EXPLODE_TIME 15

enum BulletType
{
    PEA,
    SNOWZPEA,
    PUFF,
    FUME,
    SPIKE, // cactus spike (hits balloon zombies too)
    STAR,  // starfruit star (flies in 5 directions, crosses lanes)
    LOBBED, // pult plants' arcing shots (cabbage/corn/butter/melon)
    BULLET_TYPE_NUM
};

class Bullet
{
public:
    int type;
    int row;
    int x_location, y_location;
    int directory_num;
    int explode;
    int width, height;
    int angle;
    int slow_down_rate;
    int dmg; // health units per hit (1 = plain pea; torchwood lights it to 2)
    int dir = 1; // horizontal travel sign: +1 flies right, -1 flies left

    // Roof-slope splat (repvz Projectile::CheckForHighGround,
    // Projectile.cpp:331): straight shots fired from the slanted roof columns
    // track a shrinking shadow gap as the roof climbs under them; the tick it
    // drops under the type's threshold, the shot smacks into the shingles.
    bool roof_splats = false;
    int slope_gap0 = 0;  // spawn shadow gap (orig mShadowY - mPosY)
    int slope_cont0 = 0; // roof_slope_offset(x) at spawn

public:
    Bullet(int _type, int _row, int _x);

    bool operator<(const Bullet &other) const;
    bool has_exploded();
    bool is_disappeared();
    void bind_roof_slope();
    bool splats_on_roof(const int &threshold);
    void display_shadow();

    virtual ~Bullet();
    virtual void move() = 0;
    virtual void display() = 0;
    virtual void crash_effect(){};
    virtual void make_explode(){};
};

void move_bullets(std::vector<Bullet *> &bullets);