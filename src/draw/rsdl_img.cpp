#include "rsdl.hpp"
#include <iostream>
using namespace std;

int last_render[COUNT_USED_DIRECTORY];

/*
Print error when loading image
*/
void print_error(bool ok, const string &img)
{
    if (!ok)
    {
        printf("Cannot load image %s! Error: %s\n", img.c_str(), SDL_GetError());
        exit(-123);
    }
}

/*
Print error when loading image
*/
void print_error(SDL_Surface *res, const string &img)
{
    if (res == NULL)
    {
        printf("Cannot load image %s! Error: %s\n", img.c_str(), SDL_GetError());
        exit(-123);
    }
}

/*
Draw a cached texture through the sprite batch, applying the persistent
per-slot color/alpha mods (SDL_SetTextureColorMod/AlphaMod equivalents).
*/
static void draw_entry(GlSpriteBatch &batch, const TexEntry &entry,
                       int sx, int sy, int sw, int sh,
                       int dx, int dy, int dw, int dh,
                       int angle = 0, SDL_RendererFlip flip = SDL_FLIP_NONE)
{
    // SDL drew no-alpha-source textures with SDL_BLENDMODE_NONE, so their
    // alpha mod (e.g. titlescreen declared with alpha 0) had no effect; with
    // GL blending always on, the mod must be ignored for those textures.
    Uint8 a = entry.tex.has_alpha ? (Uint8)entry.alpha : 255;
    batch.drawQuad(entry.tex, (float)sx, (float)sy, (float)sw, (float)sh,
                   (float)dx, (float)dy, (float)dw, (float)dh,
                   (float)angle,
                   (flip & SDL_FLIP_HORIZONTAL) != 0,
                   (flip & SDL_FLIP_VERTICAL) != 0,
                   (Uint8)entry.r, (Uint8)entry.g, (Uint8)entry.b, a);
}

/*
Load the white-silhouette (blink) variant of an image: keep the surface pixel
manipulation, only the upload step is GL now.
*/
GlTexture *Window::load_texture_with_blink(int file_num)
{
    string img_dir = all_img[file_num].img_dir;
    SDL_Surface *loadedSurface = IMG_Load(img_dir.c_str());
    print_error(loadedSurface, img_dir);
    int height = loadedSurface->h;

    Uint32 *pixels = (Uint32 *)loadedSurface->pixels;
    int pitch = loadedSurface->pitch;
    int pixel_count = pitch / sizeof(Uint32) * height;

    Uint32 white_color = SDL_MapRGBA(loadedSurface->format, 0xff, 0xff, 0xff, 0xff);
    Uint32 transparent = SDL_MapRGBA(loadedSurface->format, 0x00, 0x00, 0x00, 0x00);

    for (int i = 0; i < pixel_count; ++i)

    {
        Uint32 pixel = pixels[i];
        Uint8 r, g, b, a;
        SDL_GetRGBA(pixel, loadedSurface->format, &r, &g, &b, &a);
        pixels[i] = (a > 127 ? white_color : transparent);
    }

    TexEntry &entry = texture_cache[blink_of(file_num)];
    entry.tex = batch.upload(loadedSurface);
    entry.loaded = entry.tex.id != 0;
    SDL_FreeSurface(loadedSurface);
    print_error(entry.loaded, img_dir);
    return &entry.tex;
}

/*
Get loaded texture.
If has not loaded, create new texture.
*/
GlTexture *Window::load_texture(int file_num)
{
    TexEntry &entry = texture_cache[file_num];
    last_render[file_num] = 0;
    if (!entry.loaded)
    {
        int blink = blink_ori(file_num);
        if (blink != NULL_DIRECTORY)
        {
            load_texture_with_blink(blink);
            // load_texture_with_blink already filled this slot (blink_of(blink)
            // == file_num); restore a cascading default below.
        }
        else
        {
            SDL_Surface *surface = IMG_Load(all_img[file_num].img_dir.c_str());
            print_error(surface, all_img[file_num].img_dir);
            entry.tex = batch.upload(surface);
            entry.loaded = entry.tex.id != 0;
            SDL_FreeSurface(surface);
        }
        print_error(entry.loaded, all_img[file_num].img_dir);
        // default alpha from the image metadata, like the old
        // set_default_alpha on first load
        entry.alpha = all_img[file_num].alpha;
        entry.r = entry.g = entry.b = 255;
    }
    return &entry.tex;
}

