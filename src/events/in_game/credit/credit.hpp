#pragma once

#include <vector>

void display_credit();

void display_sod_roll();

// --- seed-chooser hand-off (rework 2) ---------------------------------------

// The seed chooser ran the credit pan itself (chooser CREDIT_PAN / CREDIT_OUT
// states) and marks it here when LET'S ROCK commits; the next
// display_credit() then skips its own pan and goes straight to the sod roll
// + READY-SET-PLANT. One shot: consumed by display_credit().
void credit_mark_chooser_pan();
// True while the chooser pan is pending consumption (test hook).
bool credit_chooser_pan_pending();

// --- sod-roll lawn expansion (day frontyard 1-1..1-10) ---------------------

// True when this level's lawn gained sod rows vs. the previous day level —
// the original unrolls sod on adventure levels 1, 2 and 4 (repvz
// CutScene.cpp:692); the sod roll plays then.
bool sod_lawn_expands();

// Background the intro pan shows: the previous stage when the roll is about
// to play (the pre-expansion lawn; bare dirt for 1-1), otherwise the level's
// own background.
int sod_prev_background();

// Rows the sod actually unrolls on this level: the lanes gained vs the
// previous day stage (1-1: {2} onto bare dirt; 1-2 and 1-4 each gain a top
// and a bottom lane -> two strips). Empty when the lawn does not expand.
// repvz ties the SODROLL rollers to exactly these lanes (CutScene.cpp:
// 1216-1234: level 1 one roller, levels 2 and 4 two).
std::vector<int> sod_roll_rows();

// Horizontal span of the unrolling sod strip in screen px (repvz
// Board::DrawBackdrop clip rects): the roll head travels x0 .. x0+w.
struct SodStrip
{
    int x0;
    int w;
};
SodStrip sod_strip_geom();

// Draw one frame of the classic sod-roll reveal over the base background
// (drawn by the caller): per gained row, the new lawn band clipped to
// [0, strip.x0 + strip.w*prog), band height = the row's own height, with the
// roll + cap sprites riding the reveal edge on those rows. prog == 1 draws
// the plain new background.
void draw_sod_roll_frame(float prog, int clk);
