#include "lose.hpp"
#include "draw/render_elements.hpp"
#include "elements/Level/Level.hpp"
#include "elements/button/button.hpp"
#include "elements/elements.hpp"
#include "elements/mower/mower.hpp"
#include "events/in_game/pause_menu/pvz1_ui.hpp"
#include "music/music.hpp"
#include "player/player_data.hpp"
#include "game_param.hpp" // ZOMBIE_HEALTH_LIMIT

extern int game_state;
extern bool quit;
extern Level level;
extern Elements game_characters;
extern Window win;
static bool has_display_lost = false;

/*Continue footer: retail GameOverDialog is a LawnDialog with a single stone
footer button (repvz LawnDialog.cpp:GameOverDialog) over the ZOMBIES ATE
YOUR BRAINS banner; the clone keeps its banner + "Continue" flow, now drawn
as the same 209x46 3-slice stone button the confirms use.*/
const Button LOSE_CONTINUE(WINDOW_WIDTH / 2 - 104, WINDOW_WIDTH / 2 + 105, 540, 540 + 46);

/*Lose foley clocks on the 120-frame (2 s) banner fade. repvz
CutScene::UpdateZombiesWon (CutScene.cpp:1505-1546, constants at 57-63) runs
an 11 s cutscene: two FOLEY_CHOMPs 0.9 s / 0.4 s before the banner pops,
FOLEY_SCREAM with the banner. The clone's compressed take: the scream fires
at the fade start (banner appearance), the chomps as the banner settles.*/
const int LOSE_CHOMP_CLK1 = 70;
const int LOSE_CHOMP_CLK2 = 100;
// Hold on the finished frame so the door layering reads before the Continue
// button appears (retail sits on the banner from 6 s to the 11 s GameOverDialog).
const int LOSE_BANNER_HOLD_CLK = 45;

/*Per-map gameover house art: IMAGE_BACKGROUNDn_GAMEOVER_MASK (the door frame
showing the interior dark/open) + _INTERIOR_OVERLAY (lit interior bitmap whose
opaque core sits inside the mask's door opening), repvz
Board::DrawHouseDoorTop/Bottom (Board.cpp:6868-6893). Retail draws them in
board coords after the cutscene pans left to the house; the clone has no
camera -- draw_bg always samples the art from the house end (x 0..1050 of the
1400-px image), so the usual clone x = retail x + 220 applies. Sizes are the
PNGs' native pixels (drawn 1:1).*/
struct GameoverArt
{
    int dir;
    int x, y, w, h;
};
struct GameoverHouseArt
{
    GameoverArt overlay; // lit interior; dir == NULL_DIRECTORY: none (roof/boss)
    GameoverArt mask;    // door frame, drawn OVER the interior
};

/*level.map_type is level_data.cpp's background_map index: 0-2 day frontyard,
3 night frontyard, 4 day pool, 5 fog pool, 6 roof, 7 night roof (boss).*/
static GameoverHouseArt gameover_house_art()
{
    switch (level.map_type)
    {
    case 3:
        return {{BG2_GAMEOVER_INTERIOR_OVERLAY_DIRECTORY, 95, 196, 108, 279},
                {BG2_GAMEOVER_MASK_DIRECTORY, 92, 207, 111, 241}};
    case 4:
        return {{BG3_GAMEOVER_INTERIOR_OVERLAY_DIRECTORY, 49, 241, 48, 204},
                {BG3_GAMEOVER_MASK_DIRECTORY, 48, 234, 48, 218}};
    case 5:
        return {{BG4_GAMEOVER_INTERIOR_OVERLAY_DIRECTORY, 48, 246, 50, 200},
                {BG4_GAMEOVER_MASK_DIRECTORY, 47, 133, 184, 341}};
    case 6:
        return {{NULL_DIRECTORY, 0, 0, 0, 0},
                {BG5_GAMEOVER_MASK_DIRECTORY, 0, 81, 220, 110}};
    case 7:
        return {{NULL_DIRECTORY, 0, 0, 0, 0},
                {BG6_GAMEOVER_MASK_DIRECTORY, 0, 81, 220, 110}};
    default:
        return {{BG1_GAMEOVER_INTERIOR_OVERLAY_DIRECTORY, 94, 225, 70, 195},
                {BG1_GAMEOVER_MASK_DIRECTORY, 90, 202, 113, 248}};
    }
}

