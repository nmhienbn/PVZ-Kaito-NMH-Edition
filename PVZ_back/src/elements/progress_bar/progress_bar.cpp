#include "progress_bar.hpp"

extern window win;
extern Level level;

/*
Display game progress:
    A bar: Appeared zombies will be green, remain is black.
    Text below bar: "Level progress"
    Level flags: a flag each huge wave.
    Zombie head: curent position on progress bar.
*/
void display_progress_bar()
{
    int dx = WINDOW_WIDTH - PROGRESS_BAR_WIDTH - 20;
    int dy = WINDOW_HEIGHT - PROGRESS_BAR_HEIGHT - 10;
    int progress_width = 1;
    if (level.zombie_count)
        progress_width = max(1, level.used_zombie_count * PROGRESS_BAR_WIDTH / level.zombie_count);
    int remain_width = PROGRESS_BAR_WIDTH - progress_width;
    // Green
    win.draw_png(FLAG_METER, 0, PROGRESS_BAR_HEIGHT, progress_width, PROGRESS_BAR_HEIGHT,
                 dx, dy, progress_width, PROGRESS_BAR_HEIGHT);
    // Black
    win.draw_png(FLAG_METER, progress_width, 0, remain_width, PROGRESS_BAR_HEIGHT,
                 dx + progress_width, dy, remain_width, PROGRESS_BAR_HEIGHT);
    // Level Progress text
    win.draw_png(FLAG_METER_PROGRESS, 0, 0, PROGRESS_TXT_WIDTH, PROGRESS_TXT_HEIGHT,
                 dx + (PROGRESS_BAR_WIDTH - PROGRESS_TXT_WIDTH) / 2, dy + 17,
                 PROGRESS_TXT_WIDTH, PROGRESS_TXT_HEIGHT);
    // Level flag
    int z_cnt = 0;
    for (int i = 0; i < level.wave_count; i++)
    {
        int tmp_width = max(1, z_cnt * PROGRESS_BAR_WIDTH / level.zombie_count);
        int tmp_cnt = 0;
        for (int typ = 0; typ < COUNT_ZOMBIE_TYPE; typ++)
            tmp_cnt += level.wave_zombie_count[typ][i];
        if (tmp_cnt >= 5)
        {
            tmp_cnt++; // huge wave
            if (z_cnt < level.used_zombie_count)
            {
                win.draw_png(FLAG_METER_PART, PROGRESS_PART_WIDTH, 0, PROGRESS_PART_WIDTH, PROGRESS_PART_HEIGHT,
                             dx + tmp_width, dy - 4,
                             PROGRESS_PART_WIDTH, PROGRESS_PART_HEIGHT);
                win.draw_png(FLAG_METER_PART, PROGRESS_PART_WIDTH * 2, 0, PROGRESS_PART_WIDTH, PROGRESS_PART_HEIGHT,
                             dx + tmp_width, dy - 10,
                             PROGRESS_PART_WIDTH, PROGRESS_PART_HEIGHT);
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
                 dx + progress_width - PROGRESS_PART_WIDTH + 6, dy,
                 PROGRESS_PART_WIDTH, PROGRESS_PART_HEIGHT);
}