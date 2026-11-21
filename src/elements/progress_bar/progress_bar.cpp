#include "progress_bar.hpp"
#include "../boss/boss_fight.hpp"
#include "draw/rsdl.hpp"
#include "elements/Level/Level.hpp"
#include "elements/elements.hpp"
#include "level/level_data.hpp"
#include <algorithm>
#include <cmath>

// Flag-meter geometry, ported 1:1 from the original DrawProgressMeter
// (repvz Board.cpp:6772-6865): the original board is 800x600 with the meter
// at (600, 575) -- a 200-px right margin. The clone window is 1050 wide with
// the extra 250 px of street on the right, so the meter keeps the original's
// right margin.
#define METER_WIDTH 158
#define METER_HEIGHT 27
const int METER_X = WINDOW_WIDTH - 200; // original 600
const int METER_Y = WINDOW_HEIGHT - 25; // original 575

// Original logical widths (Board.h:25 PROGRESS_METER_COUNTER = 150).
#define PROGRESS_UNITS 150     // mProgressMeterWidth domain
#define METER_FILL_MAX 143     // Board.cpp:6783: fill clip width at full progress
#define METER_HEAD_MAX 135     // Board.cpp:6863: head travel range
#define FLAG_TRACK_RIGHT 148   // Board.cpp:6825: aFlagsPosEnd - 600
#define FLAG_TRACK_LEFT 6      // Board.cpp:6840: 606 - 600
#define FLAG_BONUS 12          // Board.cpp:5669-5677: extra units credited per passed flag wave
#define FLAG_RAISE_TICKS 60    // GameConstants.h:37 FLAG_RAISE_TIME = 100cs == 60 ticks at 60 tps
#define FLAG_RAISE_HEIGHT 14   // Board.cpp:6833

#define PROGRESS_PART_WIDTH 25

extern Window win;
extern Level level;
extern Elements game_characters;
extern int game_state;

// Meter smoothing state (original Board::mProgressMeterWidth chase in
// UpdateProgressMeter).
static int now_progress = 0;   // smoothed fill width, 0..PROGRESS_UNITS
static int meter_clk = 0;      // frame counter, stands in for mMainCounter
static int flag_raise_clk = 0; // ticks since the newest flag wave spawned
static int flags_raised = 0;   // flag waves whose raise beat has started

/*Flag-wave layout of the current level: waves-per-flag and flag count are
read off the wave plan (every huge wave has Wave::has_flag set by
wave_gen.cpp), which is the clone's equivalent of the original
GetNumWavesPerFlag (Board.cpp:547-550).*/
static void flag_layout(int wave_count, int &waves_per_flag, int &num_flags)
{
    waves_per_flag = 0;
    num_flags = 0;
    for (int i = 0; i < wave_count; i++)
        if (level.waves[i].has_flag)
        {
            num_flags++;
            if (!waves_per_flag)
                waves_per_flag = i + 1;
        }
}

/*Target fill width in original units (UpdateProgressMeter, repvz
Board.cpp:5658-5702):
  + ticks by PENDING wave count: wave boundary cur sits at
    (cur-1)*aTotalWidth/(W-1) plus 12 units back per flag wave already
    reached;
  + inside the wave, the fraction is the LARGER of the countdown fraction and
    the damage fraction toward the early-trigger health target;
  + spawning the final wave (cur == W) jumps straight to full;
  + 0 before the first wave spawns (the meter stays hidden, HasProgressMeter
    Board.cpp:6740).*/
