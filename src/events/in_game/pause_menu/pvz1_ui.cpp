#include "pvz1_ui.hpp"
#include "draw/atlas_font.hpp"
#include "events/in_game/volume/volume.hpp"

extern Window win;

/*LawnDialog art constants (repvz LawnDialog.cpp + Graphics coords, art is
dialog_*.png from the 2Pak): frame body rows start under a 45-px header
offset; middle tiles repeat horizontally; center tiles vertically.*/
static const int DLG_HDR_OFFSET = 45;   // DIALOG_HEADER_OFFSET
static const int DLG_TOP_H = 97;        // IMAGE_DIALOG_TOPLEFT->mHeight
static const int DLG_TOPW = 93;         // IMAGE_DIALOG_TOPMIDDLE->mWidth
static const int DLG_LEFTW = 107;       // IMAGE_DIALOG_TOPLEFT->mWidth
static const int DLG_RIGHTW = 120;      // IMAGE_DIALOG_TOPRIGHT->mWidth
static const int DLG_CENTERH = 54;      // IMAGE_DIALOG_CENTERLEFT->mHeight
static const int DLG_BOTL_h = 112;      // IMAGE_DIALOG_BOTTOMLEFT->mHeight
static const int DLG_HPLANK_W = 187;    // IMAGE_DIALOG_HEADER->mWidth
static const int DLG_HPLANK_H = 64;

/*Stone-button 3-slice (button_left/middle/right.png, 36+46+35 wide x 46).*/
static const int BTN_LEFTW = 36;
static const int BTN_MIDW = 46;
static const int BTN_RIGHTW = 35;
static const int BTN_H = 46;

const int OPT_PANEL_X = (WINDOW_WIDTH - 423) / 2; // (800-423)/2 analog: centered
const int OPT_PANEL_Y = (WINDOW_HEIGHT - 498) / 2;
/*Retail local rects (NewOptionsDialog::Resize) + panel origin.*/
const Button OPT_BACK_TO_GAME_BUTTON(OPT_PANEL_X + 30, OPT_PANEL_X + 30 + 360,
                                     OPT_PANEL_Y + 381, OPT_PANEL_Y + 481);
const Button OPT_RESTART_ROW(OPT_PANEL_X + 107, OPT_PANEL_X + 107 + 209,
                             OPT_PANEL_Y + 284, OPT_PANEL_Y + 330);
const Button OPT_MAIN_MENU_ROW(OPT_PANEL_X + 107, OPT_PANEL_X + 107 + 209,
                               OPT_PANEL_Y + 327, OPT_PANEL_Y + 373);

const RGB PVZ1_DIALOG_GOLD(0xE0, 0xBB, 0x62);
const RGB PVZ1_SLIDER_LABEL_COLOR(107, 109, 145);

bool pvz1_hot(const Button &btn)
{
    int mx, my;
    SDL_GetMouseState(&mx, &my);
    return btn.is_mouse_in(mx, my);
}

void draw_pvz1_gold_text(const string &text, int x, int y, int font_size)
{
    win.show_text_outlined(text, x, y, PVZ1_DIALOG_GOLD, DWARVENTODCRAFT_TTF,
                           font_size, 1, BLACK);
}

void draw_pvz1_gold_text_centered(const string &text, int center_x, int y, int font_size)
{
    int tw, th;
    TTF_SizeText(win.get_font(DWARVENTODCRAFT_TTF, font_size), text.c_str(), &tw, &th);
    draw_pvz1_gold_text(text, center_x - tw / 2, y, font_size);
}

/*DrawStoneButton's label paint. User-facing call: button labels render in
READABLE text -- the pre-atlas Dwarvesc TTF with the old stone-slab palette
(black ink over a pale-green fringe; hot swaps to the old pink/magenta
hover ink) -- while titles keep the retail DwarvenTodcraft look. Same
geometry as the atlas fill it replaces (centered, retail's (+1,-1) nudge);
isDown shifts the label (1,1) via dx/dy like the atlas path did.*/
static void draw_stone_label(const string &text, const Button &btn, bool bright,
                             int font_size, int dx, int dy)
{
    const RGB ink = bright ? RGB(255, 222, 247) : BLACK;
    const RGB edge = bright ? RGB(244, 67, 191) : RGB(162, 203, 134);
    int tw, th;
    TTF_SizeText(win.get_font(DWARVESC_TTF, font_size), text.c_str(), &tw, &th);
    win.show_text_shadowed(text, (btn.x1 + btn.x2 - tw) / 2 + 1 + dx,
                           (btn.y1 + btn.y2 - th) / 2 - 1 + dy, ink,
                           DWARVESC_TTF, font_size, 0, 1, edge);
}

