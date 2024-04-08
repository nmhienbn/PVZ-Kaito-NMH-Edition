#include "elements/elements_actions.hpp"
#include "elements_actions.hpp"

extern int clk;
extern Level level;
extern Elements game_characters;
extern Icons icons;
extern Player player;
extern Map cells;

/*Handles all the changes to the game:
    + Create new wave of zombies.
    + Check all bullets' moving status.
    + Check all mowers' moving status.
    + Update all plants' & zombies' status.
    + Generate suns from the sky (not same time). No sun at night.
    + Remove the suns that appear for a long time.
    + Update remaining time of each plant seed.
*/
void handle_changes()
{
    // Create new wave of zombies.
    update_new_wave_zombies();

    // Check all bullets' moving status
    handle_pea_zombie_encounter(game_characters.peas, game_characters.zombies, game_characters.zombie_parts);

    // Check all mowers' status
    handle_mower_zombie_encounter(game_characters.zombies, game_characters.zombie_parts);

    // Update all plants' status
    update_plants_status(game_characters.plants);
    // Update all zombies' status
    update_zombies_status(game_characters.zombies, game_characters.plants);

    // Generate suns from the sky. No sun at night.
    if (level.has_sun_from_sky == true && clk % SUN_GEN_SKY_CLK_COUNT == 0)
        gen_random_sun_from_sky(game_characters.suns);

    // Remove the suns that appear for a long time
    remove_suns(game_characters.suns);

    // Update remaining time of each plant seed
    update_plant_seeds_remaining_time();
}

/*Create a new zombie on random row and at ZOMBIE_INIT_X.
If the level has not finished:
    + Generate zombies for current wave and current second.
    + Then move to the next second and maybe next wave.
    + Reduce the empty time between waves of zombies when all current zombies died (while zombies.empty() loop).
*/
void create_new_zombies()
{
    Zombie *temp;
    if (level.waves_finished == false)
    {
        // This loop is to reduce the empty time between waves of zombies when all current zombies died.
        do
        {
            if (game_characters.zombies.size() >= 10)
            {
                break;
            }
            level.last_clk_zombie_appear = clk;
            // Generate zombies for current wave and current second.
            // For all type of zombies.
            for (int typ = NORMAL_TYPE; typ < COUNT_ZOMBIE_TYPE; typ++)
            {
                // Number of zombies for this second of waves.
                int zombie_cnt = level.zombie_distr_for_wave[typ][level.cur_wave][level.cur_sec];
                level.used_zombie_count += zombie_cnt;
                // First second of first wave: sound effect "The zombies are coming".
                if (level.zombie_has_coming == false)
                {
                    if (zombie_cnt > 0)
                    {
                        play_sound_effect(ZOMBIE_COMING_MUSIC_DIRECTORY);
                        level.zombie_has_coming = true;
                    }
                }
                // Random row of new zombie.
                for (int i = 0; i < zombie_cnt; i++)
                {
                    temp = init(typ, level.level_num);
                    game_characters.zombies.push_back(temp);
                }
            }
            // Move to the next second and maybe next wave.
            // First second of each wave: sound effect "Huge wave".

            // Next sec
            if (level.cur_sec + 1 < level.wave_duration[level.cur_wave])
            {
                level.cur_sec++;
            }
            // Next wave
            else if (level.cur_wave + 1 < level.wave_count)
            {
                level.cur_sec = 0;
                level.cur_wave++;
                if (level.cur_wave == level.wave_count - 1)
                {
                    play_sound_effect(HUGE_WAVE_MUSIC_DIRECTORY);
                    level.announce_directory = FINAL_WAVE_DIRECTORY;
                    // Make flag zombies
                    game_characters.zombies.push_back(init(FLAG_TYPE, level.level_num));
                    level.used_zombie_count++;
                }
                else if (level.is_huge_wave())
                {
                    play_sound_effect(HUGE_WAVE_MUSIC_DIRECTORY);
                    level.announce_directory = HUGE_WAVE_DIRECTORY;
                    // Make flag zombies
                    game_characters.zombies.push_back(init(FLAG_TYPE, level.level_num));
                    level.used_zombie_count++;
                }
            }
            // waves finished
            else
            {
                level.waves_finished = true;
                break;
            }
        } while (game_characters.zombies.empty());
    }
}

