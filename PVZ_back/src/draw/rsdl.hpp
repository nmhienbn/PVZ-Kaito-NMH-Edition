#pragma once
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <string>
#include <iostream>
#include <cstdlib>
#include <map>
#include "game_stats.hpp"
#include "image_directory.hpp"
#define FONTS_DIRECTORY "./fonts/"
#define MAX_TIME_ANNOUNCE 180

class RGB
{
public:
	RGB(int r, int g, int b)
	{
		if (r < 0 || r > 255 || g < 0 || g > 255 || b < 0 || b > 255)
			throw "Invalid RGB color!";
		red = r;
		green = g;
		blue = b;
	}
	int red;
	int green;
	int blue;
};

const RGB WHITE(255, 255, 255);
const RGB YELLOW(255, 255, 0);
const RGB MAGENTA(255, 0, 255);
const RGB RED(255, 0, 0);
const RGB CYAN(0, 255, 255);
const RGB GREEN(0, 255, 0);
const RGB BLUE(0, 0, 255);
const RGB BLACK(0, 0, 0);
void print_error(SDL_Texture *res, const string &img);
class window
{
public:
	window(int width = 800, int height = 400, string title = "PVZ - Kaito NMH Edition");
	~window();
	void draw_bmp(int file_num, int x, int y, int width, int height);
	void draw_png(int file_num, int x, int y, int width, int height);
	void draw_png_scale(int file_num, int x, int y, int width, int height);
	void draw_png_center(int file_num);
	void draw_png(int file_num, int x, int y, int width, int height, int angle);
	void draw_png(int file_num, int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh);
	TTF_Font *get_font(string font_addr, RGB color, int size);
	void show_text(string input, int x = 0, int y = 0, RGB color = WHITE, string font_addr = "FreeSans.ttf", int size = 24);
	void draw_bg(int file_num, int x = 0, int y = 0);
	void update_screen();
	void fill_rect(int x, int y, int width, int height, RGB color = WHITE);
	void draw_line(int x1, int y1, int x2, int y2, RGB color = WHITE);
	void draw_point(int x, int y, RGB color = WHITE);
	void draw_rect(int x, int y, int width, int height, RGB color = WHITE);
	void clear_renderer();
	void set_texture_alpha(int file_num, int a);

	void fade_out();
	void show_announcer_text(const string &announcer);
	void show_announcer_text();

	int previous_time = 0;

private:
	SDL_Window *win;
	SDL_Renderer *renderer;
	SDL_Texture *texture_cache[COUNT_USED_DIRECTORY];
	std::map<string, TTF_Font *> fonts_cache;
	std::map<string, SDL_Texture *> string_cache;
	void set_color(RGB color);
	void dump_err() { std::cerr << SDL_GetError() << '\n'; }
	void init();
	string announce;
	int time_announce;
};

#define HANDLE(A)                  \
	SDL_Event e;                   \
	while (SDL_PollEvent(&e) != 0) \
	{                              \
		A                          \
	}
#define QUIT(Code)          \
	if (e.type == SDL_QUIT) \
	{                       \
		Code;               \
	}
#define LCLICK(Code)                                                         \
	if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) \
	{                                                                        \
		int mouse_x = e.button.x;                                            \
		int mouse_y = e.button.y;                                            \
		Code;                                                                \
	}
#define RCLICK(Code)                                                          \
	if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_RIGHT) \
	{                                                                         \
		int mouse_x = e.button.x;                                             \
		int mouse_y = e.button.y;                                             \
		Code;                                                                 \
	}
#define KEY_PRESS(Char, Code)                                     \
	if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_##Char) \
	{                                                             \
		Code;                                                     \
	}

#define KEY_TO_WIN(Code)                                                                                                                                        \
	if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_w && SDL_GetModState() & KMOD_CTRL && SDL_GetModState() & KMOD_ALT && SDL_GetModState() & KMOD_SHIFT) \
	{                                                                                                                                                           \
		Code;                                                                                                                                                   \
	}
#define LRELEASE(Code)                                                     \
	if (e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_LEFT) \
	{                                                                      \
		int mouse_x = e.button.x;                                          \
		int mouse_y = e.button.y;                                          \
		Code;                                                              \
	}
#define RRELEASE(Code)                                                      \
	if (e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_RIGHT) \
	{                                                                       \
		int mouse_x = e.button.x;                                           \
		int mouse_y = e.button.y;                                           \
		Code;                                                               \
	}

#define DELAY(Millis) SDL_Delay(Millis)