/*Frozen dimmed lawn, interior overlay at the door opening, door-frame mask
over it, banner on top -- retail's DrawHouseDoorBottom (under) / Top (over)
split (Board.cpp:6146, 6697); the interior bitmap is opaque where it overlaps
the frame, so drawing it below keeps the frame visible.*/
static void draw_lose_frame(double banner_ratio)
{
    win.draw_bg(level.background_directory);
    display_game_paused_elements();
    GameoverHouseArt art = gameover_house_art();
    if (art.overlay.dir != NULL_DIRECTORY)
        win.draw_png(art.overlay.dir, art.overlay.x, art.overlay.y, art.overlay.w, art.overlay.h);
    win.draw_png(art.mask.dir, art.mask.x, art.mask.y, art.mask.w, art.mask.h);
    win.draw_png_center(LOSING_MESSAGE_DIRECTORY, banner_ratio);
}

/*
If player lose, display this.
*/
void display_losing_message()
{
    set_status(game_state, IS_PAUSED, true);
    // FOLEY_SCREAM as the banner fades in (repvz CutScene.cpp:1530).
    play_sound_effect(SCREAM_MUSIC_DIRECTORY);
    for (int clk = 1; clk <= 120; clk++)
    {
        if (clk == LOSE_CHOMP_CLK1 || clk == LOSE_CHOMP_CLK2)
            play_sound_effect(CHOMP_MUSIC_DIRECTORY);
        draw_lose_frame(double(clk) / 120);
        win.update_screen();
        HANDLE_SDL_EVENT(QUIT(););
    }
    for (int clk = 0; clk < LOSE_BANNER_HOLD_CLK; clk++)
    {
        draw_lose_frame(1.0);
        win.update_screen();
        HANDLE_SDL_EVENT(QUIT(););
    }
    win.draw_png_center(LOSING_MESSAGE_DIRECTORY);
    draw_stone_button(LOSE_CONTINUE, "Continue", pvz1_hot(LOSE_CONTINUE), false, 20);
    has_display_lost = true;
}

/*
Check if player has lost: any zombie stepped past the left board edge.
repvz Zombie::CheckForBoardEdge (Zombie.cpp:4537-4563): the lose line is
BOARD_EDGE = -100 (GameConstants.h:14) for plain walkers, but heavy rigs
and strays sink deeper before the game counts them in:
gargantuar/polevault -150, catapult/football/zomboni -175,
dancer family/snorkel -130. Clone anchor = original mX + 220 (BOARD_OFFSET).
*/
static int lose_edge_x(const Zombie &z)
{
    switch (z.type)
    {
    case GARGANTUAR_TYPE:
    case POLEVAULT_TYPE:
        return 220 - 150; // 70
    case CATAPULT_TYPE:
    case FOOTBALL_TYPE:
    case ZOMBONI_TYPE:
        return 220 - 175; // 45
    case DANCER_TYPE:
    case BACKUP_TYPE:
    case SNORKEL_TYPE:
        return 220 - 130; // 90
    default:
        return 220 - 100; // 120
    }
}

