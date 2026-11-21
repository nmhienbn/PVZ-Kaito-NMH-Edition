#pragma once
#include "draw/rsdl.hpp"

/*
Set game mouse cursor.

2-5 Whack-a-Zombie: the cursor becomes the wooden mallet for the whole level
(original Board hides the system cursor and draws REANIM_HAMMER at the mouse;
here it is a hardware color cursor). A click plays a quick swing: impact pose
then two recovery frames back to the idle pose (original restarts the cursor
reanim's anim_whack_zombie on MouseDownWhackAZombie).
*/
class Mouse
{
    SDL_Cursor *Cursor;

    // Whack mallet frames (resources/images/Cursor/hammer_*.png, composed from
    // the 2Pak Hammer reanim by tools/make_hammer_cursor.py): idle pose plus
    // the impact and two recovery frames of the swing.
    SDL_Cursor *hammer_cursor[4]; // NULL while any frame art failed to load
    int hammer_frame;             // -1: idle; 0..5 runs SWING_SEQUENCE
    bool hammer_active;           // mallet currently shown

    void apply();

public:
    Mouse();

    ~Mouse();

    void set_cursor();
    /*Pick normal-vs-mallet cursor for the current screen and advance the
    swing animation; call once per main-loop iteration. advance_ticks is
    the number of logic ticks covered by the current rendered frame
    (auto-FPS: during gameplay the loop iterates at the uncapped render
    rate, so the swing advances per logic tick, 0 freezes it).*/
    void update(const bool &whack_level, const int &advance_ticks = 1);
    /*Restart the mallet swing (whack click, hit or miss).*/
    void start_swing();
};

/*The one game cursor. Function-local so it is constructed lazily after SDL
is up (and so non-game mains like the demo checks can link without it).*/
Mouse &mouse_cursor();
