#include "announcer.hpp"
#include "draw/rsdl.hpp"
#include "music/music.hpp"

extern Window win;
extern int game_state;
const int CAP_ANNOUNCER_CLK_COUNT = 30;

Announcer::Announcer() : Announcer(NULL_DIRECTORY, NULL_MUSIC_DIRECTORY)
{
    // Delegating ctor: the old body used to CONSTRUCT a temporary
    // Announcer(NULL, NULL) and drop it, leaving this object uninitialized.
}

Announcer::Announcer(const int &img_directory, const int &sound_directory)
{
    this->img_directory = img_directory;
    this->sound_directory = sound_directory;
    play_sound_effect(sound_directory);
    this->remain_time = ANNOUNCER_CLK_COUNT;
    this->state = ANNOUNCING;
}

void Announcer::display()
{
    if (remain_time <= 0)
        return;
    int alpha = 255;
    double ratio = 1.0;
    double t = (1.0 - 3 * (ANNOUNCER_CLK_COUNT - CAP_ANNOUNCER_CLK_COUNT)) / CAP_ANNOUNCER_CLK_COUNT;
    if (remain_time > ANNOUNCER_CLK_COUNT - CAP_ANNOUNCER_CLK_COUNT)
    {
        alpha = 255 * (ANNOUNCER_CLK_COUNT - remain_time) / CAP_ANNOUNCER_CLK_COUNT;
        ratio = remain_time * (1.0 - t) / (ANNOUNCER_CLK_COUNT - CAP_ANNOUNCER_CLK_COUNT) + t;
    }
    win.set_texture_alpha(img_directory, alpha);
    win.draw_png_center(img_directory, ratio);
    // Auto-FPS: the countdown tracks LOGIC ticks, not rendered frames --
    // is_finished() (remain_time exhausted) gates the wave-advance state
    // machine (elements_actions.cpp), so the uncapped render rate must not
    // burn it down early. remain_time may dip below 0 inside one multi-tick
    // frame; both tests are <= 0, so that is harmless. Pause freezes it too
    // (logic_ticked holds a stale value while paused, so IS_PAUSED is
    // checked explicitly).
    for (int _lt = logic_ticked;
         _lt > 0 && check_status(game_state, IS_PAUSED) == false; _lt--)
        remain_time--;
}

bool Announcer::is_finished()
{
    return remain_time <= 0;
}