void draw_stone_button(const Button &btn, const string &label, bool hot, bool down, int font_size)
{
    int dl = PVZ1_BUTTON_LEFT_DIRECTORY;
    int dm = PVZ1_BUTTON_MIDDLE_DIRECTORY;
    int dr = PVZ1_BUTTON_RIGHT_DIRECTORY;
    int dx = 0, dy = 0;
    if (down)
    {
        dl = PVZ1_BUTTON_DOWN_LEFT_DIRECTORY;
        dm = PVZ1_BUTTON_DOWN_MIDDLE_DIRECTORY;
        dr = PVZ1_BUTTON_DOWN_RIGHT_DIRECTORY;
        dx = dy = 1;
    }
    int x = btn.x1 + dx;
    int y = btn.y1 + dy;
    // Left cap, middle tiles, right cap at native 36/46/35 widths, height 46.
    win.draw_png(dl, x, y, BTN_LEFTW, BTN_H);
    x += BTN_LEFTW;
    int repeat = (btn.x2 - btn.x1 - BTN_LEFTW - BTN_RIGHTW) / BTN_MIDW;
    while (repeat-- > 0)
    {
        win.draw_png(dm, x, y, BTN_MIDW, BTN_H);
        x += BTN_MIDW;
    }
    win.draw_png(dr, x, y, BTN_RIGHTW, BTN_H);
    draw_stone_label(label, btn, hot, font_size, dx, dy);
}

void draw_options_back_button(const Button &btn, const string &label, bool hot)
{
    win.draw_png(PVZ1_BACK_TO_GAME_DIRECTORY, btn.x1, btn.y1, btn.x2 - btn.x1, btn.y2 - btn.y1);
    // NewOptionsDialog: 36-pt label centered + mTextOffset(-2,-5); readable
    // Dwarvesc TTF with the old button palette (hot = old hover ink).
    const RGB ink = hot ? RGB(255, 222, 247) : BLACK;
    const RGB edge = hot ? RGB(244, 67, 191) : RGB(162, 203, 134);
    int tw, th;
    TTF_SizeText(win.get_font(DWARVESC_TTF, 36), label.c_str(), &tw, &th);
    win.show_text_shadowed(label, (btn.x1 + btn.x2 - tw) / 2 - 2,
                           (btn.y1 + btn.y2 - th) / 2 - 5, ink,
                           DWARVESC_TTF, 36, 0, 1, edge);
}

void draw_classic_dialog_button(const Button &btn, const string &label, bool hot)
{
    btn.display(BUTTON_DIRECTORY);
    // The pre-atlas look: green ink + dark shadow (old display_with_text
    // on BUTTON_DIRECTORY, 25 pt, (-3,-3) bias); hot brightens the ink.
    const RGB ink = hot ? RGB(120, 255, 120) : RGB(0, 196, 0);
    int tw, th;
    TTF_SizeText(win.get_font(DWARVESC_TTF, 25), label.c_str(), &tw, &th);
    win.show_text_shadowed(label, (btn.x1 + btn.x2 - tw) / 2 - 3,
                           (btn.y1 + btn.y2 - th) / 2 - 3, ink,
                           DWARVESC_TTF, 25, 0, 1, RGB(43, 44, 58));
}

void draw_pause_base()
{
    win.draw_png(PVZ1_OPTIONS_MENUBACK_DIRECTORY, OPT_PANEL_X, OPT_PANEL_Y, 423, 498);
    // Music / Sound FX labels right-aligned like NewOptionsDialog::Draw
    // (x 186, DS_ALIGN_RIGHT); rows ride the shared knob rects' centers.
    const int slot_center[2] = {MUSIC_VOLUME.y1 + 14, SFX_VOLUME.y1 + 14};
    const char *label[2] = {"Music", "Sound FX"};
    for (int i = 0; i < 2; i++)
    {
        int tw, th;
        TTF_SizeText(win.get_font(DWARVENTODCRAFT_TTF, 18), label[i], &tw, &th);
        int tx = OPT_PANEL_X + 186 - tw;
        int ty = slot_center[i] - th / 2 + 1;
        win.show_text_outlined(label[i], tx, ty, PVZ1_SLIDER_LABEL_COLOR,
                               DWARVENTODCRAFT_TTF, 18, 1, BLACK);
        // OPTIONS_SLIDERSLOT bar (native 135x10), centered on the knob row.
        win.draw_png(ALMANAC_SLIDERSLOT_DIRECTORY, OPT_PANEL_X + 199,
                     slot_center[i] - 5, 135, 10);
    }
    draw_options_back_button(OPT_BACK_TO_GAME_BUTTON, "Back To Game",
                             pvz1_hot(OPT_BACK_TO_GAME_BUTTON));
    draw_stone_button(OPT_RESTART_ROW, "RESTART", pvz1_hot(OPT_RESTART_ROW));
    draw_stone_button(OPT_MAIN_MENU_ROW, "MAIN MENU", pvz1_hot(OPT_MAIN_MENU_ROW));
    display_slider(); // the knobs ride their own click rects (volume.cpp)
}

