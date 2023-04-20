#include "elements/elements_actions.hpp"

extern int clk;
extern Level level;
extern Elements game_characters;
extern Icons icons;
extern Player player;
extern Map cells;

/*
Handles all the changes to the game:
    + Update all zombies' moving status.
    + Check all bullets' moving status.
    + Create new wave of zombies. (if level has finised and that's time to create new wave)
        + first wave
        + there's no zombie
        + long time no seen new zombies
    + Make zombies' bite to plants and update the next time they can bite them.
    + Fire all the bullets.
    + Generate suns from the sky (not same time). No sun at night.
    + Generate suns from sunflowers (not same time).
    + Remove the suns that appear for a long time.
    + Update remaining time of each plant seed.
*/
void handle_changes()
{ // Update all zombies' moving status
    update_moving_status_for_zombies(game_characters.zombies);

    // Check all bullets' moving status
    handle_pea_zombie_encounter(game_characters.peas, game_characters.zombies, game_characters.dead_zombies);

    // Check all mowers' status
    handle_mower_zombie_encounter(game_characters.zombies, game_characters.dead_zombies);

    // Create new wave of zombies. (if level has finised and that's time to create new wave)
    if (level.waves_finished == false &&
        (clk == FIRST_WAVE_CLK_COUNT ||                                     // first wave
         (clk > FIRST_WAVE_CLK_COUNT && game_characters.zombies.empty()) || // if there's no zombie
         clk - level.last_clk_zombie_appear >= ZOMBIE_CREATE_CLK_COUNT))    // long time no seen new zombies
        create_new_zombies();
    else if (clk - level.last_clk_zombie_appear == ANNOUNCER_CLK_COUNT)
        level.announce_directory = NULL_DIRECTORY;

    // Zombie bite plant
    handle_zombie_plant_encounter(game_characters.zombies, game_characters.plants);

    // Update next time for each zombie to bite plant
    update_zombie_next_bite(game_characters.zombies);

    for (int i = 0; i < (int)game_characters.plants.size(); i++)
    {
        auto plant = game_characters.plants[i];
        switch (plant->get_type())
        {
        case PEASHOOTER_TYPE:
        {
            // Fire pea
            auto tmp = dynamic_cast<Peashooter *>(plant);
            if (tmp)
            {
                tmp->fire_pea(game_characters.zombies, game_characters.peas);
            }
            break;
        }

        case SUNFLOWER_TYPE:
        {
            // Generate suns from sunflowers.
            auto tmp = dynamic_cast<Sunflower *>(plant);
            if (tmp)
            {
                tmp->gen_sun_from_a_sunflower(game_characters.suns);
            }
            break;
        }

        case SNOWPEA_TYPE:
        {
            // Fire snowpea
            auto tmp = dynamic_cast<Snowpea *>(plant);
            if (tmp)
            {
                tmp->fire_pea(game_characters.zombies, game_characters.peas);
            }
            break;
        }

        case CHERRYBOMB_TYPE:
        {
            // Cherrybomb hit
            auto tmp = dynamic_cast<CherryBomb *>(plant);
            if (tmp)
            {
                tmp->hit_all_zombies(game_characters.zombies, game_characters.dead_zombies);
                if (tmp->is_disappeared())
                {
                    cells[plant->get_row()][plant->get_col()].is_planted = false;
                    delete plant;
                    game_characters.plants.erase(game_characters.plants.begin() + i);
                    i--;
                }
            }
            break;
        }

        default:
            break;
        }
    }

    // Generate suns from the sky. No sun at night.
    if (level.is_night == false && clk % SUN_GEN_SKY_CLK_COUNT == 0)
        gen_random_sun_from_sky(game_characters.suns);

    // Remove the suns that appear for a long time
    remove_suns(game_characters.suns);

    // Update remaining time of each plant seed
    update_remaining_time();
}

/*
- Create a new zombie on random row and at ZOMBIE_INIT_X
- If the level has not finished:
    + Generate zombies for current wave and current second.
    + Then move to the next second and maybe next wave.
    + Reduce the empty time between waves of zombies when all current zombies died (while zombies.empty() loop).
*/
void create_new_zombies()
{
    Zombie temp;
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
                    temp = Zombie(typ, level.level_num);
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
                    game_characters.zombies.push_back(Zombie(FLAG_TYPE, level.level_num));
                    level.used_zombie_count++;
                }
                else if (level.is_huge_wave())
                {
                    play_sound_effect(HUGE_WAVE_MUSIC_DIRECTORY);
                    level.announce_directory = HUGE_WAVE_DIRECTORY;
                    // Make flag zombies
                    game_characters.zombies.push_back(Zombie(FLAG_TYPE, level.level_num));
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

/*
Handle all movement is happening: zombies, suns, peas.
@param clk: the clock of game
*/
void handle_movements()
{
    move_zombies(game_characters.zombies);
    move_suns(game_characters.suns);
    move_peas(game_characters.peas);
}

/*
Update icons remainning time
*/
void update_remaining_time()
{
    for (int i = 0; i < PLANT_COUNT; i++)
    {
        if (icons.plant_remaining_time[i])
            icons.plant_remaining_time[i]--;
    }
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