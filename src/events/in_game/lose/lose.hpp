#pragma once

void display_losing_message();
/*Dev/headless-check compose (the debug_award_frame pattern): the settled
lose frame (frozen lawn + door art + banner fully in, Continue stone
button) without the 165-frame intro loop or present(), so dump drivers can
read the frame back.*/
void debug_compose_lose(bool continue_hot);
bool has_player_lost();
void display_lose();