void draw_pvz1_dialog_frame(int x, int y, int w, int h)
{
    int ry = y + DLG_HDR_OFFSET;
    int repeat_x = (w - DLG_LEFTW - DLG_RIGHTW) / DLG_TOPW;
    int repeat_y = (h - DLG_TOP_H - DLG_BOTL_h - DLG_HDR_OFFSET) / DLG_CENTERH;

    win.draw_png(PVZ1_DIALOG_TOPLEFT_DIRECTORY, x, ry, DLG_LEFTW, DLG_TOP_H);
    int px = x + DLG_LEFTW;
    for (int i = 0; i < repeat_x; i++)
    {
        win.draw_png(PVZ1_DIALOG_TOPMIDDLE_DIRECTORY, px, ry, DLG_TOPW, DLG_TOP_H);
        px += DLG_TOPW;
    }
    win.draw_png(PVZ1_DIALOG_TOPRIGHT_DIRECTORY, px, ry, DLG_RIGHTW, DLG_TOP_H);
    int py = ry + DLG_TOP_H;
    for (int j = 0; j < repeat_y; j++)
    {
        win.draw_png(PVZ1_DIALOG_CENTERLEFT_DIRECTORY, x, py, DLG_LEFTW, DLG_CENTERH);
        px = x + DLG_LEFTW;
        for (int i = 0; i < repeat_x; i++)
        {
            win.draw_png(PVZ1_DIALOG_CENTERMIDDLE_DIRECTORY, px, py, DLG_TOPW, DLG_CENTERH);
            px += DLG_TOPW;
        }
        win.draw_png(PVZ1_DIALOG_CENTERRIGHT_DIRECTORY, px, py, 117, DLG_CENTERH);
        py += DLG_CENTERH;
    }
    win.draw_png(PVZ1_DIALOG_BOTTOMLEFT_DIRECTORY, x, py, DLG_LEFTW, DLG_BOTL_h);
    px = x + DLG_LEFTW;
    for (int i = 0; i < repeat_x; i++)
    {
        win.draw_png(PVZ1_DIALOG_BOTTOMMIDDLE_DIRECTORY, px, py, DLG_TOPW, 114);
        px += DLG_TOPW;
    }
    win.draw_png(PVZ1_DIALOG_BOTTOMRIGHT_DIRECTORY, px, py, DLG_RIGHTW, 114);
    win.draw_png(PVZ1_DIALOG_HEADER_DIRECTORY, x + (w - DLG_HPLANK_W) / 2 - 5, y,
                 DLG_HPLANK_W, DLG_HPLANK_H);
}

/*Word-wrap body paragraphs (split on '\n') to a pixel width with the retail
body font size, mirroring WriteWordWrapped at line-spacing scale.*/
static vector<string> wrap_body(const string &body, int max_w, int font_size, int &line_h)
{
    vector<string> out;
    TTF_Font *font = win.get_font(DWARVENTODCRAFT_TTF, font_size);
    line_h = TTF_FontHeight(font);
    size_t pos = 0;
    auto wrap_paragraph = [&](const string &para)
    {
        if (para.empty())
        {
            out.push_back("");
            return;
        }
        string line;
        // split on spaces
        size_t i = 0;
        while (i < para.size())
        {
            size_t j = para.find(' ', i);
            string word = para.substr(i, j == string::npos ? string::npos : j - i);
            string trial = line.empty() ? word : line + " " + word;
            int tw, th;
            TTF_SizeText(font, trial.c_str(), &tw, &th);
            if (tw > max_w && !line.empty())
            {
                out.push_back(line);
                line = word;
            }
            else
            {
                line = trial;
            }
            if (j == string::npos)
                break;
            i = j + 1;
        }
        out.push_back(line);
    };
    while (true)
    {
        size_t nl = body.find('\n', pos);
        if (nl == string::npos)
        {
            wrap_paragraph(body.substr(pos));
            break;
        }
        wrap_paragraph(body.substr(pos, nl - pos));
        pos = nl + 1;
    }
    return out;
}