static int progress_target()
{
    /*Final boss level (5-10): the meter tracks Zombot DAMAGE, not waves
    (repvz UpdateProgressMeter, Board.cpp:5646-5656):
        width = 150 * (max - hp) / max, set directly (no chase);
    a missing/dead boss means the fight is won -> full (150). The level has
    no waves at all, so the wave math below must not run (it would hide the
    meter behind its waveless guard).*/
    if (level.is_special(LevelSpecial::Boss))
    {
        int hp, max_hp;
        if (bossfight::boss_health(hp, max_hp) && max_hp > 0)
        {
            hp = std::min(std::max(hp, 0), max_hp);
            return PROGRESS_UNITS * (max_hp - hp) / max_hp;
        }
        // Before the Zombot steps in (waves_finished goes up only when the
        // controller spawns it) the meter sits empty; after the kill it is
        // full.
        return level.waves_finished ? PROGRESS_UNITS : 0;
    }
    const int W = level.wave_count;
    if (W <= 1 || level.cur_wave == 0)
        return 0;
    if (level.cur_wave >= W)
        return PROGRESS_UNITS; // Board.cpp:5673 with cur == W clamps to 150

    int waves_per_flag, num_flags;
    flag_layout(W, waves_per_flag, num_flags);
    const bool has_flags = num_flags > 0;
    int total_width = PROGRESS_UNITS;
    if (has_flags)
        total_width -= FLAG_BONUS * (W / waves_per_flag); // Board.cpp:5669

    // The clone keeps cur_wave on the final index after spawning the final
    // wave (go_to_next_wave only raises waves_finished); the original's
    // mCurrentWave runs on to mNumWaves, which pins the meter at full
    // (Board.cpp:5673 clamps to 150).
    int cur = level.waves_finished ? W : level.cur_wave;
    if (cur >= W)
        return PROGRESS_UNITS;

    // Fraction elapsed inside the pending wave: countdown ...
    float fraction = 0.0f;
    int delay_total = level.waves[cur].delay_time;
    if (delay_total > 0 && level.next_wave_clk <= delay_total)
        fraction = (float)(delay_total - level.next_wave_clk) / delay_total;
    // ... or damage dealt to the wave on the lawn (Board.cpp:5457-5469). The
    // damage target is spawn health minus the early-trigger threshold
    // rolled at the wave's spawn (level.wave_trigger_health = a random
    // 50-65% of spawn health; 0 for the wait into a flag wave, so the meter
    // tracks the full wave there).
    if (level.wave_spawn_health > 0)
    {
        int rest_health = 0;
        for (const auto &zombie : game_characters.zombies)
            if (zombie->wave_id == cur - 1)
                rest_health += zombie->get_health();
        int damage_target = level.wave_spawn_health - (int)level.wave_trigger_health; // spawn - trigger
        if (damage_target < 1)
            damage_target = 1;
        float health_fraction =
            (float)(level.wave_spawn_health - rest_health) / damage_target;
        fraction = std::max(fraction, health_fraction);
    }
    fraction = std::min(std::max(fraction, 0.0f), 1.0f);

    int extra = has_flags ? FLAG_BONUS * (cur / waves_per_flag) : 0; // Board.cpp:5677
    int start_len = (cur - 1) * total_width / (W - 1) + extra;
    int end_len = cur * total_width / (W - 1) + extra;
    int target = start_len + (int)lround((end_len - start_len) * fraction);
    return std::min(std::max(target, 1), PROGRESS_UNITS);
}

/*Flag pole screen-x for the 0-based wave index of a flag wave
(Board.cpp:6840): the track runs from METER_X + FLAG_TRACK_RIGHT (wave 0)
down to METER_X + FLAG_TRACK_LEFT (wave W), so flag k of N sits at wave
fraction (wave_index + 1) / W = k / N -- evenly spaced by wave count.*/
int progress_bar_flag_x(int flag_wave_index)
{
    const int W = level.wave_count;
    if (W <= 0)
        return -1;
    return METER_X + FLAG_TRACK_RIGHT -
           (int)lround((FLAG_TRACK_RIGHT - FLAG_TRACK_LEFT) *
                       (flag_wave_index + 1) / (double)W);
}

// Headless-check hooks (see demo/progress_bar_check.cpp).
int progress_bar_fill_units()
{
    return now_progress;
}
int progress_bar_num_flags()
{
    // The boss meter has no flags (ProgressMeterHasFlags, Board.cpp:6759).
    if (level.is_special(LevelSpecial::Boss))
        return 0;
    int per_flag, num_flags;
    flag_layout(level.wave_count, per_flag, num_flags);
    return num_flags;
}
int progress_bar_raised_flags()
{
    return flags_raised;
}
// Ticks into the raise beat of the newest flag wave (0..FLAG_RAISE_TICKS);
// draw height = clk * 14 / 60 (Board.cpp:6837's TodAnimateCurve over
// FLAG_RAISE_TIME=100cs at 60 tps).
int progress_bar_flag_raise_clk()
{
    return flag_raise_clk;
}