/*repvz mHasHead port. ZombiesWon only fires for a zombie that still has its
head (Zombie.cpp:4559: `mX <= aEdgeX && mHasHead`); UpdateDamageStates
(Zombie.cpp:3897) pops the head once mBodyHealth < mBodyMaxHealth/3, and a
headless walker crossing the edge just keels over at edge+70
(Zombie.cpp:4570: TakeDamage(1800, DAMAGEFLAG_DYING|DAMAGEFLAG_NOFLASH)) --
it never ends the game.
CannotLoseBodyParts types (zomboni/bungee/catapult/gargantuar/boss,
Zombie.cpp:3460-3471) plus flyers and sled riders always keep the head.
The clone pools body + helm/shield hp into one float; the original's
mBodyMaxHealth is the BODY-only pool, so carriers of gear (cone 27 / bucket
65 raw armor etc.) resolve to the 270-pt (13.5 clone) body underneath, the
ladder carrier to its 500-pt (25) body.*/
static bool zombie_has_head(Zombie &z)
{
    switch (z.type)
    {
    case ZOMBONI_TYPE:
    case BUNGEE_TYPE:
    case CATAPULT_TYPE:
    case GARGANTUAR_TYPE:
    case ZOMBOSS_TYPE:
        return true;
    default:
        break;
    }
    if (z.is_flying() || z.is_riding_vehicle())
        return true; // IsFlying / IsBobsledTeamWithSled never lose parts
    float body_max;
    switch (z.type)
    {
    case CONE_TYPE:
    case BUCKET_TYPE:
    case DOOR_TYPE:
    case NEWSPAPER_TYPE:
    case FOOTBALL_TYPE:
        body_max = *ZOMBIE_HEALTH_LIMIT[NORMAL_TYPE].rbegin(); // 13.5 = 270 body
        break;
    case LADDER_TYPE:
        body_max = 25; // 500 body under the 25-hp ladder shield
        break;
    default:
        body_max = *ZOMBIE_HEALTH_LIMIT[z.type].rbegin(); // pool is body-only
        break;
    }
    return z.get_health() >= body_max * z.hp_scale() / 3;
}

void debug_compose_lose(bool continue_hot)
{
    draw_lose_frame(1.0);
    draw_stone_button(LOSE_CONTINUE, "Continue", continue_hot, false, 20);
}

bool has_player_lost()
{
    auto &zombies = game_characters.zombies;
    for (int i = 0; i < (int)zombies.size(); i++)
    {
        Zombie *zombie = zombies[i];
        if (zombie_has_head(*zombie))
        {
            if (zombie->x_location <= lose_edge_x(*zombie))
                return true;
            continue;
        }
        /*Headless walker at edge+70: drops dead instead of winning (repvz
        Zombie.cpp:4570 TakeDamage(1800, 9U); 1800 raw = 90 clone points).*/
        if (zombie->x_location <= lose_edge_x(*zombie) + 70 &&
            zombie->take_hit(90, game_characters.zombie_parts))
        {
            zombie->add_zombie_die(game_characters.zombie_parts);
            delete zombie;
            zombies.erase(zombies.begin() + i);
            i--;
        }
    }
    return false;
}

/*
Handle when player lose:
    Display losing message.
    Check if player click 'continue'.
    Update some variables.
*/
void display_lose()
{
    // Display losing message.
    play_music(LOSE_MUSIC_DIRECTORY, 0);
    if (!has_display_lost)
        display_losing_message();
    win.update_screen();
    while (has_display_lost)
    {
        // Redraw each frame so the Continue hover highlight tracks the mouse.
        draw_lose_frame(1.0);
        draw_stone_button(LOSE_CONTINUE, "Continue", pvz1_hot(LOSE_CONTINUE), false, 20);
        win.update_screen();
        HANDLE_SDL_EVENT(
            QUIT();
            // Check if player click 'continue'.
            LCLICK({
                if (LOSE_CONTINUE.is_mouse_in(mouse_x, mouse_y))
                {
                    // Update some variables.
                    level.waves_finished = false;
                    set_status(game_state, IS_LEVEL_CHOSEN, false);
                    set_status(game_state, IS_PAUSED, false);
                    play_music(OPENING_MUSIC_DIRECTORY);
                    win.fade_out();
                }
                has_display_lost = false;
            });

        );
    }
}
