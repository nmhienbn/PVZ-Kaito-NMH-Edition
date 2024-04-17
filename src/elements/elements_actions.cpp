#include "elements_actions.hpp"

extern int clk;
extern Level level;
extern Elements game_characters;

extern Player player;
extern Map cells;
Wave current_wave;

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
    handle_bullet_encounter_zombie(game_characters.bullets, game_characters.zombies, game_characters.zombie_parts);

    // Check all mowers' status
    handle_mower_zombie_encounter(game_characters.mowers, game_characters.zombies, game_characters.zombie_parts);

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

void add_zombies_from_level_data()
{
    // First wave of zombie: sound effect "The zombies are coming".
    if (level.zombie_has_coming == false)
    {
        play_sound_effect(ZOMBIE_COMING_MUSIC_DIRECTORY);
        level.zombie_has_coming = true;
    }

    // Add zombies from level data to game characters.
    for (const auto &zombie : current_wave.zombies)
        game_characters.zombies.push_back(zombie);

    // Number of zombies for this wave.
    level.used_zombie_count += current_wave.zombies.size();
}

void go_to_next_wave()
{
    // Next wave
    if (level.cur_wave + 1 < level.wave_count)
    {
        level.cur_wave++;
        level.next_wave_clk = level.waves[level.cur_wave].delay_time;
        level.zombie_delay_state = NO_DELAY;
        level.announcer.state = ANNOUNCED;
    }
    // waves finished
    else
        level.waves_finished = true;
}

/*Create a new zombie on random row and at ZOMBIE_INIT_X.
If the level has not finished:
    + Generate zombies for current wave and current second.
    + Then move to the next second and maybe next wave.
    + Reduce the empty time between waves of zombies when all current zombies died (while zombies.empty() loop).
*/
void create_new_zombies()
{
    // Get zombies for current wave.
    current_wave = level.waves[level.cur_wave];
    if (level.cur_wave > 0 && game_characters.zombies.empty())
        level.next_wave_clk = 0;
    if (level.next_wave_clk > 0)
    {
        level.next_wave_clk--;
        return;
    }
    if (level.zombie_delay_state == FINAL_WAVE)
    {
        if (game_characters.zombies.size() < 10)
        {
            if (level.announcer.state == ANNOUNCED)
            {
                level.announcer = Announcer(HUGE_WAVE_DIRECTORY, HUGE_WAVE_MUSIC_DIRECTORY);
            }
        }
        else
            return;
        if (level.announcer.state == ANNOUNCING && level.announcer.is_finished())
        {
            level.zombie_delay_state = HUGE_WAVE;
            level.announcer = Announcer(FINAL_WAVE_DIRECTORY, FINAL_WAVE_MUSIC_DIRECTORY);
            play_sound_effect(SIREN_MUSIC_DIRECTORY);
        }
        return;
    }

    if (level.zombie_delay_state == HUGE_WAVE)
    {
        if (game_characters.zombies.size() < 10)
        {
            if (level.announcer.state == ANNOUNCED)
            {
                level.announcer = Announcer(HUGE_WAVE_DIRECTORY, HUGE_WAVE_MUSIC_DIRECTORY);
            }
        }
        else
            return;
        if (level.announcer.state == ANNOUNCING && level.announcer.is_finished())
        {
            level.zombie_delay_state = AFTER_ANNOUNCE;
            level.announcer.state = ANNOUNCED;
        }
    }

    // Final wave
    if (level.zombie_delay_state == NO_DELAY)
    {
        if (level.cur_wave == level.final_wave)
        {
            level.zombie_delay_state = FINAL_WAVE;
            return;
        }
        // Huge wave
        else if (current_wave.has_flag)
        {
            level.zombie_delay_state = HUGE_WAVE;
            return;
        }
    }

    add_zombies_from_level_data();
    go_to_next_wave();
}

/*Create new wave of zombies.
If level has finised and that's time to create new wave:
        + first wave
        + there's no zombie
        + long time no seen new zombies
*/
void update_new_wave_zombies()
{
    if (level.waves_finished)
        return;

    create_new_zombies();
    return;
}

/*Handle all movement is happening: zombies, suns, peas.*/
void handle_movements()
{
    move_zombies(game_characters.zombies);
    move_suns(game_characters.suns);
    move_bullets(game_characters.bullets);
}

/*Update icons remainning time. Sun count blink*/
void update_plant_seeds_remaining_time()
{
    // Update icons remainning time
    for (auto &seed_packet : player.seed_packets)
    {
        if (seed_packet.remaining_time)
        {
            seed_packet.remaining_time--;
        }
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