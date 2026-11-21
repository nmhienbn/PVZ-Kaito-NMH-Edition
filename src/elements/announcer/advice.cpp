#include "advice.hpp"
#include "draw/rsdl.hpp"
#include "elements/Level/Level.hpp"
#include "elements/Player/Player.hpp"
#include "elements/button/button.hpp"
#include "elements/elements.hpp"
#include "elements/plants/plants.hpp"
#include <cstdlib>

extern int clk;
extern Level level;
extern Elements game_characters;
extern Player player;
extern Window win;

// Bottom banner geometry (same lane as the announcer error text, which
// takes priority for the few ticks it is up).
const int ADVICE_BAR_Y = 540;
const int ADVICE_BAR_H = 60;
const int ADVICE_FONT_SIZE = 30;

const int ARROW_WIDTH = 48;
const int ARROW_HEIGHT = 39; // 32x26 source art, scaled x1.5

// 1-1: keep the first wave this many ticks (~1.5 s) away until 2 peashooters
// are planted.
const int L1_ZOMBIE_GATE_CLK = 90;

// 1-2 / 1-3: show the "before the first wave" banner about this many ticks
// (~10 s into the countdown) ahead of the first zombie.
const int FIRST_WAVE_ADVICE_CLK = 600;

/*Hide the current script banner (script moves on by itself).*/
void Advice::set_banner(const string &text, int ticks)
{
    script_text = text;
    script_clk = ticks;
}

void Advice::set_arrow(bool on, int packet_index)
{
    arrow_on = on;
    arrow_packet = packet_index;
}

/*One-shot banner on the manual channel (for Dave intros, shovel advice...).
ADVICE_CLK_PERSISTENT keeps it up until hide()/the next show().*/
void Advice::show(const string &text, AdviceStyle style, int ticks)
{
    manual_text = text;
    manual_style = style;
    manual_clk = ticks;
}

void Advice::hide()
{
    manual_clk = 0;
}

bool Advice::is_manual_active() const
{
    return manual_clk != 0; // ADVICE_CLK_PERSISTENT (-1) counts as active
}

void Advice::add_trigger(const std::function<bool()> &cond, const string &text, int ticks)
{
    triggers.push_back({cond, text, ticks});
}

/*Fresh script for a level. Called right after read_level() so seed packets
are known (the arrow of 1-2 targets the Sunflower packet, not slot 0).*/
void Advice::init(int lvl_num)
{
    level_num = lvl_num;
    script_clk = 0;
    script_text.clear();
    manual_clk = 0;
    manual_text.clear();
    triggers.clear();
    sunflowers_less_than_3_hint_shown = false;
    arrow_on = false;

    switch (level_num)
    {
    case 1:
        state = L11_CLICK_PACKET;
        set_banner("Click on a seed packet to pick it up!", ADVICE_CLK_PERSISTENT);
        set_arrow(!player.seed_packets.empty(), 0);
        break;
    case 2:
    {
        state = L12_IMPORTANT;
        set_banner("Sunflowers are an extremely important plant!", 240);
        int sunflower_packet = -1;
        for (int i = 0; i < (int)player.seed_packets.size(); i++)
            if (player.seed_packets[i].plant_name == SUNFLOWER_TYPE)
                sunflower_packet = i;
        set_arrow(sunflower_packet >= 0, sunflower_packet < 0 ? 0 : sunflower_packet);
        break;
    }
    case 3:
        state = L13_PLANT_THREE;
        set_banner("Try to plant at least 3 sunflowers!", 360);
        break;
    default:
        state = ADVICE_INACTIVE;
        break;
    }
}

/*Count living plants of a type (real game state, no timers).*/
static int count_plants_of_type(const int &type)
{
    int n = 0;
    for (const auto &plant : game_characters.plants)
        if (plant->get_type() == type)
            n++;
    return n;
}

/*A collectable (not clicked, not expired-out) sun is on the lawn.
Level 1-1 has no sunflowers, so every sun is a sky sun.*/
static bool sun_on_lawn()
{
    for (const auto &sun : game_characters.suns)
        if (!sun.is_clicked)
            return true;
    return false;
}

