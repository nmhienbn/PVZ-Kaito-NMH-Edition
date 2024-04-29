#pragma once

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
    Announcer();
    Announcer(const int &img_directory, const int &sound_directory);
    void display();
    bool is_finished();
};