/*
Retail-style in-game dialog kit, ported from repvz:
  + LawnApp.cpp:699 DoNewOptions -> NewOptionsDialog.cpp (pause panel:
    OPTIONS_MENUBACK art with the "Options" title baked in, big BACK TO GAME
    stone button at (30,381,360x100), the two 209x46 stone rows RESTART /
    MAIN MENU at (107,284)/(107,327), Music/Sound FX slider slots at
    (199,116)/(199,143), labels right-aligned, gold-green inset fonts).
  + LawnDialog.cpp: the 9-slice DIALOG_* frame (Draw/CalcSize/Resize), the
    DIALOG_HEADER band, and the YES/NO stone-button placement, used for the
    restart/leave confirms.
  + Lawn_Widget_GameButton.cpp::DrawStoneButton: 3-slice BUTTON_LEFT/MIDDLE/
    RIGHT stone button; label from the real DwarvenTodcraft bitmap atlas
    (draw/atlas_font.*), swapping GreenInset -> BrightGreenInset on hover
    exactly like the retail FONT_DWARVENTODCRAFT18{,BRIGHT}GREENINSET swap;
    BUTTON_DOWN_* art while pressed.
With the Sexy ImageFont atlases now parsed and blitted directly (glyphs
pre-baked: green inset fill + rind on the stone buttons, gold + keyline on
the Yellow titles), only plain dialog body text and the slider labels keep
the TTF_DwarvenTodcraft + outline approximation (gold 0xE0BB62 dialog text,
green panel labels).
Panel/dialog placement: retail draws on the 800x600 board; the clone window
is 1050 wide with every screen centered, so dialogs are centered on the
window (same anchoring as the replaced clone art).
*/
#pragma once
#include "draw/rsdl.hpp"
#include "elements/button/button.hpp"

/*Pause options panel (OPTIONS_MENUBACK is 423x498; retail centers it in the
800x600 board -> the clone centers the same native size in its 1050x600
window). Panel-local button rects per NewOptionsDialog::Resize.*/
extern const int OPT_PANEL_X;
extern const int OPT_PANEL_Y;
extern const Button OPT_BACK_TO_GAME_BUTTON; // 360x100, panel-local (30,381)
extern const Button OPT_RESTART_ROW;         // 209x46, panel-local (107,284)
extern const Button OPT_MAIN_MENU_ROW;       // 209x46, panel-local (107,327)

/*Slider rows stay where the shared MUSIC_VOLUME/SFX_VOLUME click rects put
them (button.cpp): the knob is 22x29, so the OPTIONS slot bar is drawn
vertically centered on each knob row, left-aligned like retail's (199,116).*/

/*Retail text colors: dialog title/body re-colorize DwarvenTodcraft's white
glyph layer with (0xE0,0xBB,0x62) (LawnDialog SetColor 0/1); the "Music"/
"Sound FX" slider labels use NewOptionsDialog::Draw's (107,109,145).*/
extern const RGB PVZ1_DIALOG_GOLD;
extern const RGB PVZ1_SLIDER_LABEL_COLOR;
/*Stone-button labels are no longer tinted TTF: they blit the real
GreenInset / BrightGreenInset (hover) bitmap atlases straight (colors live
in the glyphs), via draw/atlas_font.*.*/

/*@return true when the mouse is inside btn (Button::blink's hit test).*/
bool pvz1_hot(const Button &btn);

/*Draw a DIALOG_CENTER*-tile-tiled 9-slice frame plus the DIALOG_HEADER plank
centered over its top edge, exactly like LawnDialog::Draw. (x,y) is the
dialog origin; the frame body starts at y + DIALOG_HEADER_OFFSET(45).*/
void draw_pvz1_dialog_frame(int x, int y, int w, int h);

/*Draw green-inset dialog text (gold, thin black outline like the retail
ImageFont layers), centered on center_x with the text top at y.*/
void draw_pvz1_gold_text(const string &text, int x, int y, int font_size);
void draw_pvz1_gold_text_centered(const string &text, int center_x, int y, int font_size);

/*Draw a 3-slice stone button (BUTTON_LEFT/MIDDLE/RIGHT; BUTTON_DOWN_* while
pressed per DrawStoneButton) + a readable Dwarvesc TTF label (the old
button palette; hot = the old hover ink), centered with the retail offsets.
font_size is the TTF point size (20 matches the pre-atlas approximation).*/
void draw_stone_button(const Button &btn, const string &label, bool hot, bool down = false,
                       int font_size = 20);

/*Draw the big "Back To Game" stone button (OPTIONS_BACKTOGAMEBUTTON0, text
size 36 offset (-2,-5) per NewOptionsDialog) with a readable Dwarvesc TTF
label (hot = the old hover ink).*/
void draw_options_back_button(const Button &btn, const string &label, bool hot);

/*Draw the classic single-art dialog button (Icons/button.png, the LawnDialog
GameButtons' look: quit/reset/get-name confirm rows) with a readable Dwarvesc
TTF label: green ink + dark shadow, 25 pt, (-3,-3) bias -- the pre-atlas
look; hot brightens the ink. Ink-centered.*/
void draw_classic_dialog_button(const Button &btn, const string &label, bool hot);

/*Draw the full pause options panel: OPTIONS_MENUBACK art + Music/Sound FX
labels + slot bars (the knobs on top come from display_slider()). Fitting
the two rows the clone ships (RESTART/MAIN MENU; no VIEW ALMANAC view),
laid out on retail's row coordinates.*/
void draw_pause_base();

/*Confirm-dialog geometry, ported from LawnDialog::CalcSize/Resize (restart
and leave share it). x/y center the dialog on the window; the frame body
lies 45 px under y (the HEADER plank sticks up over it).*/
struct Pvz1ConfirmLayout
{
    int x = 0, y = 0, w = 0, h = 0;
    int header_baseline_ascent = 0; // title baseline = y + 45 + 35 + ascent
    int body_top = 0;
    int line_height = 0;
    vector<string> body_lines;
    Button yes, no;
};
Pvz1ConfirmLayout lay_pvz1_confirm(const string &header, const string &body);

/*Draw frame + gold title + gold wrapped body + the two stone buttons
(labels GREENINSET, hover-bright on the rolled-over one). hover: 0 none,
1 yes, 2 no.*/
void draw_pvz1_confirm(const Pvz1ConfirmLayout &box, const string &header,
                       const string &yes_label, const string &no_label, int hover);
