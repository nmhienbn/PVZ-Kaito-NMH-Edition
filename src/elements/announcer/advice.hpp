#pragma once
#include <functional>
#include <string>
#include <vector>
using std::string;
using std::vector;

/*
Tutorial advice state machine for Adventure levels 1-1, 1-2, 1-3
(PvZ1 GOTY LawnStrings script). A banner of small handwritten text is drawn
at the bottom-center of the screen; the first steps also bob a green arrow
above the target seed packet. Level 1-1 additionally gates the first zombie:
next_wave_clk is pinned until two peashooters are planted.

States are ordered so that the zombie gate of 1-1 opens exactly when the
script reaches L11_NICELY_DONE (2nd peashooter planted).

Other systems (Dave intros, shovel advice on 1-5, ...) can drive the module
through the manual channel:
    level.advice.show(text);                // one-shot banner
    level.advice.add_trigger(cond, text);   // fire a banner when cond() holds
*/
enum AdviceState
{
    ADVICE_INACTIVE = 0,

    // Level 1-1 script (keep this block ordered: state < L11_NICELY_DONE
    // means the first zombie wave stays gated)
    L11_CLICK_PACKET = 1, // "Click on a seed packet to pick it up!" + arrow
    L11_CLICK_GRASS,      // "Click on the grass to plant your seed!"
    L11_NEXT_PLANT,       // "Excellent! You've collected enough ..."
    L11_CLICK_SUN,        // "Click on the falling sun to collect it!"
    L11_KEEP_COLLECTING,  // "Keep on collecting sun! ..."
    L11_NICELY_DONE,      // "Nicely done!" (2nd peashooter planted, gate opens)
    L11_DONT_LET_ZOMBIES, // "Don't let the zombies reach your house!"
    L11_DONE,

    // Level 1-2 script
    L12_IMPORTANT = 10, // "Sunflowers are an extremely important plant!" + arrow
    L12_PLANT_THREE,    // "Try to plant at least 3 of them!"
    L12_IMPROVES,       // "Planting at least 3 sunflowers improves ..."
    L12_DONE,

    // Level 1-3 script
    L13_PLANT_THREE = 20, // "Try to plant at least 3 sunflowers!"
    L13_IMPROVES,         // "Planting sunflowers will improve your chances ..."
    L13_DONE,
};

enum AdviceStyle
{
    ADVICE_STYLE_BANNER = 0, // small bar at the bottom-center of the screen
};

const int ADVICE_CLK_PERSISTENT = -1; // banner stays until the state moves on
const int ADVICE_DEFAULT_TIME = 240;  // ~4 s at 60 FPS

class Advice
{
public:
    int level_num = 0; // 0: inactive
    int state = ADVICE_INACTIVE;

    // Script banner (driven by the state machine); script_clk ==
    // ADVICE_CLK_PERSISTENT keeps it up, 0 hides it.
    string script_text;
    int script_clk = 0;

    void init(int lvl_num); // called by load_level()
    void update();          // every game tick (handle_changes)
    void display();         // every rendered frame (display_game_announce)

    // Tutorial 1-1 holds the sky sun while the player hasn't planted the
    // first peashooter yet (repvz Board.cpp:5417-5421: UpdateSunSpawning
    // returns early during TUTORIAL_LEVEL_1_PICK_UP/PLANT_PEASHOOTER when
    // mPlants is empty). Read by the sky-sun spawner in elements_actions.
    bool holds_sky_sun();

    // Manual channel for other tutorials: drawn with priority over the
    // script banner while active.
    void show(const string &text, AdviceStyle style = ADVICE_STYLE_BANNER,
              int ticks = ADVICE_DEFAULT_TIME);
    void hide();
    bool is_manual_active() const;
    // Fire a manual banner once as soon as cond() ticks true and the manual
    // channel is free.
    void add_trigger(const std::function<bool()> &cond, const string &text,
                     int ticks = ADVICE_DEFAULT_TIME);

private:
    bool arrow_on = false;
    int arrow_packet = 0; // index into player.seed_packets

    string manual_text;
    int manual_clk = 0;
    AdviceStyle manual_style = ADVICE_STYLE_BANNER;

    struct Trigger
    {
        std::function<bool()> cond;
        string text;
        int ticks;
    };
    vector<Trigger> triggers;

    bool sunflowers_less_than_3_hint_shown = false;

    void set_banner(const string &text, int ticks);
    void set_arrow(bool on, int packet_index = 0);
    void update_level_1();
    void update_level_2();
    void update_level_3();
    void display_arrow();
    void display_banner(const string &text);
};
