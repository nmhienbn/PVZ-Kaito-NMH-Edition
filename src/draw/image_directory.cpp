#include "ImageType.hpp"

/*
all image in game

Rows come from IMAGE_LIST (image_manifest.hpp): each X(name, ...) expands to
one Image(...) entry here and one enum member in ImageType.hpp, so the table
and the enum cannot fall out of alignment.
*/
#define IMAGE_ARRAY_ENTRY(name, ...) Image(__VA_ARGS__),

const Image all_img[] = {
    IMAGE_LIST(IMAGE_ARRAY_ENTRY)
};

#undef IMAGE_ARRAY_ENTRY

// The loading screen reports real per-file progress against
// COUNT_USED_DIRECTORY; make sure the count can never drift from the array.
static_assert(sizeof(all_img) / sizeof(all_img[0]) == COUNT_USED_DIRECTORY,
              "all_img must have exactly COUNT_USED_DIRECTORY entries");