/*
Load & draw bmp:
    Load: Surface -> Texture
    Draw: batch.drawQuad
*/
void Window::draw_bmp(int file_num, int x, int y, int width, int height)
{
    if (file_num == NULL_DIRECTORY || width <= 0 || height <= 0)
        return;
    TexEntry &entry = texture_cache[file_num];
    if (!entry.loaded)
    {
        SDL_Surface *surface = SDL_LoadBMP(all_img[file_num].img_dir.c_str());
        print_error(surface, all_img[file_num].img_dir);
        entry.tex = batch.upload(surface);
        entry.loaded = entry.tex.id != 0;
        SDL_FreeSurface(surface);
        print_error(entry.loaded, all_img[file_num].img_dir);
        entry.alpha = all_img[file_num].alpha;
    }
    draw_entry(batch, entry, 0, 0, entry.tex.w, entry.tex.h, x, y, width, height);
}

/*
Load & draw with height scaled by width: height = width * mHeight / mWidth
    Load: Texture
    Draw: batch.drawQuad
    mHeight, mWidth: from the texture entry
*/
void Window::draw_png_height_scaled(int file_num, int x, int y, int width, int angle)
{
    if (file_num == NULL_DIRECTORY || width <= 0)
        return;
    load_texture(file_num);
    const TexEntry &entry = texture_cache[file_num];
    int mWidth = entry.tex.w, mHeight = entry.tex.h;
    draw_entry(batch, entry, 0, 0, mWidth, mHeight, x, y, width, width * mHeight / mWidth, angle);
}

/*
Bottom-left anchor variant of draw_png_height_scaled: (x, y_bottom) is the
contact/ground point, so the top-left y is pushed up by the scaled height.
*/
void Window::draw_png_height_scaled_bl(int file_num, int x, int y_bottom, int width, int angle)
{
    if (file_num == NULL_DIRECTORY || width <= 0)
        return;
    load_texture(file_num);
    const TexEntry &entry = texture_cache[file_num];
    int mWidth = entry.tex.w, mHeight = entry.tex.h;
    int scaled_h = width * mHeight / mWidth;
    draw_entry(batch, entry, 0, 0, mWidth, mHeight, x, y_bottom - scaled_h, width, scaled_h, angle);
}

/*
Load & draw with width scaled by height: width = height * mWidth / mHeight
    Load: Texture
    Draw: batch.drawQuad
    mHeight, mWidth: from the texture entry
*/
void Window::draw_png_width_scaled(int file_num, int x, int y, int height, int angle)
{
    if (file_num == NULL_DIRECTORY || height <= 0)
        return;
    load_texture(file_num);
    const TexEntry &entry = texture_cache[file_num];
    int mWidth = entry.tex.w, mHeight = entry.tex.h;
    draw_entry(batch, entry, 0, 0, mWidth, mHeight, x, y, height * mWidth / mHeight, height, angle);
}

/*
Load & draw in the center of window:
    If x1 = -1, center by x (width)
    If y1 = -1, center by y (height)
    Load: Texture
    Draw: batch.drawQuad
    mHeight, mWidth: from the texture entry
*/
void Window::draw_png_center(int file_num, double ratio, int x1, int y1)
{
    if (file_num == NULL_DIRECTORY)
        return;
    load_texture(file_num);
    const TexEntry &entry = texture_cache[file_num];
    int mWidth = entry.tex.w, mHeight = entry.tex.h;
    mWidth = mWidth * ratio;
    mHeight = mHeight * ratio;
    if (x1 == -1)
        x1 = (WINDOW_WIDTH - mWidth) >> 1;
    if (y1 == -1)
        y1 = (WINDOW_HEIGHT - mHeight) >> 1;
    draw_entry(batch, entry, 0, 0, entry.tex.w, entry.tex.h, x1, y1, mWidth, mHeight);
}

/*
Load & draw in the SDL_Rect(x, y, width, height)
    Load: Texture
    Draw: batch.drawQuad
*/
void Window::draw_png(int file_num, int x, int y, int width, int height)
{
    if (file_num == NULL_DIRECTORY || width <= 0 || height <= 0)
        return;
    load_texture(file_num);
    const TexEntry &entry = texture_cache[file_num];
    draw_entry(batch, entry, 0, 0, entry.tex.w, entry.tex.h, x, y, width, height);
}

/*
Load & draw in the SDL_Rect(x, y, width, height) and rotation of angle
    Load: Texture
    Draw: batch.drawQuad (rotated about the destination center)
*/
void Window::draw_png(int file_num, int x, int y, int width, int height, int angle, SDL_RendererFlip flip)
{
    if (file_num == NULL_DIRECTORY || width <= 0 || height <= 0)
        return;
    load_texture(file_num);
    const TexEntry &entry = texture_cache[file_num];
    draw_entry(batch, entry, 0, 0, entry.tex.w, entry.tex.h, x, y, width, height, angle, flip);
}

