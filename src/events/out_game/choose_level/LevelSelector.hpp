#pragma once
#include "draw/rsdl.hpp"
#include "level/level_data.hpp"
#include <iostream>
#include <string>
using namespace std;

enum LevelStatus
{
    MOUSE_IN,
    MOUSE_OUT,
    LOCKED
};

class LevelSelector
{
private:
    int bg_dir;
    int display_zombie;
    int display_plant;
    string level_name;

public:
    static int BACKGROUND_CHOOSE_LEVEL;
    static int TYPE_LEVEL;
    LevelSelector(int level_id);
    void display(int x, int y, RGB color);
    void display_locked(int x, int y);
    void display_blink(int x, int y);
    void display_complete(int x, int y);
    bool is_mouse_in(int mouse_x, int mouse_y, int x, int y);
};

struct Point
{
    int x, y;
};

const Point LEVEL_BUTTON[] = {
    {75, 40}, {225, 40}, {375, 40}, {525, 40}, {675, 40}, {825, 40}};