/*
Display game progress (repvz Board::DrawProgressMeter, Board.cpp:6772-6865):
    + The bar mirrors to the right edge: the green fill grows RIGHT -> LEFT
      from (METER_X + 151, METER_Y) as progress rises, and the zombie head
      rides its left edge toward the house.
    + Flags are spaced evenly by WAVE fraction (k * waves-per-flag / wave
      count) -- not by spawned-zombie count -- and pop up FLAG_RAISE_HEIGHT px
      over ~1 s when their wave lands (mFlagRaiseCounter). Passed flags stay
      raised; there is no gray/yellow color swap in the original, only the
      height change.
    + Meter and head are hidden until the first wave spawns; waveless levels
      (4-5 vasebreaker) skip it entirely (HasProgressMeter / ScaryPotter).
    + Boss level 5-10 is the exception: no waves, no flags, meter always
      visible, and the fill is the Zombot's DAMAGE fraction (Board.cpp:
      5646-5656 sets the width straight from boss health -- 150 * (max-hp) /
      max -- with no smoothing chase).
    + Level number sits left of the meter like the original DrawLevel string
      (Board.cpp:6937-6947, right-aligned at x=593, baseline 595).
*/
void display_progress_bar(const bool advance)
{
    const bool boss_mode = level.is_special(LevelSpecial::Boss); // 5-10: Zombot-health meter
    const int W = level.wave_count;
    if (W <= 1 && !boss_mode)
        return; // waveless (vasebreaker) or stubbed special levels

    int waves_per_flag, num_flags;
    flag_layout(W, waves_per_flag, num_flags);
    // Board.cpp:6759: the boss meter never shows flags (5-10 still carries a
    // generated wave plan on disk -- never spawned, never flagged).
    const bool has_flags = !boss_mode && num_flags > 0;
    int total_width = PROGRESS_UNITS;
    if (has_flags)
        total_width -= FLAG_BONUS * (W / waves_per_flag);

    // Auto-FPS advance gate: the chase/raise state below must advance once
    // per LOGIC tick, not per display call -- the render rate is uncapped,
    // so a rendered frame can cover several logic ticks (turbo). advance =
    // false marks the 4-10 storm redraw pass (redraw_ui_on_top in
    // storm.cpp calls this a SECOND time per frame because the storm sheet
    // covers the first draw; the bar must not chase at 2x speed there).
    // Pause freezes the meter too (logic_ticked holds a stale value while
    // paused, so IS_PAUSED is checked explicitly).
    const int adv_ticks = (advance && check_status(game_state, IS_PAUSED) == false)
                              ? logic_ticked
                              : 0;
    // Flag raise bookkeeping: a flag's height animates 0 -> FLAG_RAISE_HEIGHT
    // over FLAG_RAISE_TICKS once its wave has spawned (Board.cpp:5161-5164
    // sets the counter at spawn, 6829-6838 map it to the draw height).
    int eff_wave = level.waves_finished ? W : level.cur_wave;
    int flags_spawned = 0;
    for (int i = 0; i < eff_wave && i < W; i++)
        if (level.waves[i].has_flag)
            flags_spawned++;
    if (level.cur_wave == 0 && flags_raised)
        flags_raised = 0; // level restarted
    if (flags_spawned > flags_raised)
    {
        flags_raised = flags_spawned;
        flag_raise_clk = 0;
    }
    else
    {
        // The raise beat steps once per logic tick (auto-FPS).
        for (int _lt = adv_ticks; _lt > 0 && flag_raise_clk < FLAG_RAISE_TICKS; _lt--)
            flag_raise_clk++;
    }

    // Smooth chase toward the target width (Board.cpp:5704-5709): +1 every
    // 20cs (12 ticks here) normally, +1 every 5cs (3 ticks) while more than
    // one wave length behind. On restart (target shrank) reset like
    // Board::BoardInit zeroing mProgressMeterWidth (Board.cpp:1272). The boss
    // meter skips the chase: the original sets the width directly from the
    // Zombot's health (Board.cpp:5651).
    int target = progress_target();
    if (boss_mode)
        now_progress = target;
    else
    {
        for (int _lt = adv_ticks; _lt > 0; _lt--)
        {
            if (target < now_progress)
            {
                now_progress = 0;
                meter_clk = 0;
            }
            meter_clk++;
            if (now_progress < target)
            {
                int wave_length = total_width / (W - 1);
                int delta = target - now_progress;
                if ((delta > wave_length && meter_clk % 3 == 0) ||
                    (delta > 0 && meter_clk % 12 == 0))
                    now_progress++;
            }
        }
    }

    if (now_progress == 0 && !boss_mode)
        return; // HasProgressMeter: nothing drawn before the first wave;
                // the boss meter is visible from frame one (Board.cpp:6734:
                // IsFinalBossLevel always has a meter)

    // Empty bar (FlagMeter cel row 0).
    win.draw_png(FLAG_METER, 0, 0, METER_WIDTH, METER_HEIGHT,
                 METER_X, METER_Y, METER_WIDTH, METER_HEIGHT);
    // Green fill (cel row 1), clipped to the right-hand side so it grows
    // leftward from x = METER_X + 151 (Board.cpp:6783-6786).
    int clip = (int)lround(now_progress * METER_FILL_MAX / (double)PROGRESS_UNITS);
    if (clip > 0)
        win.draw_png(FLAG_METER, METER_WIDTH - clip - 7, METER_HEIGHT, clip, METER_HEIGHT,
                     METER_X + METER_WIDTH - clip - 7, METER_Y, clip, METER_HEIGHT);
    // "Level progress" plaque (Board.cpp:6852: drawn at (638, 589)).
    win.draw_png(FLAG_METER_PROGRESS, 0, 0, 86, 11,
                 METER_X + 38, METER_Y + 14, 86, 11);
    // Flags, evenly spaced by wave fraction (Board.cpp:6821-6845): the wave
    // carrying flag k has k * waves_per_flag zombies-waves behind it, and the
    // track runs from METER_X + FLAG_TRACK_RIGHT (level start) down to
    // METER_X + FLAG_TRACK_LEFT (level end).
    if (has_flags)
    {
        int flag_idx = 0;
        for (int i = 0; i < W; i++)
        {
            if (!level.waves[i].has_flag)
                continue;
            flag_idx++;
            int x = progress_bar_flag_x(i);
            int height = 0;
            if (flag_idx == flags_raised)
                height = flag_raise_clk * FLAG_RAISE_HEIGHT / FLAG_RAISE_TICKS;
            else if (flag_idx < flags_raised)
                height = FLAG_RAISE_HEIGHT;
            // Pole (parts cel 1), then the flag itself (cel 2) riding the
            // raise height (Board.cpp:6842-6844).
            win.draw_png(FLAG_METER_PART, PROGRESS_PART_WIDTH, 0, PROGRESS_PART_WIDTH, 25,
                         x, METER_Y - 4, PROGRESS_PART_WIDTH, 25);
            win.draw_png(FLAG_METER_PART, PROGRESS_PART_WIDTH * 2, 0, PROGRESS_PART_WIDTH, 25,
                         x, METER_Y - 3 - height, PROGRESS_PART_WIDTH, 25);
        }
    }
    // Zombie head on the fill edge (Board.cpp:6863-6864).
    int head_progress = (int)lround(now_progress * METER_HEAD_MAX / (double)PROGRESS_UNITS);
    win.draw_png(FLAG_METER_PART, 0, 0, PROGRESS_PART_WIDTH, 25,
                 METER_X + 138 - head_progress, METER_Y - 3,
                 PROGRESS_PART_WIDTH, 25);
    // Level number, left of the meter (original DrawLevel: right-aligned at
    // x=593 = meter x - 7, baseline 595 = meter y + 20, 16-px font).
    string LEVEL_NUMBER = get_lv_json(level.level_num)["#comment"];
    int w, h;
    TTF_SizeText(win.get_font(PVZUI_TTF, 16), LEVEL_NUMBER.c_str(), &w, &h);
    win.set_style(PVZUI_TTF, 16, TTF_STYLE_BOLD);
    win.show_text_outlined(LEVEL_NUMBER, METER_X - 7 - w, METER_Y + 20 - h, YELLOW_A, PVZUI_TTF, 16);
    win.set_style(PVZUI_TTF, 16, TTF_STYLE_NORMAL);
}
