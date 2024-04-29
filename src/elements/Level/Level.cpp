#include "Level.hpp"
#include "draw/ImageType.hpp"
#include "music/music.hpp"

Level::Level()
{
    background_directory = BACKGROUND_DIRECTORY;
}

/*Reset level: Clear all zombie waves' information*/
void Level::reset()
{
    announcer.state = ANNOUNCED;
    wave_count = 0;
    cur_wave = 0;
    waves_finished = false;
    zombie_has_coming = false;
    zombie_count = 0;
    used_zombie_count = 0;
    final_wave = -1;
    next_wave_clk = 0;
    announcer = Announcer(NULL_DIRECTORY, NULL_MUSIC_DIRECTORY);
}