/*Create a new zombie on random row and at ZOMBIE_INIT_X.
If the level has not finished:
    + Generate zombies for current wave and current second.
    + Then move to the next second and maybe next wave.
    + Reduce the empty time between waves of zombies when all current zombies died (while zombies.empty() loop).
*/
void create_new_zombies1()
{
    if (level.waves_finished == false)
    {
        // This loop is to reduce the empty time between waves of zombies when all current zombies died.
        do
        {
            if (game_characters.zombies.size() >= 10)
            {
                break;
            }
            level.last_clk_zombie_appear = clk;

            // Get zombies for current wave.
            auto wave = level.waves[level.cur_wave];

            for (const auto &zombie : wave.zombies)
                game_characters.zombies.push_back(zombie);

            // Number of zombies for this wave.
            level.used_zombie_count += wave.zombies.size();

            // First wave of zombie: sound effect "The zombies are coming".
            if (level.zombie_has_coming == false)
            {
                play_sound_effect(ZOMBIE_COMING_MUSIC_DIRECTORY);
                level.zombie_has_coming = true;
            }
            // Final wave sound
            else if (level.cur_wave == level.final_wave)
            {
                play_sound_effect(HUGE_WAVE_MUSIC_DIRECTORY);
                level.announce_directory = FINAL_WAVE_DIRECTORY;
            }
            // Huge wave sound
            else if (wave.has_flag)
            {
                play_sound_effect(HUGE_WAVE_MUSIC_DIRECTORY);
                level.announce_directory = HUGE_WAVE_DIRECTORY;
            }

            // Next wave
            if (level.cur_wave + 1 < level.wave_count)
            {
                level.cur_wave++;
                level.next_wave_clk = level.waves[level.cur_wave].delay_time;
            }
            // waves finished
            else
            {
                level.waves_finished = true;
                break;
            }
        } while (game_characters.zombies.empty());
    }
}

/*Create new wave of zombies.
If level has finised and that's time to create new wave:
        + first wave
        + there's no zombie
        + long time no seen new zombies
*/
void update_new_wave_zombies()
{
    if (level.level_num <= 3)
    {
        if (--level.next_wave_clk <= 0 ||                            // next wave
            (level.cur_wave > 0 && game_characters.zombies.empty())) // if there's no zombie
            create_new_zombies1();
        if (clk - level.last_clk_zombie_appear == ANNOUNCER_CLK_COUNT) // remove announcer huge wave, final wave
            level.announce_directory = NULL_DIRECTORY;
        return;
    }
    if (level.waves_finished == false &&
        (clk == FIRST_WAVE_CLK_COUNT ||                                     // first wave
         (clk > FIRST_WAVE_CLK_COUNT && game_characters.zombies.empty()) || // if there's no zombie
         clk - level.last_clk_zombie_appear >= ZOMBIE_CREATE_CLK_COUNT))    // long time no seen new zombies
        create_new_zombies();
    else if (clk - level.last_clk_zombie_appear == ANNOUNCER_CLK_COUNT) // remove announcer huge wave, final wave
        level.announce_directory = NULL_DIRECTORY;
}

/*Handle all movement is happening: zombies, suns, peas.*/
void handle_movements()
{
    move_zombies(game_characters.zombies);
    move_suns(game_characters.suns);
    move_peas(game_characters.peas);
}

/*Update icons remainning time. Sun count blink*/
void update_plant_seeds_remaining_time()
{
    // Update icons remainning time
    for (int i = 0; i < PLANT_COUNT; i++)
    {
        if (icons.plant_remaining_time[i])
            icons.plant_remaining_time[i]--;
    }
    // Sun count blink
    if (player.sun_count_change_color_times)
    {
        if (player.sun_count_change_color_time)
        {
            player.sun_count_change_color_time--;
        }
        else
        {
            player.sun_count_change_color_time = SUN_CHANGE_COLOR_CLK_COUNT;
            player.sun_count_change_color_times--;
        }
    }
}