/*
Load & draw part of an image: from source SDL_Rect(sx, sy, sw, sh) to
destination SDL_Rect(dx, dy, dw, dh). The destination box is honored exactly
(every caller passes its authored box; the aspect call is the caller's).
    Load: Texture
    Draw: batch.drawQuad
*/
void Window::draw_png(int file_num, int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, SDL_RendererFlip flip)
{
    if (file_num == NULL_DIRECTORY || sw <= 0 || sh <= 0 || dw <= 0 || dh <= 0)
        return;
    load_texture(file_num);
    const TexEntry &entry = texture_cache[file_num];
    draw_entry(batch, entry, sx, sy, sw, sh, dx, dy, dw, dh, 0, flip);
}

void Window::draw_nth_frame(int file_num, int sframe, int ori_w, int ori_h,
                            int game_x, int game_y, int game_w, int game_h, int angle)
{
    if (file_num == NULL_DIRECTORY || game_w <= 0 || game_h <= 0 || ori_w <= 0 || ori_h <= 0)
        return;
    if (all_img[file_num].c_sheet == 0)
    {
        cout << "NOT A SHEET IMAGE\n";
        exit(-1234567);
    }
    // current col in source image
    int scol = sframe % all_img[file_num].c_sheet;
    // current row in source image
    int srow = sframe / all_img[file_num].c_sheet;

    draw_png(file_num, ori_w * scol, ori_h * srow, ori_w, ori_h,
             game_x, game_y, game_w, game_h);
}

/*
Load & draw image to all window.
The src rect may exceed the texture (scrolling background); out-of-bounds UVs
sample the edge (GL_CLAMP_TO_EDGE), as SDL_RenderCopy does.
*/
void Window::draw_bg(int file_num, int x, int y)
{
    if (file_num == NULL_DIRECTORY)
        return;
    load_texture(file_num);
    const TexEntry &entry = texture_cache[file_num];
    draw_entry(batch, entry, x, y, WINDOW_WIDTH, WINDOW_HEIGHT,
               0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
}

/*
Set texture alpha mod (persisted per slot, applied at draw time)
*/
void Window::set_texture_alpha(int file_num, int a)
{
    if (file_num != NULL_DIRECTORY && texture_cache[file_num].loaded)
    {
        texture_cache[file_num].alpha = a;
    }
}

/*
Set texture RGB mod (persisted per slot, applied at draw time)
*/
void Window::set_texture_color(int file_num, int r, int g, int b)
{
    if (file_num != NULL_DIRECTORY && texture_cache[file_num].loaded)
    {
        texture_cache[file_num].r = r;
        texture_cache[file_num].g = g;
        texture_cache[file_num].b = b;
    }
}

extern bool quit;
/*
Fade out effect.
Since present() clears the back buffer after every swap (GL swap contents are
undefined), the old accumulate-translucent-black-over-preserved-frame trick no
longer works: present() snapshots every frame into batch.lastFrame(); here we
freeze that snapshot and redraw it under a progressively opaque black overlay.
*/
void Window::fade_out()
{
    batch.setLastFrameFrozen(true);
    const GlTexture &shot = batch.lastFrame();
    load_texture(BLACK_SCREEN_DIRECTORY); // so the alpha set below lands
    for (int i = 0; i <= 64; i++)
    {
        if (shot.id != 0)
            batch.drawQuad(shot, 0, 0, (float)shot.w, (float)shot.h,
                           0, 0, WINDOW_WIDTH, WINDOW_HEIGHT,
                           0.0f, false, true); // flipV: snapshot is GL-origin
        set_texture_alpha(BLACK_SCREEN_DIRECTORY, min(255, i * 4));
        draw_png(BLACK_SCREEN_DIRECTORY, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
        update_screen();
        HANDLE_SDL_EVENT(QUIT())
    }
    batch.setLastFrameFrozen(false);
    set_texture_alpha(BLACK_SCREEN_DIRECTORY, 150);
    clear_renderer();
    delete_all_texture();
}

/*
Delete a game texture
*/
void Window::delete_texture(const int &file_num)
{
    TexEntry &entry = texture_cache[file_num];
    if (entry.loaded)
    {
        batch.free(entry.tex);
        entry.loaded = false;
        entry.alpha = 255;
        entry.r = entry.g = entry.b = 255;
    }
}

/*
Delete all game textures except the mouse cursor
*/
void Window::delete_all_texture()
{
    for (int i = 0; i < COUNT_USED_DIRECTORY; i++)
        if (i != MOUSE_CURSOR_DIRECTORY)
        {
            delete_texture(i);
        }
}

void Window::delete_all_texture_no_use()
{
    for (int i = 0; i < COUNT_USED_DIRECTORY; i++)
        if (i != MOUSE_CURSOR_DIRECTORY &&
            i != CHERRYBOMB_EXPLOSION_DIRECTORY &&
            i != POTATOMINE_EXPLOSION_DIRECTORY &&
            i != BOOM_DIRECTORY &&
            ++last_render[i] > 300)
        {
            delete_texture(i);
        }
}
