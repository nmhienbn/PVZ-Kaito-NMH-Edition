#include "unlock_plant.hpp"
#include "draw/atlas_font.hpp"
#include "draw/plant_icon_dir.hpp"
#include "elements/Level/Level.hpp"
#include "elements/button/button.hpp"
#include "elements/elements.hpp"
#include "events/almanac/almanac.hpp"

extern int game_state;
extern bool quit;
extern Level level;
extern Elements game_characters;
extern Window win;

/*New-plant presentation: the plant rises onto the award-screen pedestal as
its idle render (the almanac's reanim preview) with a light glow, sparkles
and a white fade-in -- the retail AwardScreen fades in from white over the
backdrop with the new seed on the pedestal
(.tmp_research/repvz/AwardScreen.cpp:25 mFadeInCounter=180, Draw at 407-409,
DrawAwardSeed at 259-273); the choir sting (new_plant.mp3) plays from the
first frame. unlock_plant() is pumped once per frame by the main loop /
startup flow, so the intro just advances the session clock below; the
Continue flow on top of it is unchanged.

Layout follows the retail AwardScreen: title in the top header band
(AwardScreen.cpp:253: FONT_DWARVENTODCRAFT24 gold 213,159,43, centered),
the award name in the panel band (254: DWARVENTODCRAFT18YELLOW), the
wrapped description in the paper panel (255: BRIANNETOD16 navy 40,50,90),
and the Bottom button. The title band rides the Replanted header plaque
(SPR_AwardScreen_Header) sized to the backdrop's carved band; title and
name are now the real DwarvenTodcraft18Yellow bitmap atlas (gold fill +
baked keyline, no runtime shadow passes), the description stays Brianne.*/