/*LawnDialog::CalcSize + Resize, ported with the clone's TTF metrics standing
in for the ImageFonts (sizes 24/15 -> 25/16 TTF px).*/
Pvz1ConfirmLayout lay_pvz1_confirm(const string &header, const string &body)
{
    Pvz1ConfirmLayout box;
    const int HDR_FONT = 25;
    const int BODY_FONT = 16;
    // CalcSize: width from content insets (36,*,46) + header, tiled to 93.
    int hw, hh;
    TTF_SizeText(win.get_font(DWARVENTODCRAFT_TTF, HDR_FONT), header.c_str(), &hw, &hh);
    int aWidth = 36 + 46 + hw;
    const int image_w = DLG_LEFTW + DLG_RIGHTW + DLG_TOPW; // 320
    if (aWidth <= image_w)
        aWidth = image_w;
    else
    {
        int rem = (aWidth - image_w) % DLG_TOPW;
        if (rem)
            aWidth += DLG_TOPW - rem;
    }
    aWidth += DLG_TOPW; // CalcSize always adds one when there are body lines
    int basic_w = aWidth - 36 - 46 - 4;

    int line_h = 0;
    box.body_lines = wrap_body(body, basic_w, BODY_FONT, line_h);
    int wrapped_h = (int)box.body_lines.size() * line_h;
    int wrapped_h_visual = wrapped_h + 30; // CalcSize's + 30

    // height: content insets + header offset + header text + space + body + buttons.
    int ascent = TTF_FontAscent(win.get_font(DWARVENTODCRAFT_TTF, HDR_FONT));
    int font_h = TTF_FontHeight(win.get_font(DWARVENTODCRAFT_TTF, HDR_FONT));
    int aHeight = 35 + 36 + DLG_HDR_OFFSET + (-0 + font_h + 10) + wrapped_h_visual + 24;
    const int image_h = DLG_TOP_H + DLG_BOTL_h + DLG_HDR_OFFSET; // 254
    if (aHeight < image_h)
        aHeight = image_h;
    else
    {
        int rem = (aHeight - image_h) % DLG_CENTERH;
        if (rem)
            aHeight += DLG_CENTERH - rem;
    }
    box.w = aWidth;
    box.h = aHeight;
    box.x = (WINDOW_WIDTH - aWidth) / 2;
    box.y = (WINDOW_HEIGHT - aHeight) / 2;
    box.header_baseline_ascent = ascent;
    box.line_height = line_h;
    // Draw's vertical centering of the wrapped body (mVerticalCenterText).
    int aFontY = 35 + DLG_HDR_OFFSET;                    // 80
    aFontY = (aFontY - 0 + ascent) - ascent + font_h + 10; // = 80 + font_h + 10
    int lines_area_h = aHeight - 36 - 24 - aFontY - 55;
    box.body_top = box.y + aFontY + (lines_area_h - wrapped_h) / 2;

    // Resize: the YES/NO stone buttons from the content area insets.
    int area_x = 36 + 0 - 5;
    int area_y = aHeight - 36 - 0 - BTN_H + 2;
    int area_w = aWidth - 46 - 0 - 0 - 36 + 8;
    int bmin = BTN_LEFTW + BTN_RIGHTW;
    int extra = (area_w - 10) / 2 - BTN_MIDW - bmin + 1;
    if (extra < 0)
        extra = 0;
    else if (extra % BTN_MIDW)
        extra += BTN_MIDW - extra % BTN_MIDW;
    int bw = bmin + extra;
    box.yes = Button(box.x + area_x, box.x + area_x + bw, box.y + area_y, box.y + area_y + BTN_H);
    box.no = Button(box.x + area_x + area_w - bw, box.x + area_x + area_w,
                    box.y + area_y, box.y + area_y + BTN_H);
    return box;
}

void draw_pvz1_confirm(const Pvz1ConfirmLayout &box, const string &header,
                       const string &yes_label, const string &no_label, int hover)
{
    draw_pvz1_dialog_frame(box.x, box.y, box.w, box.h);
    // Title: same line as Draw's aOffsetY baseline, now the real
    // DwarvenTodcraft18Yellow atlas (gold glyphs + baked keyline) scaled to
    // the old TTF-25 ink height (~16 px) -- the layout measurement above
    // (lay_pvz1_confirm) is unchanged, so the dialog geometry is untouched.
    int tw, th;
    TTF_SizeText(win.get_font(DWARVENTODCRAFT_TTF, 25), header.c_str(), &tw, &th);
    AtlasFont::yellow().draw_centered(win, header, box.x + box.w / 2,
                                      box.y + 35 + DLG_HDR_OFFSET + box.header_baseline_ascent - th / 2,
                                      16.0f / 20.0f);
    // Wrapped body (gold), each line left-aligned in the content strip.
    for (size_t i = 0; i < box.body_lines.size(); i++)
    {
        if (box.body_lines[i].empty())
            continue;
        draw_pvz1_gold_text(box.body_lines[i], box.x + 38,
                            box.body_top + (int)i * box.line_height, 16);
    }
    draw_stone_button(box.yes, yes_label, hover == 1);
    draw_stone_button(box.no, no_label, hover == 2);
}
