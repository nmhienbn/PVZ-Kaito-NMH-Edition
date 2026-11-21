#pragma once
#include "support.hpp"
#include <map>
#include <string>
#include "image_manifest.hpp"
using namespace std;

#define ZOMBIE_CREDIT_DIRECTORY rand(ZOMBIE_CREDIT1_DIRECTORY, ZOMBIE_CREDIT2_DIRECTORY)
#define ZOMBIE_WALK_DIRECTORY rand(ZOMBIE_WALK1_DIRECTORY, ZOMBIE_WALK2_DIRECTORY)
#define ARMLESS_ZOMBIE_WALK_DIRECTORY rand(ARMLESS_ZOMBIE_WALK1_DIRECTORY, ARMLESS_ZOMBIE_WALK2_DIRECTORY)
#define ZOMBIE_DIE_DIRECTORY rand(ZOMBIE_DIE1_DIRECTORY, ZOMBIE_DIE2_DIRECTORY)

#define FLAG_ZOMBIE_WALK_1_DIRECTORY rand(FLAG_ZOMBIE_WALK1_1_DIRECTORY, FLAG_ZOMBIE_WALK2_1_DIRECTORY)
#define FLAG_ZOMBIE_WALK_2_DIRECTORY rand(FLAG_ZOMBIE_WALK1_2_DIRECTORY, FLAG_ZOMBIE_WALK2_2_DIRECTORY)
#define FLAG_ZOMBIE_WALK_3_DIRECTORY rand(FLAG_ZOMBIE_WALK1_3_DIRECTORY, FLAG_ZOMBIE_WALK2_3_DIRECTORY)

#define DOOR_ZOMBIE_WALK_1_DIRECTORY rand(DOOR_ZOMBIE_WALK1_1_DIRECTORY, DOOR_ZOMBIE_WALK2_1_DIRECTORY)
#define DOOR_ZOMBIE_WALK_2_DIRECTORY rand(DOOR_ZOMBIE_WALK1_2_DIRECTORY, DOOR_ZOMBIE_WALK2_2_DIRECTORY)
#define DOOR_ZOMBIE_WALK_3_DIRECTORY rand(DOOR_ZOMBIE_WALK1_3_DIRECTORY, DOOR_ZOMBIE_WALK2_3_DIRECTORY)

/*
Contain all image used for SDL

The members come from IMAGE_LIST (image_manifest.hpp) -- the SAME list that
builds all_img[] in image_directory.cpp, so an enum name can never drift from
its image row. Enum order and COUNT_USED_DIRECTORY are generated, not edited.
*/
#define IMAGE_ENUM_ENTRY(name, ...) name,
enum ImageType
{
    NULL_DIRECTORY = -1,

    IMAGE_LIST(IMAGE_ENUM_ENTRY)

    COUNT_USED_DIRECTORY
};
#undef IMAGE_ENUM_ENTRY

struct Image
{
    /*
    Contain all image directory corresponding
    */
    string img_dir;
    /*
    The number of sheet, column in sprite of images
    */
    int n_sheet, c_sheet;
    int alpha;
    Image(string _dir);
    Image(string _dir, int _n, int _c);
    Image(string _dir, int _n, int _c, int _a);
};

extern const Image all_img[];
