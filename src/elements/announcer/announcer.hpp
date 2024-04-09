#pragma once
#include "draw/rsdl.hpp"
#include "music/music.hpp"

enum AnnouncerState
{
    ANNOUNCING,
    ANNOUNCED
};

class Announcer
{
public:
    int img_directory;
    int sound_directory;
    int remain_time;
    int state;
    Announcer(const int &img_directory, const int &sound_directory);
    void display();
    bool is_finished();
};