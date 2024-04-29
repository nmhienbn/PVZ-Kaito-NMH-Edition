#include "unlock_plant.hpp"
#include "elements/Level/Level.hpp"
#include "elements/button/button.hpp"
#include "elements/elements.hpp"

extern int game_state;
extern bool quit;
extern Level level;
extern Elements game_characters;
extern Window win;

// Plants *unlocked_plant;
// PlantAssets unlocked_plant_assets;

// void display_unlocked_plant()
// {
//     unlocked_plant->display(unlocked_plant_assets);
// }

/*
If player unlock a new plant, display this
*/
void display_unlock_plant(const int &_plant_type)
{
    win.clear_renderer();
    win.draw_bg(AWARD_SCREEN_DIRECTORY);
    string title;
    string plant_name;
    vector<string> description;
    if (_plant_type == SUNFLOWER_TROPHY)
    {
        title = "YOU GOT THE GOLD SUNFLOWER TROPHY";
        plant_name = "SUNFLOWER TROPHY";
        description.push_back("Thank you for playing my game!");
        description.push_back("I hope you enjoyed it!");
        win.draw_png_center(SUNFLOWER_TROPHY_DIRECTORY, 0.6, -1, 150);
    }
    else
    {
        // if (unlocked_plant == nullptr)
        // {
        //     unlocked_plant = init_plant(_plant_type, 0, 0);
        //     unlocked_plant_assets = PlantAssets(1, 100, 100, 100, 100, 0, 0);
        // }
        title = "YOU GOT A NEW PLANT!";
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

        default:
            break;
        }
        win.set_texture_alpha(plant_dir, 255);
        win.draw_png_center(plant_dir, 1.2, -1, 170);
        win.set_texture_alpha(plant_dir, all_img[plant_dir].alpha);
    }
    win.show_text_center(title, -1, 34, RGB(196, 149, 67), DWARVESC_TTF,
                         32, TTF_STYLE_NORMAL, 1, RGB(80, 28, 0));
    win.show_text_center(plant_name, -1, 350, RGB(196, 149, 67), DWARVESC_TTF,
                         28, TTF_STYLE_NORMAL, 1, RGB(135, 78, 10));

    int lower = 415;
    int delta = 25;
    for (auto &des : description)
    {
        win.show_text_center(des, -1, lower, RGB(43, 67, 77), BRIANNE_TTF, 20);
        lower += delta;
    }

    CONTINUE.show_text("Continue", 0, 0, 20, RGB(0, 196, 0), RGB(43, 44, 58));
}

/*
Display and handle player click on unlocking plant screen.
*/
void unlock_plant(const int &_plant_type)
{
    display_unlock_plant(_plant_type);
    play_music(NEW_PLANT_MUSIC_DIRECTORY, 0);
    CONTINUE.display_with_text(BUTTON_DIRECTORY, "Continue", 20, RGB(0, 196, 0), RGB(43, 44, 58));
    win.update_screen();
    HANDLE_SDL_EVENT(
        QUIT();
        LCLICK({
            if (CONTINUE.is_mouse_in(mouse_x, mouse_y))
            {
                play_music(OPENING_MUSIC_DIRECTORY);
                win.fade_out();
                set_status(game_state, IS_UNLOCKING_PLANT, false);
            }
        });

    );
}