#pragma once
#include "ImageType.hpp"
#include "game_param.hpp"
#include "gl/gl_sprite_batch.hpp"
#include "gl/gl_sprite_renderer.hpp"
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <cstdlib>
#include <iostream>
#include <map>
#include <string>
#define FONTS_DIRECTORY "./resources/fonts/"
#define FREESANS_TTF "FreeSans.ttf"
#define CONTM_TTF "contm.ttf"
#define BRIANNE_TTF "Brianne_s_hand.ttf"
#define HOUSEOFTERROR_TTF "HouseofTerror.ttf"
#define DWARVESC_TTF "Dwarvesc.ttf"
#define DWARVENTODCRAFT_TTF "FNT_DwarvenTodcraft.ttf" /* PvZ Replanted UI font, in resources/fonts */
#define PVZUI_TTF "PVZUI.ttf"
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
const RGB YELLOW_A(226, 189, 98);

void print_error(bool ok, const string &img);
void print_error(SDL_Surface *res, const string &img);

// One texture_cache slot: the GL texture plus the persistent color/alpha
// mods applied at draw time (SDL_SetTextureColorMod/AlphaMod equivalents).
struct TexEntry
{
	GlTexture tex;
	bool loaded = false;
	int alpha = 255;
	int r = 255;
	int g = 255;
	int b = 255;
};

int walk_of(const int &img_dir);
int blink_of(const int &img_dir);
int blink_ori(const int &img_dir);
int eat_of(const int &img_dir);

/*F2 layout-editor debug scale: a per-object render-scale override keyed by
the object's live address. Read by the render paths the editor can restyle:
Mower::display/display_reanim, Plants::display + the reanim plant hook,
Zombie::render_zombie/render_reanim, SeedPacket::display/blink and
display_suns. The editor sets entries while it is open and
rsdl_clear_debug_scales() when it closes, so normal gameplay never sees them.
Render-only: movement/collision anchor fields are untouched.*/
void rsdl_set_debug_scale(const void *key, float scale);
void rsdl_clear_debug_scale(const void *key);
void rsdl_clear_debug_scales();
float rsdl_debug_scale(const void *key); // 1.0 when no override is set

class Window
{
public:
	// global window & renderer

	Window(int width = 800, int height = 400, string title = "PVZ - Kaito NMH Edition");
	~Window();
	void update_screen();
	void clear_renderer();

	// Presentation cap for update_screen: 0 = uncapped (present as fast as the
	// GPU/vsync allows -- gameplay with the auto-FPS fixed-timestep loop),
	// > 0 = paced at that many frames/s (menus and blocking scenes keep 60).
	// Re-arms the pacer schedule only when the value actually changes; setting
	// the current value again is a no-op.
	void set_frame_cap(int tps);

	// normal draw

	void fill_rect(int x, int y, int width, int height, RGB color = WHITE);
	void draw_line(int x1, int y1, int x2, int y2, RGB color = WHITE);
	void draw_point(int x, int y, RGB color = WHITE);
	void draw_rect(int x, int y, int width, int height, RGB color = WHITE);

	// draw images
	// Note that we don't draw 0-width and 0-height images
	// To optimize, we use texture_cache[] to store images

	GlTexture *load_texture_with_blink(int file_num);
	GlTexture *load_texture(int file_num);
	void draw_bmp(int file_num, int x, int y, int width, int height);
	void draw_png(int file_num, int x, int y, int width, int height);
	void draw_png_height_scaled(int file_num, int x, int y, int width, int angle = 0);
	void draw_png_width_scaled(int file_num, int x, int y, int height, int angle = 0);
	void draw_png_center(int file_num, double ratio = 1.0, int x1 = -1, int y1 = -1);
	// Bottom-left anchor variants: the game world thinks in feet/ground contact
	// points, SDL draws from the top-left, so these take (x, y_bottom) and
	// translate internally. Prefer them for anything planted on the lawn
	// (plants, zombies, mowers) over hand-rolled "- height" math.
	void draw_png_bl(int file_num, int x, int y_bottom, int width, int height)
	{
		draw_png(file_num, x, y_bottom - height, width, height);
	}
	void draw_png_height_scaled_bl(int file_num, int x, int y_bottom, int width, int angle = 0);
	void draw_nth_frame_bl(int file_num, int sframe, int ori_w, int ori_h,
						   int game_x, int game_y_bottom, int game_w, int game_h, int angle = 0)
	{
		draw_nth_frame(file_num, sframe, ori_w, ori_h, game_x, game_y_bottom - game_h,
					   game_w, game_h, angle);
	}
	void draw_png(int file_num, int x, int y, int width, int height, int angle,
				  SDL_RendererFlip flip = SDL_FLIP_NONE);
	void draw_png(int file_num, int sx, int sy, int sw, int sh,
				  int dx, int dy, int dw, int dh,
				  SDL_RendererFlip flip = SDL_FLIP_NONE);
	void draw_nth_frame(int file_num, int sframe, int ori_w, int ori_h,
						int game_x, int game_y, int game_w, int game_h, int angle = 0);
	void draw_bg(int file_num, int x = 0, int y = 0);
	void delete_texture(const int &file_num);
	void delete_all_texture();
	void delete_all_texture_no_use();

