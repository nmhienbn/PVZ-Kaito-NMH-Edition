#pragma once
#include "draw/render_elements.hpp"
#include <string>
using namespace std;
#define SAVED_DATA_DIRECTORY "./resources/userdata/savedata.txt"

void update_player_name(const string &new_name);
// Scene-frame painter for the call site's home screen (display_choose_level's
// signature). `backdrop` paints one frame of the screen behind the dim +
// dialog (choose-level scene when nullptr); the selector/mode screen passes
// its own frame pass so rename opens over the tombstones.
typedef void (*SceneBackdrop)(const bool &);
void display_get_name_player(const int &get_name_dir, SceneBackdrop backdrop = nullptr);
// One get-name dialog frame (display_get_name_player's render body; frame_clk
// drives the edit-box caret blink).
void draw_get_name_dialog(const int &get_name_dir, const string &inputText, int frame_clk,
                          SceneBackdrop backdrop = nullptr);