/*True for a few ticks while a clicked sun flies to the sun counter.*/
static bool sun_just_collected()
{
    for (const auto &sun : game_characters.suns)
        if (sun.is_clicked)
            return true;
    return false;
}

void Advice::update()
{
    // Manual channel: countdown, then fire queued triggers once free.
    if (manual_clk > 0)
        manual_clk--;
    if (manual_clk == 0)
        for (auto it = triggers.begin(); it != triggers.end(); ++it)
            if (it->cond())
            {
                show(it->text, ADVICE_STYLE_BANNER, it->ticks);
                triggers.erase(it);
                break;
            }

    if (script_clk > 0)
        script_clk--;

    // 1-1 zombie gate: hold the first wave L1_ZOMBIE_GATE_CLK ticks away
    // until the script passes "Nicely done!" (2 peashooters planted).
    if (level_num == 1 && state >= L11_CLICK_PACKET && state < L11_NICELY_DONE &&
        !level.waves_finished && !level.zombie_has_coming &&
        level.next_wave_clk < L1_ZOMBIE_GATE_CLK)
        level.next_wave_clk = L1_ZOMBIE_GATE_CLK;

    switch (level_num)
    {
    case 1:
        update_level_1();
        break;
    case 2:
        update_level_2();
        break;
    case 3:
        update_level_3();
        break;
    default:
        break;
    }
}

/*1-1 tutorial freeze: no sky sun while the click-packet / click-grass steps
are on and the lawn is still empty (repvz Board.cpp:5417-5421).*/
bool Advice::holds_sky_sun()
{
    if (level_num != 1 || state < L11_CLICK_PACKET || state > L11_CLICK_GRASS)
        return false;
    return count_plants_of_type(PEASHOOTER_TYPE) == 0;
}

void Advice::update_level_1()
{
    int peashooters = count_plants_of_type(PEASHOOTER_TYPE);
    switch (state)
    {
    case L11_CLICK_PACKET:
        if (SeedPacket::chosen_plant != PLANT_COUNT)
        {
            state = L11_CLICK_GRASS;
            set_arrow(false);
            set_banner("Click on the grass to plant your seed!", ADVICE_CLK_PERSISTENT);
        }
        break;
    case L11_CLICK_GRASS:
        if (peashooters >= 1)
        {
            state = L11_NEXT_PLANT;
            set_banner("Excellent! You've collected enough for your next plant!", 240);
            // repvz Board.cpp:5795-5797 (TUTORIAL_LEVEL_1_REFRESH_PEASHOOTER):
            // the first post-plant sky sun is rushed in at mSunCountDown =
            // 400 cs = 240 ticks at 60 tps. Don't shorten an already-sooner
            // countdown.
            if (level.sky_sun_clk > 240)
                level.sky_sun_clk = 240;
        }
        // Clicking the packet again cancels the pick: back to the arrow step.
        else if (SeedPacket::chosen_plant == PLANT_COUNT)
        {
            state = L11_CLICK_PACKET;
            set_arrow(!player.seed_packets.empty(), 0);
            set_banner("Click on a seed packet to pick it up!", ADVICE_CLK_PERSISTENT);
        }
        break;
    case L11_NEXT_PLANT:
        if (peashooters >= 2)
        {
            state = L11_NICELY_DONE;
            set_banner("Nicely done!", 120);
        }
        else if (sun_on_lawn())
        {
            state = L11_CLICK_SUN;
            set_banner("Click on the falling sun to collect it!", ADVICE_CLK_PERSISTENT);
        }
        break;
    case L11_CLICK_SUN:
        if (sun_just_collected())
        {
            state = L11_KEEP_COLLECTING;
            set_banner("Keep on collecting sun! You'll need it to grow more plants!", 300);
        }
        // The sun expired unclicked: wait for the next one, no banner.
        else if (!sun_on_lawn())
        {
            state = L11_NEXT_PLANT;
            set_banner("", 0);
        }
        break;
    case L11_KEEP_COLLECTING:
        if (peashooters >= 2)
        {
            state = L11_NICELY_DONE;
            set_banner("Nicely done!", 120);
        }
        break;
    case L11_NICELY_DONE:
        if (script_clk == 0)
        {
            state = L11_DONT_LET_ZOMBIES;
            set_banner("Don't let the zombies reach your house!", 300);
        }
        break;
    case L11_DONT_LET_ZOMBIES:
        if (script_clk == 0)
            state = L11_DONE;
        break;
    default:
        break;
    }
}

