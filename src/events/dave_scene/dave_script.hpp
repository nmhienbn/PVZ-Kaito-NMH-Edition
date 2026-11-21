#pragma once
#include <string>
#include <utility>
#include <vector>
using std::pair;
using std::string;
using std::vector;

/*
Crazy Dave cutscene scripts — verbatim PvZ1 GOTY LawnStrings lines.
The original markup ("{SCREAM}", "{SHOW_WALLNUT}", ...) is resolved into flags
here; the on-screen text is the clean line only.

Pure data + pure helpers on purpose (no game globals): demo/test harnesses can
compile this header alone to check the script tables.
*/
namespace dave
{

enum DaveVoice
{
    DAVE_VOICE_SHORT = 0, // crazydaveshort1-3
    DAVE_VOICE_LONG,      // crazydavelong1-3
    DAVE_VOICE_EXTRALONG, // crazydaveextralong1-3
    DAVE_VOICE_SCREAM,    // crazydavescream{,2}
    DAVE_VOICE_COUNT
};

struct DaveLine
{
    string text;
    DaveVoice voice;
};

// Short lines get the short mumble clips, long ones the long takes.
inline DaveVoice voice_for_length(int len)
{
    if (len <= 28)
        return DAVE_VOICE_SHORT;
    if (len <= 60)
        return DAVE_VOICE_LONG;
    return DAVE_VOICE_EXTRALONG;
}

// Level 1-5, part 1 — before digging (LawnStrings 2400-2406).
inline const vector<DaveLine> &script_intro_1_5()
{
    static const vector<DaveLine> lines = {
        {"Greetings, neighbor!", DAVE_VOICE_SHORT},
        {"The name's Crazy Dave.", DAVE_VOICE_SHORT},
        {"But you can just call me Crazy Dave.", DAVE_VOICE_LONG},
        {"Listen, I've got a surprise for you.", DAVE_VOICE_LONG},
        {"But first, I'll need you to clear your lawn.", DAVE_VOICE_LONG},
        {"Use your shovel and dig up those plants!", DAVE_VOICE_LONG},
        {"LET THE DIGGING COMMENCE!", DAVE_VOICE_SHORT},
    };
    return lines;
}

// Level 1-5, part 2 — the lawn is clear (LawnStrings 2410-2415); the original
// "{SHOW_WALLNUT}" line hands the player a Wall-nut (done code-side) and the
// last two lines carry "{SCREAM}".
inline const vector<DaveLine> &script_bowling_1_5()
{
    static const vector<DaveLine> lines = {
        {"Ok goody, now for the surprise...", DAVE_VOICE_LONG},
        {"We're going BOWLING!", DAVE_VOICE_SHORT},
        {"HERE, TAKE THIS WALL-NUT!!", DAVE_VOICE_SHORT},
        {"Why'd I put a wall-nut in your hand?", DAVE_VOICE_LONG},
        {"Because I'm CRAAAZY!!!!!", DAVE_VOICE_SCREAM},
        {"NOW GO! BOWL ME A WINNER!", DAVE_VOICE_SCREAM},
    };
    return lines;
}

// Level 2-1 (11) — first night level (LawnStrings ADVICE dialog).
inline const vector<DaveLine> &script_intro_2_1()
{
    static const vector<DaveLine> lines = {
        {"Evening.", DAVE_VOICE_SHORT},
        {"Those zombies just won't let up, will they?", DAVE_VOICE_LONG},
        {"You'll notice that fighting zombies at night is different than in the day.", DAVE_VOICE_EXTRALONG},
        {"For one, you won't get any sun falling from the sky.", DAVE_VOICE_LONG},
        {"You can still get sun from sunflowers, though.", DAVE_VOICE_LONG},
        {"And lucky for you, you got yourself some puff-shrooms.", DAVE_VOICE_LONG},
        {"Plant as many of those as you can and you'll be A-O-GOOD.", DAVE_VOICE_LONG},
    };
    return lines;
}

// Shovel advice banners of the 1-5 dig phase (LawnStrings).
inline const char *advice_pick_shovel() { return "Click on the shovel to pick it up!"; }
inline const char *advice_remove_plant() { return "Click on a plant to remove it!"; }
inline const char *advice_keep_digging() { return "Keep digging until your lawn is clear of plants!"; }

// Levels with a Dave intro scene.
inline bool has_intro_scene(const int &level_num)
{
    return level_num == 5 || level_num == 11;
}

// Level-1-5 starts with Dave's 3 peashooters on the lawn (grid coords of the
// original 9x5 board, (col, row)).
inline const vector<pair<int, int>> &preplaced_peashooters_1_5()
{
    static const vector<pair<int, int>> cells_xy = {{5, 1}, {7, 2}, {6, 3}};
    return cells_xy;
}

// Code-keyed rewards after a first-time win (so level JSONs need no entry):
// after 1-4 -> the shovel, after 1-9 -> the zombie note, after 2-9 -> the
// second zombie note. Area 3-5 item rewards (no plant those levels): after
// 2-4 (14) -> the almanac, after 3-5 (25) -> the car keys, after 4-5 (35) ->
// the taco, after 5-5 (45) -> the watering can; the area-final notes after
// 3-9 (29) / 4-9 (39) / 5-9 (49). Plain plant rewards keep flowing through
// level_unlock_new_plant.
enum DaveAward
{
    DAVE_AWARD_NONE = 0,
    DAVE_AWARD_SHOVEL,
    DAVE_AWARD_NOTE,
    DAVE_AWARD_NOTE2,
    DAVE_AWARD_ALMANAC,
    DAVE_AWARD_CARKEYS,
    DAVE_AWARD_TACO,
    DAVE_AWARD_WATERINGCAN,
};

inline DaveAward award_for_level(const int &level_num)
{
    switch (level_num)
    {
    case 4:  return DAVE_AWARD_SHOVEL;
    case 9:  return DAVE_AWARD_NOTE;
    case 14: return DAVE_AWARD_ALMANAC;
    case 19: return DAVE_AWARD_NOTE2;
    case 24: return DAVE_AWARD_CARKEYS;
    case 29: return DAVE_AWARD_NOTE;
    case 34: return DAVE_AWARD_TACO;
    case 39: return DAVE_AWARD_NOTE;
    case 44: return DAVE_AWARD_WATERINGCAN;
    case 49: return DAVE_AWARD_NOTE2;
    default: return DAVE_AWARD_NONE;
    }
}

} // namespace dave
