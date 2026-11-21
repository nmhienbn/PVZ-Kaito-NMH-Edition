#pragma once
#include "../zombie_struct.hpp"
#include "game_param.hpp" // CONE_TYPE / BUCKET_TYPE / DOOR_TYPE
#include <map>

/*
Data row driving an ArmoredZombie: everything cone/bucket/door zombies
differ in (an armored zombie soaks straight shots on a helm/shield tier
until health crosses the plain-body cap, then converts to NORMAL_TYPE --
repvz Zombie::TakeDamage's subtractive helm/shield health). The shared
logic lives in ArmoredZombie; the per-type constants are rows here.

    zombie_type        CONE_TYPE / BUCKET_TYPE / DOOR_TYPE
    walk_dir_lo/hi     init() walk-state roll (rand() bounds, one roll)
    dir_w, dir_h       sprite frame size while armored
    credit_dir_lo/hi   make_credit() directory roll
    credit_y_margin    make_credit(): y = rand(0, WINDOW_HEIGHT - margin)
    start_armor_dir    helm overlay sheet (NULL_DIRECTORY for door: the door
                       sheets bake the door into the body frames)
    has_overlay +      legacy sheet helm overlay drawn by render_zombie /
    overlay_*          display_credited; door has none (its own 196x197
    overlay_frame_bias sheets carry the door through the degrade chain)
    reanim_drop_img    reanim tossed-part img on the drop
    sheet_drop_dir     legacy dropped-helm sheet part, NULL_DIRECTORY for door
    drop_dir_w/h       frame size after the drop (door shrinks back to the
                       plain body 166x144; 0 keeps the spawn dims)
*/
struct ArmorSpec
{
    int zombie_type;
    int walk_dir_lo, walk_dir_hi;
    int dir_w, dir_h;
    int credit_dir_lo, credit_dir_hi;
    int credit_y_margin;
    int start_armor_dir;
    bool has_overlay;
    int overlay_w, overlay_h, overlay_x_bias, overlay_y_bias;
    const map<int, int> *overlay_frame_bias;
    int reanim_drop_img;
    int sheet_drop_dir;
    int drop_dir_w, drop_dir_h;
};

/*The row for CONE_TYPE / BUCKET_TYPE / DOOR_TYPE (static storage in
armored_zombie.cpp -- callers only hold references).*/
const ArmorSpec &armor_spec(int type);

/*Helm- (cone/bucket) and carried-shield- (door) armored zombie: init /
credit placement / armor-drop conversion / helm overlay rendering driven
entirely by the ArmorSpec row. Every HP threshold still goes through
health_scale, so big_trouble minis degrade at the /4 marks.*/
class ArmoredZombie : public Zombie
{
public:
    explicit ArmoredZombie(const ArmorSpec &_spec) : spec(_spec) {}
    void init() override;
    void make_credit() override;
    void armor_drop(vector<ZombiePart> &zombie_parts) override;
    void render_zombie() override;
    void display_credited(const int &_minus_x) override;

private:
    const ArmorSpec &spec;
};

/*Thin shells so the spawn factory stays a plain type -> class table
(zombie_factory.hpp): constructing one picks its ArmorSpec row. No logic
of their own -- every behavior difference is in ArmorSpec.*/
class ConeZombie : public ArmoredZombie
{
public:
    ConeZombie() : ArmoredZombie(armor_spec(CONE_TYPE)) {}
};
class BucketZombie : public ArmoredZombie
{
public:
    BucketZombie() : ArmoredZombie(armor_spec(BUCKET_TYPE)) {}
};
class DoorZombie : public ArmoredZombie
{
public:
    DoorZombie() : ArmoredZombie(armor_spec(DOOR_TYPE)) {}
};