void Advice::update_level_2()
{
    switch (state)
    {
    case L12_IMPORTANT:
        // Planted (or even just picked) the first sunflower: arrow off.
        if (count_plants_of_type(SUNFLOWER_TYPE) > 0 ||
            SeedPacket::chosen_plant == SUNFLOWER_TYPE)
            set_arrow(false);
        if (script_clk == 0)
        {
            state = L12_PLANT_THREE;
            set_banner("Try to plant at least 3 of them!", 300);
        }
        break;
    case L12_PLANT_THREE:
        // Planted a peashooter before 3 sunflowers: nudge the rhythm once.
        if (!sunflowers_less_than_3_hint_shown &&
            count_plants_of_type(PEASHOOTER_TYPE) > 0 &&
            count_plants_of_type(SUNFLOWER_TYPE) < 3)
        {
            sunflowers_less_than_3_hint_shown = true;
            show("The more sunflowers you have, the faster you can grow plants!");
        }
        if (level.zombie_has_coming || level.next_wave_clk <= FIRST_WAVE_ADVICE_CLK)
        {
            state = L12_IMPROVES;
            set_arrow(false);
            set_banner("Planting at least 3 sunflowers improves your chances of "
                       "surviving a zombie attack!",
                       420);
        }
        break;
    case L12_IMPROVES:
        if (script_clk == 0)
            state = L12_DONE;
        break;
    default:
        break;
    }
}

void Advice::update_level_3()
{
    switch (state)
    {
    case L13_PLANT_THREE:
        if (level.zombie_has_coming || level.next_wave_clk <= FIRST_WAVE_ADVICE_CLK)
        {
            state = L13_IMPROVES;
            set_banner("Planting sunflowers will improve your chances of "
                       "surviving the zombie attack!",
                       420);
        }
        break;
    case L13_IMPROVES:
        if (script_clk == 0)
            state = L13_DONE;
        break;
    default:
        break;
    }
}

void Advice::display()
{
    // The manual channel (Dave intros, shovel advice) runs on any level,
    // independent of the 1-1/1-2/1-3 script state.
    if (state == ADVICE_INACTIVE && manual_clk == 0)
        return;
    if (arrow_on)
        display_arrow();
    // The announcer error text (not enough suns, ...) owns the bottom bar.
    if (win.has_announcer_text())
        return;
    if (manual_clk != 0 && !manual_text.empty())
        display_banner(manual_text);
    else if (state != ADVICE_INACTIVE && script_clk != 0 && !script_text.empty())
        display_banner(script_text);
}

/*Green arrow bobbing above the target seed packet (triangle wave, +-6 px).*/
void Advice::display_arrow()
{
    if (arrow_packet < 0 || arrow_packet >= (int)player.seed_packets.size())
        return;
    int bob = abs(clk % 24 - 12) - 6;
    const Button &packet = plant_seed[arrow_packet];
    int x = (packet.x1 + packet.x2 - ARROW_WIDTH) / 2;
    int y = packet.y1 - ARROW_HEIGHT - 4 + bob;
    win.draw_png(DOWN_ARROW_DIRECTORY, x, y, ARROW_WIDTH, ARROW_HEIGHT);
}

/*Small handwritten banner, bottom-center on the translucent black bar.
The font shrinks until the text fits the window.*/
void Advice::display_banner(const string &text)
{
    int w = 0, h = 0;
    int size = ADVICE_FONT_SIZE;
    for (; size > 16; size -= 2)
    {
        TTF_SizeText(win.get_font(BRIANNE_TTF, size), text.c_str(), &w, &h);
        if (w <= WINDOW_WIDTH - 120)
            break;
    }
    win.draw_png(BLACK_SCREEN_DIRECTORY, 0, ADVICE_BAR_Y, WINDOW_WIDTH, ADVICE_BAR_H);
    win.show_text(text, (WINDOW_WIDTH - w) / 2, ADVICE_BAR_Y + (ADVICE_BAR_H - h) / 2,
                  WHITE, BRIANNE_TTF, size);
}
