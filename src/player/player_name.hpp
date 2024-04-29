#pragma once
#include "draw/render_elements.hpp"
#include <string>
using namespace std;
#define SAVED_DATA_DIRECTORY "./resources/userdata/savedata.txt"

void update_player_name(const string &new_name);
void display_get_name_player(const int &get_name_dir);