namespace
{
// SUNFLOWER_TROPHY(-1) is a real award here, so -2 marks "no session".
int s_present_plant = -2;
int s_present_clk = 0;      // frames pumped this session
float s_idle_clock = 0.0f;  // reanim idle time (almanac clock units, ~12 fps)

// Non-plant award screens (shovel/note/item) share the same intro clock
// (fade-in gate) keyed by the award id, -2 = no session.
int s_award_id = -2;
int s_award_clk = 0;

const int PRESENT_TEXT_CLK = 36;    // texts/Continue pop in once the plant shows
const int PRESENT_RISE_CLKS = 42;   // plant slides up from below the window
const int PRESENT_SPARKLE_CLKS = 90;
const int AWARD_FADE_CLKS = 30;     // award screens settle in ~1/2s
const int AWARD_TEXT_CLK = 8;       // paper-note texts right after the black flash

// Pedestal: plant feet land just above the plant-name text line.
const int PRESENT_FOOT_END = 312;
const int PRESENT_FOOT_START = 700; // below the window bottom
const int PRESENT_GLOW_Y = 235;

// Replanted header plaque (1320x216) sized ~1.15x the backdrop's carved
// band (x 168..879, y 8..118 on AwardScreen.png): spans the full band.
// The carved middle strip of the plaque (rows 36..168 of 216) lands on the
// band's own recess where the title is drawn.
const int AWARD_HEADER_W = 810;
const int AWARD_HEADER_H = 133;
const int AWARD_HEADER_X = (WINDOW_WIDTH - AWARD_HEADER_W) / 2;
const int AWARD_HEADER_Y = -2;
const int AWARD_TITLE_Y = 36;    // top edge of the title text
const int AWARD_TITLE_MAX_W = AWARD_HEADER_W - 40;

// Panel band/name/description lines (drawn over the backdrop's carved
// mid band and the tan paper panel).
const int AWARD_NAME_Y = 352;
const int AWARD_DESC_Y = 418;
const int AWARD_DESC_DELTA = 26;
const int AWARD_DESC_SIZE = 20;
const int AWARD_DESC_MAX_W = 360;

// Continue: Replanted basic-button art (426x168) aspect-fit, centered on
// the fixed CONTINUE hit-rect.
const int CONTINUE_BTN_H = 74;
const int CONTINUE_BTN_W = 426 * CONTINUE_BTN_H / 168;

// Retail palette (AwardScreen.cpp:253-255): the title/name lines now come
// from the real DwarvenTodcraft18Yellow atlas (its baked gold face +
// keyline), so the color lives in the glyphs; only the Brianne description
// keeps a TTF color.
const RGB AWARD_DESC_COLOR(40, 50, 90);     // BRIANNETOD16 navy

struct Twinkle
{
    int dx, dy, phase;
};
// Deterministic star ring around the pedestal (stable for framebuffer probes).
const Twinkle PRESENT_TWINKLES[] = {{-150, -60, 0}, {140, -80, 14}, {165, 55, 30},
                                    {-160, 65, 8},  {0, -120, 40}, {60, 105, 22}};

float rise_ease(int f)
{
    if (f >= PRESENT_RISE_CLKS)
        return 1.0f;
    float t = float(f) / PRESENT_RISE_CLKS;
    // ease-out-back: settles from a hair above the pedestal.
    const float c1 = 1.2f;
    return 1.0f + (c1 + 1.0f) * (t - 1.0f) * (t - 1.0f) * (t - 1.0f) + c1 * (t - 1.0f) * (t - 1.0f);
}

void draw_present_glow(int f)
{
    float ramp = f < 12 ? f / 12.0f : 1.0f;
    int pulse = ((f / 8) & 1) ? 25 : 0;
    int alpha = int((f < PRESENT_SPARKLE_CLKS ? 165 : 95) * ramp) + pulse;
    int r = 240 + int(60 * rise_ease(f));
    win.set_texture_alpha(COIN_GLOW_DIRECTORY, alpha > 255 ? 255 : alpha);
    win.draw_png(COIN_GLOW_DIRECTORY, (WINDOW_WIDTH >> 1) - r, PRESENT_GLOW_Y - r, 2 * r, 2 * r);
    win.set_texture_alpha(COIN_GLOW_DIRECTORY, all_img[COIN_GLOW_DIRECTORY].alpha);

    if (f >= PRESENT_SPARKLE_CLKS)
        return;
    int fade = f > PRESENT_SPARKLE_CLKS - 24 ? PRESENT_SPARKLE_CLKS - f : 24; // 1..24
    for (const Twinkle &tw : PRESENT_TWINKLES)
    {
        int cyc = (f + tw.phase) % 48;
        if (cyc >= 24)
            continue;
        int a = (cyc < 12 ? cyc * 21 : 255 - (cyc - 12) * 21) * fade / 24;
        int s = 22 + a * 24 / 255;
        win.set_texture_alpha(DIAMOND_DIRECTORY, a);
        win.draw_png(DIAMOND_DIRECTORY, (WINDOW_WIDTH >> 1) + tw.dx - s / 2,
                     PRESENT_GLOW_Y + tw.dy - s / 2, s, s);
    }
    win.set_texture_alpha(DIAMOND_DIRECTORY, all_img[DIAMOND_DIRECTORY].alpha);
}

// Rising plant: reanim idle render when the type has a skin, the static
// award-screen PNG otherwise (fallback top-y rides the same ease).
void draw_present_plant(int _plant_type, int plant_dir, double png_ratio,
                        int png_y1, int f)
{
    float e = rise_ease(f);
    bool drawn = false;
    if (_plant_type >= 0 && _plant_type < PLANT_COUNT)
    {
        int foot = PRESENT_FOOT_START + int((PRESENT_FOOT_END - PRESENT_FOOT_START) * e);
        drawn = draw_plant_preview(_plant_type, s_idle_clock, WINDOW_WIDTH >> 1, foot);
    }
    if (!drawn && plant_dir > 0)
    {
        int y1 = png_y1 + int((WINDOW_HEIGHT - png_y1) * (1.0f - e));
        win.set_texture_alpha(plant_dir, 255);
        win.draw_png_center(plant_dir, png_ratio, -1, y1);
        win.set_texture_alpha(plant_dir, all_img[plant_dir].alpha);
    }
}

void present_clock_reset(int _plant_type)
{
    if (_plant_type != s_present_plant)
    {
        s_present_plant = _plant_type;
        s_present_clk = 0;
        s_idle_clock = 0.0f;
    }
}

void present_clock_leave()
{
    s_present_plant = -2;
}

// Shrink a centered text to a pixel width (the plaque recess / paper panel).
int fit_text_pt(const string &text, const char *font_addr, int size, int max_w)
{
    int w = 0, h = 0;
    for (; size > 10; size--)
    {
        TTF_SizeText(win.get_font(font_addr, size), text.c_str(), &w, &h);
        if (w <= max_w)
            break;
    }
    return size;
}

// Header plaque + yellow-inset title. Drawn over the backdrop's carved band:
// the plaque's own recess aligns with the band. The title uses the real
// DwarvenTodcraft18Yellow atlas (gold glyphs, baked keyline -- the retail
// AwardScreen header look) scaled to the old TTF-34 ink height (~23 px),
// shrunk to the plaque recess width like the old fit_text_pt.
void draw_award_header(const string &title)
{
    win.draw_png(REPLANTED_AWARD_HEADER_DIRECTORY, AWARD_HEADER_X, AWARD_HEADER_Y,
                 AWARD_HEADER_W, AWARD_HEADER_H);
    const AtlasFont &yellow = AtlasFont::yellow();
    float it = 0.0f, ib = 0.0f;
    yellow.ink_bounds(title, it, ib);
    float scale = 23.0f / (ib - it);
    while (scale > 0.2f && yellow.string_width(title) * scale > AWARD_TITLE_MAX_W)
        scale -= 0.05f;
    yellow.draw_centered(win, title, WINDOW_WIDTH / 2, AWARD_TITLE_Y + 18, scale);
}

// Award name in the carved mid band: DwarvenTodcraft18Yellow at its native
// 18-pt size, exactly the retail AwardScreen's name line.
void draw_award_name(const string &plant_name)
{
    const AtlasFont &yellow = AtlasFont::yellow();
    float scale = 1.0f;
    while (scale > 0.2f && yellow.string_width(plant_name) * scale > AWARD_TITLE_MAX_W)
        scale -= 0.05f;
    yellow.draw_centered(win, plant_name, WINDOW_WIDTH / 2, AWARD_NAME_Y + 14, scale);
}

// Wrapped description lines, navy Brianne centered in the paper panel.
void draw_award_desc(const vector<string> &description)
{
    int y = AWARD_DESC_Y;
    for (const string &des : description)
    {
        if (!des.empty())
        {
            int pt = fit_text_pt(des, BRIANNE_TTF, AWARD_DESC_SIZE, AWARD_DESC_MAX_W);
            win.show_text_center(des, -1, y, AWARD_DESC_COLOR, BRIANNE_TTF, pt);
        }
        y += AWARD_DESC_DELTA;
    }
}

// Continue = Replanted basic button (Over skin on hover) with a readable
// Dwarvesc TTF label (pre-atlas green ink, brightened on hover); hit-rect
// stays the shared CONTINUE (button.cpp:45).
void display_continue_button()
{
    int mx = 0, my = 0;
    SDL_GetMouseState(&mx, &my);
    bool over = CONTINUE.is_mouse_in(mx, my);
    win.draw_png(over ? REPLANTED_BASIC_BUTTON_OVER_DIRECTORY : REPLANTED_BASIC_BUTTON_DIRECTORY,
                 (CONTINUE.x1 + CONTINUE.x2 - CONTINUE_BTN_W) / 2,
                 (CONTINUE.y1 + CONTINUE.y2 - CONTINUE_BTN_H) / 2 - 4,
                 CONTINUE_BTN_W, CONTINUE_BTN_H);

    // READABLE Dwarvesc TTF label (user request: buttons back to English):
    // the pre-atlas green ink + dark shadow, 20 pt like the old
    // CONTINUE.show_text; hover brightens the ink.
    const RGB ink = over ? RGB(120, 255, 120) : RGB(0, 196, 0);
    int tw, th;
    TTF_SizeText(win.get_font(DWARVESC_TTF, 20), "Continue", &tw, &th);
    win.show_text_shadowed("Continue", (CONTINUE.x1 + CONTINUE.x2 - tw) / 2,
                           (CONTINUE.y1 + CONTINUE.y2 - th) / 2 - 4, ink,
                           DWARVESC_TTF, 20, 0, 1, RGB(43, 44, 58));
}

// Award-screen fade-in (the retail AwardScreen settles in from a full
// flash over its first frames): white for the plant/shovel/item screens,
// black for the paper notes (AwardScreen::Draw 407-409 IsPaperNote).
void draw_award_flash(int f, bool paper_note)
{
    if (f >= AWARD_FADE_CLKS)
        return;
    int dir = paper_note ? BLACK_SCREEN_DIRECTORY : WHITE_SCREEN_DIRECTORY;
    win.set_texture_alpha(dir, 210 * (AWARD_FADE_CLKS - f) / AWARD_FADE_CLKS);
    win.draw_png(dir, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    win.set_texture_alpha(dir, all_img[dir].alpha);
}

// Stationary halo behind the shovel/item award spot (the paper panel has
// no art of its own, so the glow marks the pedestal like on the seed).
void draw_award_glow()
{
    win.set_texture_alpha(COIN_GLOW_DIRECTORY, 120);
    win.draw_png(COIN_GLOW_DIRECTORY, (WINDOW_WIDTH >> 1) - 280, PRESENT_GLOW_Y - 280, 560, 560);
    win.set_texture_alpha(COIN_GLOW_DIRECTORY, all_img[COIN_GLOW_DIRECTORY].alpha);
}

void award_screen_leave()
{
    play_music(OPENING_MUSIC_DIRECTORY);
    win.fade_out();
    present_clock_leave();
    s_award_id = -2;
    set_status(game_state, IS_UNLOCKING_PLANT, false);
}
} // namespace

// Continue shared by click and the retail keys (AwardScreen::KeyChar
// space/return/esc -> StartButtonPressed, AwardScreen.cpp:443-447).
#define AWARD_KEYS()                                \
    KEY_PRESS(RETURN, award_screen_leave();)        \
    KEY_PRESS(KP_ENTER, award_screen_leave();)      \
    KEY_PRESS(SPACE, award_screen_leave();)         \
    KEY_PRESS(ESCAPE, award_screen_leave();)


// Plants *unlocked_plant;
// PlantAssets unlocked_plant_assets;

// void display_unlocked_plant()
// {
//     unlocked_plant->display(unlocked_plant_assets);
// }

/*
If player unlock a new plant, display this (f = frames pumped this session;
drives the rise/glow intro, 0 = fully risen for the headless checks).
*/
void display_unlock_plant(const int &_plant_type, int f)
{
    win.clear_renderer();
    win.draw_bg(AWARD_SCREEN_DIRECTORY);
    string title;
    string plant_name;
    vector<string> description;
    draw_present_glow(f);
    if (_plant_type == SUNFLOWER_TROPHY)
    {
        title = "YOU GOT THE GOLD SUNFLOWER TROPHY";
        plant_name = "SUNFLOWER TROPHY";
        description.push_back("Thank you for playing my game!");
        description.push_back("I hope you enjoyed it!");
        draw_present_plant(-1, SUNFLOWER_TROPHY_DIRECTORY, 0.6, 150, f);
    }
    else
    {
        // if (unlocked_plant == nullptr)
        // {
        //     unlocked_plant = init_plant(_plant_type, 0, 0);
        //     unlocked_plant_assets = PlantAssets(1, 100, 100, 100, 100, 0, 0);
        // }
        title = "YOU GOT A NEW PLANT!";
        if (_plant_type >= LILYPAD_TYPE && _plant_type < PLANT_COUNT)
        {
            // Area 3-5 roster (STUB-AREA-AGENT classes; canon almanac texts).
            // Icons come from PLANT_ICON_DIR (draw/plant_icon_dir.hpp).
            static const struct
            {
                const char *name, *line1, *line2;
            } AREA_PLANTS[] = {
                {"LILY PAD", "Lily pads let you plant non-aquatic", "plants on top of them"},                    // LILYPAD_TYPE
                {"SQUASH", "Squashes will smash the first", "zombie that gets close to them"},                 // SQUASH_TYPE
                {"THREEPEATER", "Threepeaters shoot peas", "in three lanes"},                                  // THREEPEATER_TYPE
                {"TANGLE KELP", "Tangle Kelp are aquatic plants that pull the", "first zombie that nears them underwater"}, // TANGLEKELP_TYPE
                {"JALAPENO", "Jalapenos destroy an entire", "lane of zombies"},                                // JALAPENO_TYPE
                {"SPIKEWEED", "Spikeweeds pop tires and hurt any", "zombies that step on them"},               // SPIKEWEED_TYPE
                {"TORCHWOOD", "Torchwoods turn peas that pass through them", "into fireballs that deal double damage"}, // TORCHWOOD_TYPE
                {"TALL-NUT", "Tall-nuts are heavy-duty wall plants", "that can't be vaulted over"},            // TALLNUT_TYPE
                {"SEA-SHROOM", "Sea-shrooms are aquatic plants that", "shoot short ranged spores"},            // SEASHROOM_TYPE
                {"PLANTERN", "Planterns light up an area,", "letting you see through fog"},                    // PLANTERN_TYPE
                {"CACTUS", "Cactuses shoot spikes that can hit", "both ground and air targets"},               // CACTUS_TYPE
                {"BLOVER", "Blovers blow away all balloon", "zombies and fog"},                                // BLOVER_TYPE
                {"SPLIT PEA", "Split peas shoot peas", "forward and backwards"},                               // SPLITPEA_TYPE
                {"STARFRUIT", "Starfruits shoot stars", "in 5 directions"},                                    // STARFRUIT_TYPE
                {"PUMPKIN", "Pumpkins protect plants that", "are within their shells"},                        // PUMPKIN_TYPE
                {"MAGNET-SHROOM", "Magnet-shrooms remove helmets and other", "metal objects from zombies"},    // MAGNETSHROOM_TYPE
                {"CABBAGE-PULT", "Cabbage-pults hurl cabbages", "at the enemy"},                               // CABBAGEPULT_TYPE
                {"FLOWER POT", "Flower Pots let you", "plant on the roof"},                                    // FLOWERPOT_TYPE
                {"KERNEL-PULT", "Kernel-pults fling corn kernels", "and butter at zombies"},                   // KERNELPULT_TYPE
                {"COFFEE BEAN", "Use Coffee Beans to wake up", "sleeping mushrooms"},                          // COFFEEBEAN_TYPE
                {"GARLIC", "Garlic diverts zombies", "into other lanes"},                                      // GARLIC_TYPE
                {"UMBRELLA LEAF", "Umbrella Leaves protect nearby plants", "from bungees and catapults"},      // UMBRELLALEAF_TYPE
                {"MARIGOLD", "Marigolds give you", "silver and gold coins"},                                   // MARIGOLD_TYPE
                {"MELON-PULT", "Melon-pults do heavy damage", "to groups of zombies"},                         // MELONPULT_TYPE
            };
            const auto &p = AREA_PLANTS[_plant_type - LILYPAD_TYPE];
            int plant_dir = PLANT_ICON_DIR[_plant_type];
            plant_name = p.name;
            description.push_back(p.line1);
            description.push_back(p.line2);
            draw_present_plant(_plant_type, plant_dir, 1.2, 170, f);
        }
        else
        {
        int plant_dir = 0;
        switch (_plant_type)
        {
        case PEASHOOTER_TYPE:
            plant_dir = PEASHOOTER_DIRECTORY;
            plant_name = "PEASHOOTER";
            description.push_back("Shoots peas at attacking zombies");
            break;

        case SUNFLOWER_TYPE:
            plant_dir = SUNFLOWER_DIRECTORY;
            plant_name = "SUNFLOWER";
            description.push_back("Give you additional sun");
            break;

        case WALLNUT_TYPE:
            plant_dir = WALLNUT_DIRECTORY;
            plant_name = "WALLNUT";
            description.push_back("Blocks off zombies and");
            description.push_back("protects your other plants");
            break;

        case CHERRYBOMB_TYPE:
            plant_dir = CHERRYBOMB_DIRECTORY;
            plant_name = "CHERRY BOMB";
            description.push_back("Blows up all zombies");
            description.push_back("in a 3x3 tiles area");
            break;

        case SNOWPEA_TYPE:
            plant_dir = SNOWPEA_DIRECTORY;
            plant_name = "SNOW PEA";
            description.push_back("Shoots frozen peas that");
            description.push_back("damage and slow zombies");
            break;

        case POTATOMINE_TYPE:
            plant_dir = POTATOMINE_DIRECTORY;
            plant_name = "POTATO MINE";
            description.push_back("Explodes on contact, but");
            description.push_back("takes time to arm itself");
            break;

        case REPEATER_TYPE:
            plant_dir = REPEATER_DIRECTORY;
            plant_name = "REPEATER";
            description.push_back("Fires two peas at a time");
            break;

        case CHOMPER_TYPE:
            plant_dir = CHOMPER_DIRECTORY;
            plant_name = "CHOMPER";
            description.push_back("Devours zombies whole, but is");
            description.push_back("vulnerable while chewing");
            break;

        case PUFFSHROOM_TYPE:
            plant_dir = PUFFSHROOM_DIRECTORY;
            plant_name = "PUFF-SHROOM";
            description.push_back("Shoots short-ranged spores, but");
            description.push_back("only works at night");
            break;

        case SUNSHROOM_TYPE:
            plant_dir = SUNSHROOM_DIRECTORY;
            plant_name = "SUN-SHROOM";
            description.push_back("Makes small suns at first,");
            description.push_back("then grows up and makes big ones");
            break;

        case FUMESHROOM_TYPE:
            plant_dir = FUMESHROOM_DIRECTORY;
            plant_name = "FUME-SHROOM";
            description.push_back("Sprays fumes that pass through");
            description.push_back("doors and newspapers");
            break;

        case GRAVEBUSTER_TYPE:
            plant_dir = GRAVEBUSTER_DIRECTORY;
            plant_name = "GRAVE BUSTER";
            description.push_back("Plant it on a grave to");
            description.push_back("remove the grave");
            break;

        case HYPNOSHROOM_TYPE:
            plant_dir = HYPNOSHROOM_DIRECTORY;
            plant_name = "HYPNO-SHROOM";
            description.push_back("Makes a zombie turn around");
            description.push_back("and fight for you");
            break;

        case SCAREDYSHROOM_TYPE:
            plant_dir = SCAREDYSHROOM_DIRECTORY;
            plant_name = "SCAREDY-SHROOM";
            description.push_back("Shoots far, but hides when");
            description.push_back("zombies get close");
            break;

        case ICESHROOM_TYPE:
            plant_dir = ICESHROOM_DIRECTORY;
            plant_name = "ICE-SHROOM";
            description.push_back("Freezes every zombie");
            description.push_back("on the lawn for a while");
            break;

        case DOOMSHROOM_TYPE:
            plant_dir = DOOMSHROOM_DIRECTORY;
            plant_name = "DOOM-SHROOM";
            description.push_back("Destroys everything in a huge area,");
            description.push_back("but leaves a crater");
            break;

        default:
            break;
        }
        draw_present_plant(_plant_type, plant_dir, 1.2, 170, f);
        }
    }
    if (f >= PRESENT_TEXT_CLK)
    {
        draw_award_header(title);
        draw_award_name(plant_name);
        draw_award_desc(description);
    }
    // White award fade-in over everything (repvz AwardScreen::Draw,
    // .tmp_research/repvz/AwardScreen.cpp:407-409 -- 255->0; clone settles in
    // 30 frames).
    draw_award_flash(f, false);
}

/*
Display and handle player click on unlocking plant screen.
*/
void unlock_plant(const int &_plant_type)
{
    present_clock_reset(_plant_type);
    display_unlock_plant(_plant_type, s_present_clk);
    play_music(NEW_PLANT_MUSIC_DIRECTORY, 0); // choir sting (idempotent per frame)
    if (s_present_clk >= PRESENT_TEXT_CLK)
        display_continue_button();
    win.update_screen();
    HANDLE_SDL_EVENT(
        QUIT();
        LCLICK({
            if (CONTINUE.is_mouse_in(mouse_x, mouse_y))
                award_screen_leave();
        });
        AWARD_KEYS();

    );
    s_present_clk++;
    s_idle_clock += 12.0f / FPS; // almanac preview cadence (almanac.cpp:1014)
}

/*Shared frame + Continue handling of the non-plant award screens (shovel
after 1-4, zombie note after 1-9). Mirrors unlock_plant(): texts/button
pop in right after the note's black flash (papers have no rising seed).
award_id keys the intro clock so consecutive awards re-play their fade.*/
static void award_screen_pump(int award_id, bool paper_note, void (*draw_award)())
{
    if (award_id != s_award_id)
    {
        s_award_id = award_id;
        s_award_clk = 0;
    }
    draw_award();
    play_music(NEW_PLANT_MUSIC_DIRECTORY, 0);
    if (s_award_clk >= AWARD_TEXT_CLK)
        display_continue_button();
    draw_award_flash(s_award_clk, paper_note);
    win.update_screen();
    HANDLE_SDL_EVENT(
        QUIT();
        LCLICK({
            if (CONTINUE.is_mouse_in(mouse_x, mouse_y))
                award_screen_leave();
        });
        AWARD_KEYS();

    );
    s_award_clk++;
}

/*
Award screen after 1-4: the shovel is a tool, not a seed, so nothing is added
to the seed packets (level_data starts offering it from level 5 on).
*/
static void display_award_shovel()
{
    win.clear_renderer();
    win.draw_bg(AWARD_SCREEN_DIRECTORY);
    draw_award_glow();
    win.set_texture_alpha(SHOVEL_AWARD_DIRECTORY, 255);
    // Feet on the pedestal line (like the seed): 116x125 art at 1.6x.
    win.draw_png_center(SHOVEL_AWARD_DIRECTORY, 1.6, -1, PRESENT_FOOT_END - 200);
    win.set_texture_alpha(SHOVEL_AWARD_DIRECTORY, all_img[SHOVEL_AWARD_DIRECTORY].alpha);
    draw_award_header("YOU GOT THE SHOVEL!");
    draw_award_name("SHOVEL");
    draw_award_desc({"Lets you dig up a plant", "to make room for another plant"});
}

/*
Award screens after 1-9 and 2-9: the zombies' notes. The letter is shown as
art (ZombieNote*.png is the hand-drawn content, no text rendering).
*/
static void display_award_note(const int &note_dir, int note_w, int note_h)
{
    win.clear_renderer();
    win.draw_bg(AWARD_SCREEN_DIRECTORY);
    // Paper frame centered; the hand-drawn content sits inside it (~3/4 down).
    win.draw_png_center(ZOMBIE_NOTE_FRAME_DIRECTORY, 1.0, -1, 80);
    win.draw_png(note_dir, (WINDOW_WIDTH - 709) / 2 + 80, 80 + 70, note_w, note_h);
    if (s_award_clk >= AWARD_TEXT_CLK)
        draw_award_header("YOU FOUND A NOTE:");
}

void unlock_shovel()
{
    award_screen_pump(0, false, display_award_shovel);
}

void unlock_zombie_note()
{
    award_screen_pump(1, true, [] { display_award_note(ZOMBIE_NOTE1_DIRECTORY, 537, 344); });
}

void unlock_zombie_note2()
{
    award_screen_pump(2, true, [] { display_award_note(ZOMBIE_NOTE2_DIRECTORY, 550, 343); });
}

/*
Text-only award screens for the area items that have no art in the asset pack
(almanac after 2-4, car keys after 3-4, taco after 4-4, watering can after
5-4): same layout as the shovel screen, minus the icon.
*/
static void display_award_item(const char *title, const char *name,
                               const char *line1, const char *line2)
{
    win.clear_renderer();
    win.draw_bg(AWARD_SCREEN_DIRECTORY);
    draw_award_glow();
    draw_award_header(title);
    draw_award_name(name);
    draw_award_desc({line1, line2 && *line2 ? line2 : ""});
}

void unlock_almanac()
{
    award_screen_pump(3, false, [] {
        display_award_item("YOU GOT THE ALMANAC!", "SUBURBAN ALMANAC",
                           "Keeps track of all the plants and zombies", "you have encountered");
    });
}

void unlock_carkeys()
{
    award_screen_pump(4, false, [] {
        display_award_item("YOU FOUND THE CAR KEYS!", "CRAZY DAVE'S CAR KEY",
                           "You found Crazy Dave's car key!", "Check out the trunk of his car for neat stuff!");
    });
}

void unlock_taco()
{
    award_screen_pump(5, false, [] {
        display_award_item("YOU FOUND A TACO!", "MAGIC TACO",
                           "It's Crazy Dave's long-lost magic taco!", "He'll surely reward you for it");
    });
}

void unlock_wateringcan()
{
    award_screen_pump(6, false, [] {
        display_award_item("YOU GOT A WATERING CAN!", "WATERING CAN",
                           "Now you can water your plants", "in the Zen Garden");
    });
}
// Dev/headless-check compose (see unlock_plant.hpp): the same visuals the
// pumps present at frame f, minus update_screen(), so the frame can be
// read back to a PNG dump.
void debug_award_frame(int award_id, const int &_plant_type, int f)
{
    if (award_id == 100)
    {
        display_unlock_plant(_plant_type, f);
        if (f >= PRESENT_TEXT_CLK)
            display_continue_button();
        return;
    }
    s_award_clk = f;
    switch (award_id)
    {
    case 101:
        display_award_note(ZOMBIE_NOTE1_DIRECTORY, 537, 344);
        break;
    case 102:
        display_award_note(ZOMBIE_NOTE2_DIRECTORY, 550, 343);
        break;
    case 103:
        display_award_shovel();
        break;
    case 104:
        display_award_item("YOU GOT THE ALMANAC!", "SUBURBAN ALMANAC",
                           "Keeps track of all the plants and zombies", "you have encountered");
        break;
    case 105:
        display_award_item("YOU FOUND THE CAR KEYS!", "CRAZY DAVE'S CAR KEY",
                           "You found Crazy Dave's car key!", "Check out the trunk of his car for neat stuff!");
        break;
    case 106:
        display_award_item("YOU FOUND A TACO!", "MAGIC TACO",
                           "It's Crazy Dave's long-lost magic taco!", "He'll surely reward you for it");
        break;
    case 107:
        display_award_item("YOU GOT A WATERING CAN!", "WATERING CAN",
                           "Now you can water your plants", "in the Zen Garden");
        break;
    default:
        return;
    }
    if (f >= AWARD_TEXT_CLK)
        display_continue_button();
    draw_award_flash(f, award_id == 101 || award_id == 102);
}