	void set_texture_alpha(int file_num, int a);
	void set_texture_color(int file_num, int r, int g, int b);
	void fade_out();

	GlSpriteBatch &sprite_batch() { return batch; }
	ISpriteRenderer &sprite_renderer() { return sprite_rend; }

	// draw text
	// Note that we don't draw null strings
	// To optimize, we use fonts_cache[] to store fonts

	TTF_Font *get_font(string font_addr, const int &size);
	void show_text(const string &input, const int &x = 0, const int &y = 0,
				   const RGB &color = WHITE, string font_addr = FREESANS_TTF,
				   const int &size = 24, const int &aplha = 255);
	void show_text_outlined(const string &input, const int &x = 0, const int &y = 0,
							const RGB &color = WHITE, string font_addr = FREESANS_TTF,
							const int &size = 24, const int &outline_weight = 2,
							const RGB &outline_color = BLACK);
	void show_text_shadowed(const string &input, const int &x = 0, const int &y = 0,
							const RGB &color = WHITE, string font_addr = FREESANS_TTF,
							const int &size = 24, const int &x_shadow = 0, const int &y_shadow = 2,
							const RGB &shadow_color = BLACK);
	void show_text_center(const string &input, int x = -1, int y = -1,
						  const RGB &color = WHITE, string font_addr = FREESANS_TTF,
						  const int &size = 24, const int &style = TTF_STYLE_NORMAL,
						  const int &outline_weight = 0, const RGB &outline_color = BLACK);
	void show_announcer_text(const string &announcer, const int &time_a = MAX_TIME_ANNOUNCE);
	void show_announcer_text();
	bool has_announcer_text() const { return time_announce > 0; }
	void set_style(const string &font_addr, const int &size, const int &style);
	void set_outline(const string &font_addr, const int &size, const int &outline);
	void show_text_utf8(const string &input, const int &x, const int &y,
						const RGB &color, string font_addr, const int &size);

private:
	// Frame-cap schedule for update_screen (60 fps == 60 gameplay tps).
	FramePacer frame_pacer;
	// Effective presentation cap used by update_screen: 0 = uncapped
	// (present as fast as GPU/vsync allow), > 0 = paced frames/s.
	// -1 = unset, update_screen falls back to the global FPS default (60).
	int frame_cap = -1;
	SDL_Window *win;
	SDL_GLContext gl_context = nullptr;
	GlSpriteBatch batch;
	GlSpriteRenderer sprite_rend{batch};
	TexEntry texture_cache[COUNT_USED_DIRECTORY];
	std::map<string, TTF_Font *> fonts_cache;

	// Cached rendered text, keyed by (font, size, style, outline, text,
	// color, alpha); entries unused for TEXT_CACHE_EVICT_FRAMES consecutive
	// frames are freed in update_screen().
	struct TextCacheEntry
	{
		GlTexture tex;
		long last_used_frame = 0;
	};
	std::map<string, TextCacheEntry> text_cache;
	long frame_counter = 0;
	static const long TEXT_CACHE_EVICT_FRAMES = 300;
	int clear_r = 0, clear_g = 0, clear_b = 0;

	void set_color(RGB color);
	void show_text_cached(TTF_Font *font, const string &input, const int &x, const int &y,
						  const RGB &color, const int &alpha, bool utf8);
	void dump_err() { std::cerr << SDL_GetError() << '\n'; }
	void init();
	string announce;
	int time_announce = 0;
};

#define HANDLE_SDL_EVENT(A)        \
	SDL_Event e;                   \
	while (SDL_PollEvent(&e) != 0) \
	{                              \
		A                          \
	}
#define QUIT()              \
	if (e.type == SDL_QUIT) \
	{                       \
		exit(0);            \
	}
/* PvZ Reflourished-style: losing window focus no longer pauses the game.
The macro is kept (expanded to nothing) so existing call-sites stay untouched;
music and simulation keep running while the window is unfocused. */
#define LOST_FOCUS(Code)
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
