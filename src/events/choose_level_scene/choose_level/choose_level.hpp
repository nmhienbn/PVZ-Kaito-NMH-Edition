#pragma once
#include <deque>
using namespace std;

void handle_choosing_level_screen();

// Page math of the journey map (LEVELS_PER_PAGE wide, clamped to
// 1..LEVEL_COUNT); shared with the area tabs so their jump lands exactly on
// an area's first page.
deque<int> get_page_starting_at(int first);
deque<int> get_prev_page(deque<int> shown_level);
deque<int> get_next_page(deque<int> shown_level);
