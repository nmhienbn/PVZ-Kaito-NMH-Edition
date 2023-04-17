#include "progress_bar.hpp"
#define PROGRESS_BAR_WIDTH 158
#define PROGRESS_BAR_WIDTH_1 7
#define PROGRESS_BAR_WIDTH_2 144
#define PROGRESS_BAR_HEIGHT 27
#define PROGRESS_TXT_WIDTH 86
#define PROGRESS_TXT_HEIGHT 11
#define PROGRESS_PART_WIDTH 25
#define PROGRESS_PART_HEIGHT 25

extern window win;
extern Level level;
int now_progress = 0;
const int dx = WINDOW_WIDTH - PROGRESS_BAR_WIDTH - 25;
const int dy = WINDOW_HEIGHT - PROGRESS_BAR_HEIGHT - 5;

/*
Display game progress:
    A bar: Appeared zombies will be green, remain is black.
    Text below bar: "Level progress"
    Level flags: a flag each huge wave.
    Zombie head: curent position on progress bar.
    Level num text
*/
void display_progress_bar()
{
    int progress_width = PROGRESS_BAR_WIDTH_1 + level.used_zombie_count * PROGRESS_BAR_WIDTH_2 / level.zombie_count;
    // Not let this progress update too fast
    if (now_progress > progress_width)
    {
        now_progress = PROGRESS_BAR_WIDTH_1;
    }
    if (now_progress < progress_width)
    {
        now_progress++;
    }
    int remain_width = PROGRESS_BAR_WIDTH - now_progress;
    // Green
    win.draw_png(FLAG_METER, 0, PROGRESS_BAR_HEIGHT, now_progress, PROGRESS_BAR_HEIGHT,
                 dx, dy, now_progress, PROGRESS_BAR_HEIGHT);
    // Black
    win.draw_png(FLAG_METER, now_progress, 0, remain_width, PROGRESS_BAR_HEIGHT,
                 dx + now_progress, dy, remain_width, PROGRESS_BAR_HEIGHT);
    // Level Progress text
    win.draw_png(FLAG_METER_PROGRESS, 0, 0, PROGRESS_TXT_WIDTH, PROGRESS_TXT_HEIGHT,
                 dx + (PROGRESS_BAR_WIDTH - PROGRESS_TXT_WIDTH) / 2, dy + 17,
                 PROGRESS_TXT_WIDTH, PROGRESS_TXT_HEIGHT);
    // Level flag
    int z_cnt = 0;
    for (int i = 0; i < level.wave_count; i++)
    {
        int tmp_width = PROGRESS_BAR_WIDTH_1 + z_cnt * PROGRESS_BAR_WIDTH_2 / level.zombie_count;
        // cout << z_cnt << ' ' << level.zombie_count << endl;
        int tmp_cnt = 0;
        for (int typ = NORMAL_TYPE; typ < COUNT_ZOMBIE_TYPE; typ++)
            tmp_cnt += level.wave_zombie_count[typ][i];
        if (tmp_cnt >= 5 || i == level.wave_count - 1)
        {
            tmp_cnt++; // huge wave
            if (z_cnt < level.used_zombie_count)
            {
                win.draw_png(FLAG_METER_PART, PROGRESS_PART_WIDTH, 0, PROGRESS_PART_WIDTH, PROGRESS_PART_HEIGHT,
                             dx + tmp_width, dy - 4, PROGRESS_PART_WIDTH, PROGRESS_PART_HEIGHT);
                win.draw_png(FLAG_METER_PART, PROGRESS_PART_WIDTH * 2, 0, PROGRESS_PART_WIDTH, PROGRESS_PART_HEIGHT,
                             dx + tmp_width, dy - 10, PROGRESS_PART_WIDTH, PROGRESS_PART_HEIGHT);
            }
            else
            {
                win.draw_png(FLAG_METER_PART, PROGRESS_PART_WIDTH * 2, 0, PROGRESS_PART_WIDTH, PROGRESS_PART_HEIGHT,
                             dx + tmp_width, dy - 2,
                             PROGRESS_PART_WIDTH, PROGRESS_PART_HEIGHT);
            }
        }
        z_cnt += tmp_cnt;
    }
    // Zombie head
    win.draw_png(FLAG_METER_PART, 0, 0, PROGRESS_PART_WIDTH, PROGRESS_PART_HEIGHT,
                 dx + now_progress - PROGRESS_PART_WIDTH / 2, dy,
                 PROGRESS_PART_WIDTH, PROGRESS_PART_HEIGHT);

    string LEVEL_NUMBER = "Level " + to_string(level.level_num);
    int w = 1, h = 1;
    TTF_SizeText(win.get_font(CONTM_TTF, 24), LEVEL_NUMBER.c_str(), &w, &h);

    win.show_text_shadowed(LEVEL_NUMBER, dx - w - 15, dy, YELLOW_A, CONTM_TTF, 24);
}