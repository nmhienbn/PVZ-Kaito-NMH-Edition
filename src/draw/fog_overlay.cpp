#include "fog_overlay.hpp"
#include "elements/Level/Level.hpp"
#include "elements/Map/Map.hpp"
#include "elements/elements.hpp"
#include "gl/gl_loader.hpp"
#include "rsdl.hpp"
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <vector>

extern int game_state;
extern Level level;
extern Elements game_characters;
extern Map cells;
extern Window win;

namespace fogov
{
const int FOG_BLOWN_TICKS = 20 * 60;   // 20 s at the 60 fps game clock
const int FOG_FADE_IN_TICKS = 3 * 60;  // fog drifts back over the last 3 s
const int PLANTERN_LIT_RADIUS = 185;   // ~2.3 tiles around the Plantern

namespace
{
// fog.jpg is an 8-cel atlas (8 x 210x190), like PvZ1's IMAGE_FOG.
const int FOG_CEL_W = 210;
const int FOG_CEL_H = 190;
const int FOG_CEL_COUNT = 8;

// Grid-size upper bounds for the static per-cell state.
const int MAX_FOG_COLS = 12; // >= HORIZ_BLOCK_COUNT(9) + spare
const int MAX_FOG_ROWS = 8;  // >= grid rows(6) + 1 extra row below (Board draws MAX_GRID_SIZE_Y+1)

// Per-cell fog, mirroring Board::mGridCelFog (alpha) and mGridCelLook
// (atlas look 0..19, cel = look % 8). Alpha ramps +3/tick toward 255
// (200 on the leftmost fog column), and Planterns erode it back at -6/tick.
int fog_alpha[MAX_FOG_COLS][MAX_FOG_ROWS];
int fog_look[MAX_FOG_COLS][MAX_FOG_ROWS];

float blow_offset = 0;  // Blover wind: slides the whole bank right, eases back
int fog_counter = 0;    // own tick counter for the color shimmer phase

void invalidate_layer(); // defined with the layer composer below

int blown_ticks = 0;   // Blover: remaining ticks of clear sky
int seen_level = -999; // lazy per-level reset (level_num changes per mission)

int grid_rows() // playable grid height; +1 like Board's extra fog row
{
    return cells.size() ? (int)cells.size() + 1 : 0;
}

void reset_level_state()
{
    blown_ticks = 0;
    blow_offset = 0;
    fog_counter = 0;
    for (int x = 0; x < MAX_FOG_COLS; x++)
        for (int y = 0; y < MAX_FOG_ROWS; y++)
        {
            fog_alpha[x][y] = 0; // fog rolls in at mission start
            fog_look[x][y] = rand() % 20;
        }
    invalidate_layer();
}

// Blover push distance: far enough to slide even the first fog column's
// (overlapping) cel fully past the right screen edge.
float blow_push_px()
{
    if (cells.size() && level.fog_column >= 0 && level.fog_column < HORIZ_BLOCK_COUNT)
        return (float)(WINDOW_WIDTH - cells[0][level.fog_column].x1 + FOG_CEL_W);
    return 800.f;
}

// Board::ClearFogAroundPlant: fade the fog out in a diamond around the plant.
// size 4 (Plantern): |dx|<=3, |dy|<=2, |dx|+|dy|<=4; other sizes: |dx|+|dy|<=size.
// The whole pattern shifts with the fog offset so holes track the bank.
void clear_fog_around(int prow, int pcol, int size)
{
    const int CLEAR_SPEED = 6;
    int off_cols = (int)((blow_offset + BLOCK_WIDTH / 2) / BLOCK_WIDTH);
    int rmax = grid_rows() - 1; // includes the extra row below the lawn
    if (rmax < 0)
        return;
    for (int x = level.fog_column; x < HORIZ_BLOCK_COUNT && x < MAX_FOG_COLS; x++)
    {
        int dist_x = abs(x + off_cols - pcol);
        if (dist_x > size)
            continue;
        for (int y = 0; y <= rmax && y < MAX_FOG_ROWS; y++)
        {
            int dist_y = abs(y - prow);
            if (size == 4)
            {
                if (dist_x > 3 || dist_y > 2 || dist_x + dist_y > 4)
                    continue;
            }
            else if (dist_x + dist_y > size)
                continue;
            fog_alpha[x][y] = fog_alpha[x][y] > CLEAR_SPEED ? fog_alpha[x][y] - CLEAR_SPEED : 0;
        }
    }
}
} // namespace

void blow_away(int ticks)
{
    blown_ticks = ticks;
}

bool is_blown()
{
    return blown_ticks > FOG_FADE_IN_TICKS;
}

bool is_tile_lit(int row, int col)
{
    if (row < 0 || row >= (int)cells.size() || col < 0 || col >= HORIZ_BLOCK_COUNT)
        return false;
    int cx = (cells[row][col].x1 + cells[row][col].x2) / 2;
    int cy = (cells[row][col].y1 + cells[row][col].y2) / 2;
    for (const auto &p : game_characters.plants)
    {
        if (p->get_type() != PLANTERN_TYPE)
            continue;
        int pr = p->get_row(), pc = p->get_col();
        int px = (cells[pr][pc].x1 + cells[pr][pc].x2) / 2;
        int py = (cells[pr][pc].y1 + cells[pr][pc].y2) / 2;
        int dx = cx - px, dy = cy - py;
        if (dx * dx + dy * dy <= PLANTERN_LIT_RADIUS * PLANTERN_LIT_RADIUS)
            return true;
    }
    return false;
}

/*One logic tick of fog sim (Board::UpdateFog): lazy per-level reset, Blover
wind offset, per-cell fade-in and Plantern/Torchwood holes; the body runs once
per logic tick covered by the current rendered frame (auto-FPS). The original
never scrolls the fog (mFogOffset moves only for the level intro and the Blover
blow-out), and the color wobble already keeps it alive, so there is no
continuous drift.*/
void update()
{
    if (seen_level != level.level_num)
    {
        seen_level = level.level_num;
        reset_level_state();
    }
    // Auto-FPS: the whole fog simulation (fog_counter, blow_offset lerp,
    // fog_alpha fade, erosion, blown_ticks) advances once per LOGIC tick,
    // not per rendered frame -- display() is update()'s only caller, and
    // the render rate is uncapped, so one frame can cover several logic
    // ticks (turbo). Pause freezes it (logic_ticked holds a stale value
    // while paused, so the IS_PAUSED check stays in the loop condition).
    // The win-screen / Dave loops force logic_ticked = 1, so the fog
    // keeps moving there.
    if (level.fog_column < 0 || level.fog_column >= HORIZ_BLOCK_COUNT)
        return;
    for (int _lt = logic_ticked;
         _lt > 0 && check_status(game_state, IS_PAUSED) == false; _lt--)
    {
        fog_counter++;

        // Blover wind: shove the bank off the right edge fast, ease it back
        // over the last FOG_FADE_IN_TICKS of the blow.
        float target = blown_ticks > FOG_FADE_IN_TICKS ? blow_push_px() : 0.f;
        blow_offset += (target - blow_offset) * (target > blow_offset ? 0.09f : 0.03f);

        // Fog regrows toward its cap; the wind keeps it thin while it is up.
        int fade_in = blown_ticks > 0 ? 2 : 3;
        int rmax = grid_rows();
        for (int x = level.fog_column; x < HORIZ_BLOCK_COUNT && x < MAX_FOG_COLS; x++)
        {
            int cap = x == level.fog_column ? 200 : 255;
            for (int y = 0; y < rmax && y < MAX_FOG_ROWS; y++)
                fog_alpha[x][y] = fog_alpha[x][y] + fade_in > cap ? cap : fog_alpha[x][y] + fade_in;
        }

        // Light bubbles: when a plant dies the fog simply fades back in above.
        for (const auto &p : game_characters.plants)
        {
            if (p->get_type() == PLANTERN_TYPE)
                clear_fog_around(p->get_row(), p->get_col(), 4);
            else if (p->get_type() == TORCHWOOD_TYPE)
                clear_fog_around(p->get_row(), p->get_col(), 1);
        }

        if (blown_ticks > 0)
            blown_ticks--;
    }
}

/*Draw the fog bank (Board::DrawFog), as one composed RGBA layer.

Why a layer instead of per-cel blits: PvZ1 blits each 210x190 fog cel with a
per-cell SetColor(alpha). Two things made that ugly here:
  1. The fog atlas is a JPEG: surfaces without an alpha channel get their
     alpha MOD IGNORED by the GL sprite path (rsdl_img.cpp draw_entry), so
     every cel rendered at full opacity — no fade-in, plantern holes were
     painted over by opaque neighbor cels, and the whole fog bank snapped on
     instead of rolling in.
  2. Even with alpha honoured, per-cell constant alpha gives razor-straight
     vertical alpha steps on cel borders; the original gets away with it
     because 2.6x cel overlap lets neighbour cels bleed over cleared cells.

Mapping to the original (repvz Board::DrawFog / UpdateFog):
  - Same 8-cel 210x190 atlas, same anchors (col*BLOCK_WIDTH - 15, row base -
    (190 - row_step)/2 - 10), same look table, same color-variant wobble
    (255 - look*1.5 - motion*1.5) and the same +3/tick ramp to 255 (cap 200
    on the leftmost fog column). The texture a pixel shows is the topmost
    covering cel in the original painter order (column outer, row inner),
    exactly like DrawFog's x-layout including the extra street cel at col 9.
  - The ALPHA is the one deliberate deviation: instead of constant per-cell,
    it is bilinear interpolation of the cell-alpha lattice anchored at cell
    centres (so interior cells at 255 still render identically to the
    original, but bank edges and plantern/torchwood holes ramp smoothly over
    ~one block instead of stepping on cel borders).
  - The alpha lattice is static in board space while the cels drift/slide
    under it (mFogOffset model), so Blover shoves the bank off-screen while
    plantern holes stay planted.
Runs after the zombies so they sink into the fog, before suns/UI so the
interface stays readable.*/
namespace
{
bool atlas_loaded = false;
std::vector<uint32_t> atlas; // RGBA little-endian, 1680x190

GlTexture layer_tex;
std::vector<uint32_t> layer_px; // CPU side, A<<24 | B<<16 | G<<8 | R
int layer_w = 0, layer_h = 0, layer_x0 = 0, layer_y0 = 0;

void invalidate_layer()
{
    if (layer_tex.id)
        win.sprite_batch().free(layer_tex);
    layer_w = layer_h = 0;
    layer_px.clear();
}

// Per-frame caches (recomputed by compute_frame()).
int cel_alpha[MAX_FOG_COLS + 1][MAX_FOG_ROWS]; // effective per-cel alpha, col 9 = street cel
int cel_tint[MAX_FOG_COLS + 1][MAX_FOG_ROWS][3];
int lat_col0, lat_col_cnt, lat_row_cnt;    // lattice: cols lat_col0..lat_col0+cnt-1
int lat_val[MAX_FOG_COLS + 1][MAX_FOG_ROWS];
int geo_row_step, geo_base_y0, geo_dy0;    // dy(r) = geo_dy0 + r * geo_row_step

bool ensure_atlas()
{
    if (atlas_loaded)
        return true;
    SDL_Surface *surf = IMG_Load(all_img[FOG_DIRECTORY].img_dir.c_str());
    if (!surf)
        return false;
    SDL_Surface *rgba = SDL_ConvertSurfaceFormat(surf, SDL_PIXELFORMAT_RGBA32, 0);
    SDL_FreeSurface(surf);
    if (!rgba || rgba->w < FOG_CEL_W * FOG_CEL_COUNT || rgba->h < FOG_CEL_H)
    {
        if (rgba)
            SDL_FreeSurface(rgba);
        return false;
    }
    atlas.resize((size_t)rgba->w * rgba->h);
    memcpy(atlas.data(), rgba->pixels, atlas.size() * 4);
    int w = rgba->w; // RGBA32 little-endian == uint32 A<<24|B<<16|G<<8|R
    SDL_FreeSurface(rgba);
    (void)w;
    atlas_loaded = true;
    return true;
}

/*Recompute the per-frame caches: lattice values, per-cel tints, left-feather
anchors. The alpha lattice spans columns fog_column .. 9 (street cel = col 8's
values) anchored at cell centres; the left feather starts at the leftmost
drawn cel's edge so entering the bank never steps alpha.*/
int left_z[MAX_FOG_ROWS]; // leftmost active cel's left edge per lattice row
const int RAMP_X = 64;    // left feather slope width (px)
const int RAMP_Y = 40;    // top/bottom feather slope width (px)

int col_center(int col) // lattice anchor: board column centre
{
    return X_UPPER_LEFT + col * BLOCK_WIDTH + BLOCK_WIDTH / 2;
}

int row_center(int row, int n_rows)
{
    int ry = row < n_rows ? cells[row][0].y1 : geo_base_y0 + row * geo_row_step;
    return ry + geo_row_step / 2;
}

bool compute_frame(int n_rows, int off_i)
{
    const int fc = level.fog_column;
    geo_row_step = n_rows > 1 ? cells[1][0].y1 - cells[0][0].y1 : GRID_BLOCK_H;
    geo_base_y0 = cells[0][0].y1;
    geo_dy0 = geo_base_y0 - (FOG_CEL_H - geo_row_step) / 2 - 10;
    lat_col0 = fc;
    lat_col_cnt = 9 - lat_col0 + 1; // through the street cel column 9
    lat_row_cnt = n_rows + 1;

    for (int cx = fc; cx <= HORIZ_BLOCK_COUNT; cx++)
    {
        int col = cx >= HORIZ_BLOCK_COUNT ? HORIZ_BLOCK_COUNT - 1 : cx;
        for (int r = 0; r < lat_row_cnt && r < MAX_FOG_ROWS; r++)
        {
            int alpha = fog_alpha[col][r];
            cel_alpha[cx][r] = alpha;
            lat_val[cx][r] = alpha;
            int look = fog_look[col][r % n_rows] % 20;

            // Board::DrawFog periodic color wobble (per-column/row phase).
            float t = (float)fog_counter * 3.14159265f * 2.f;
            float phase_x = 6.f * 3.14159265f * col / HORIZ_BLOCK_COUNT;
            float phase_y = 6.f * 3.14159265f * r / (float)(n_rows + 1);
            float motion = 13.f + 4.f * sinf(t / 900.f + phase_y) + 8.f * sinf(t / 500.f + phase_x);
            int color_variant = (int)(255 - look * 1.5f - motion * 1.5f);
            int light_variant = (int)(255 - look - motion);
            if (color_variant < 0)
                color_variant = 0;
            if (light_variant < 0)
                light_variant = 0;
            cel_tint[cx][r][0] = cel_tint[cx][r][1] = color_variant;
            cel_tint[cx][r][2] = light_variant;
        }
    }
    for (int r = 0; r < lat_row_cnt && r < MAX_FOG_ROWS; r++)
    {
        left_z[r] = WINDOW_WIDTH + 4096; // no active cel on this row line
        for (int cx = fc; cx <= HORIZ_BLOCK_COUNT; cx++)
            if (cel_alpha[cx][r] > 0)
            {
                left_z[r] = X_UPPER_LEFT + cx * BLOCK_WIDTH - 15 + off_i;
                break;
            }
    }
    return true;
}

/*Bilinear-interpolated coverage alpha at a screen point (0..255), feathered
to zero at the bank's outer cel edges (left edge of the leftmost active cel,
top of the first row, bottom of the last drawn row) so no cel rectangle ever
shows a hard alpha step.*/
int field_alpha(int x, int y, int n_rows)
{
    float cf = (float)(x - col_center(lat_col0)) / BLOCK_WIDTH;
    float rf = (float)(y - row_center(0, n_rows)) / geo_row_step;
    if (cf < 0)
        cf = 0;
    if (cf > lat_col_cnt - 1.f)
        cf = lat_col_cnt - 1.f;
    if (rf < 0)
        rf = 0;
    if (rf > lat_row_cnt - 1.f)
        rf = lat_row_cnt - 1.f;
    int c0 = (int)cf, r0 = (int)rf;
    float ft = cf - c0, fr = rf - r0;
    int i00 = lat_val[lat_col0 + c0][r0], i10 = lat_val[lat_col0 + c0 + 1][r0];
    int i01 = lat_val[lat_col0 + c0][r0 + 1], i11 = lat_val[lat_col0 + c0 + 1][r0 + 1];
    float top = i00 + (i10 - i00) * ft;
    float bot = i01 + (i11 - i01) * ft;
    int a = (int)(top + (bot - top) * fr + 0.5f);
    if (a <= 0)
        return 0;

    float z = left_z[r0] + (left_z[r0 + 1] - left_z[r0]) * fr;
    float feather = (x - z) / RAMP_X;
    int top_y = geo_dy0;
    int bot_y = geo_dy0 + n_rows * geo_row_step + FOG_CEL_H;
    float fy = (y - top_y) / (float)RAMP_Y;
    if (bot_y < WINDOW_HEIGHT)
    {
        float fb = (bot_y - y) / (float)RAMP_Y;
        if (fb < fy)
            fy = fb;
    }
    if (fy < feather)
        feather = fy;
    if (feather >= 1.f)
        return a;
    if (feather <= 0.f)
        return 0;
    return (int)(a * feather + 0.5f);
}

/*Pick the topmost covering cel in DrawFog's painter order (column outer,
then row, both ascending) whose effective alpha > 0. Returns false when no
cel covers this point (also kills alpha: the bank is not there at all).*/
bool pick_cel(int x, int y, int n_rows, int off_i, int &out_u, int &out_v, int &out_col, int &out_row)
{
    const int fc = level.fog_column;
    float relx = x - off_i - (X_UPPER_LEFT - 15.f);
    float rely = y - (float)geo_dy0;
    if (relx < 0 || rely < 0)
        return false;
    int cx_hi = (int)floorf(relx / BLOCK_WIDTH);
    int cx_lo = (int)floorf((relx - FOG_CEL_W + 1) / BLOCK_WIDTH);
    int ry_hi = (int)floorf(rely / geo_row_step);
    int ry_lo = (int)floorf((rely - FOG_CEL_H + 1) / geo_row_step);
    if (cx_lo < fc)
        cx_lo = fc;
    if (cx_hi > HORIZ_BLOCK_COUNT)
        cx_hi = HORIZ_BLOCK_COUNT; // col 9 = the extra street cel
    if (ry_hi > n_rows)
        ry_hi = n_rows;
    if (ry_lo < 0)
        ry_lo = 0;
    for (int cx = cx_hi; cx >= cx_lo; cx--)
        for (int ry = ry_hi; ry >= ry_lo; ry--)
            if (cel_alpha[cx][ry] > 0)
            {
                out_col = cx;
                out_row = ry;
                out_u = (int)(relx - cx * BLOCK_WIDTH);
                out_v = (int)(rely - ry * geo_row_step);
                if (out_u < 0)
                    out_u = 0;
                if (out_u >= FOG_CEL_W)
                    out_u = FOG_CEL_W - 1;
                if (out_v < 0)
                    out_v = 0;
                if (out_v >= FOG_CEL_H)
                    out_v = FOG_CEL_H - 1;
                return true;
            }
    return false;
}
} // namespace

void display()
{
    if (level.fog_column < 0 || level.fog_column >= HORIZ_BLOCK_COUNT)
        return;
    update();
    int n_rows = cells.size();
    if (n_rows == 0 || level.playable_rows.empty())
        return; // grid not loaded yet
    if (!ensure_atlas())
        return;

    const int fc = level.fog_column;
    int off_i = (int)blow_offset;
    if (!compute_frame(n_rows, off_i))
        return;

    // Layer rect: from the first fog column's cel anchor (unshifted) to the
    // right window edge; vertical span of every cel row.
    int x0 = X_UPPER_LEFT + fc * BLOCK_WIDTH - 15;
    if (x0 < 0)
        x0 = 0;
    int y0 = geo_dy0 > 0 ? geo_dy0 : 0;
    int y1 = geo_dy0 + n_rows * geo_row_step + FOG_CEL_H;
    if (y1 > WINDOW_HEIGHT)
        y1 = WINDOW_HEIGHT;
    // The whole bank slid past the right edge (full Blover): nothing to draw.
    if (X_UPPER_LEFT + fc * BLOCK_WIDTH - 15 + off_i >= WINDOW_WIDTH)
        return;

    int W = WINDOW_WIDTH - x0, H = y1 - y0;
    if (W <= 0 || H <= 0)
        return;
    GlSpriteBatch &batch = win.sprite_batch();
    // Compose at half resolution: the coverage field is bilinear-smooth by
    // construction and the atlas cels are soft lumps, so the GPU's LINEAR
    // upscale back to screen size is visually identical at 4x less CPU.
    const int CW = (W + 1) / 2, CH = (H + 1) / 2;
    if (!layer_tex.id || layer_w != CW || layer_h != CH)
    {
        if (layer_tex.id)
            batch.free(layer_tex);
        SDL_Surface *empty = SDL_CreateRGBSurfaceWithFormat(0, CW, CH, 32, SDL_PIXELFORMAT_RGBA32);
        if (!empty)
            return;
        memset(empty->pixels, 0, (size_t)CW * CH * 4);
        layer_tex = batch.upload(empty);
        SDL_FreeSurface(empty);
        layer_w = CW;
        layer_h = CH;
        layer_px.assign((size_t)CW * CH, 0);
        layer_x0 = x0;
        layer_y0 = y0;
    }

    // 1D precomputes: per composed-x column the cel candidates + lattice cell
    // pair; per composed-y row the same vertically plus the feathers. The
    // pixel loop then does a couple of table lookups per pixel.
    static std::vector<int> cx_lo_a, cx_hi_a, c10_a, x_a;
    static std::vector<float> ft_a, rex_a;
    static std::vector<int> ry_lo_a, ry_hi_a, r10_a, y_a;
    static std::vector<float> fr_a, feather_y_a, z_a;
    cx_lo_a.resize(CW); cx_hi_a.resize(CW); c10_a.resize(CW); ft_a.resize(CW); rex_a.resize(CW); x_a.resize(CW);
    ry_lo_a.resize(CH); ry_hi_a.resize(CH); r10_a.resize(CH);
    fr_a.resize(CH); feather_y_a.resize(CH); z_a.resize(CH); y_a.resize(CH);

    for (int lx = 0; lx < CW; lx++)
    {
        int x = x0 + lx * 2;
        x_a[lx] = x;
        float relx = x - off_i - (X_UPPER_LEFT - 15.f);
        rex_a[lx] = relx;
        if (relx < 0)
        {
            cx_hi_a[lx] = -1;
            cx_lo_a[lx] = 0;
        }
        else
        {
            int hi = (int)floorf(relx / BLOCK_WIDTH);
            int lo = (int)floorf((relx - FOG_CEL_W + 1) / BLOCK_WIDTH);
            if (hi > HORIZ_BLOCK_COUNT)
                hi = HORIZ_BLOCK_COUNT;
            if (lo < level.fog_column)
                lo = level.fog_column;
            cx_hi_a[lx] = hi;
            cx_lo_a[lx] = lo;
        }
        float cf = (float)(x - col_center(lat_col0)) / BLOCK_WIDTH;
        if (cf < 0)
            cf = 0;
        if (cf > lat_col_cnt - 1.f)
            cf = lat_col_cnt - 1.f;
        int c0 = (int)cf;
        c10_a[lx] = c0;
        ft_a[lx] = cf - c0;
    }
    for (int ly = 0; ly < CH; ly++)
    {
        int y = layer_y0 + ly * 2;
        y_a[ly] = y;
        float rely = (float)(y - geo_dy0);
        int hi = (int)floorf(rely / geo_row_step);
        int lo = (int)floorf((rely - FOG_CEL_H + 1) / geo_row_step);
        if (hi > n_rows)
            hi = n_rows;
        if (lo < 0)
            lo = 0;
        ry_hi_a[ly] = rely < 0 ? -1 : hi;
        ry_lo_a[ly] = lo;

        float rf = (float)(y - row_center(0, n_rows)) / geo_row_step;
        if (rf < 0)
            rf = 0;
        if (rf > lat_row_cnt - 1.f)
            rf = lat_row_cnt - 1.f;
        int r0 = (int)rf;
        float fr = rf - r0;
        r10_a[ly] = r0;
        fr_a[ly] = fr;
        z_a[ly] = left_z[r0] + (left_z[r0 + 1] - left_z[r0]) * fr;

        float fy = (y - geo_dy0) / (float)RAMP_Y;
        int bot_y = geo_dy0 + n_rows * geo_row_step + FOG_CEL_H;
        if (bot_y < WINDOW_HEIGHT)
        {
            float fb = (bot_y - y) / (float)RAMP_Y;
            if (fb < fy)
                fy = fb;
        }
        feather_y_a[ly] = fy;
    }

    memset(layer_px.data(), 0, (size_t)CW * CH * 4);
    const int atlas_stride = FOG_CEL_W * FOG_CEL_COUNT;
    for (int ly = 0; ly < CH; ly++)
    {
        uint32_t *row = layer_px.data() + (size_t)ly * CW;
        int ry_hi = ry_hi_a[ly];
        if (ry_hi < 0)
            continue;
        int ry_lo = ry_lo_a[ly];
        int r0 = r10_a[ly];
        float fr = fr_a[ly];
        float fy = feather_y_a[ly];
        if (fy <= 0.f)
            continue; // above the top cel edge feather
        float z = z_a[ly];
        const int y = y_a[ly];
        for (int lx = 0; lx < CW; lx++)
        {
            int cx_hi = cx_hi_a[lx];
            if (cx_hi < 0)
                continue;
            float rex = rex_a[lx];
            // topmost cel in painter order: greatest column, greatest row
            int col = -1, r = -1;
            for (int cx = cx_hi; cx >= cx_lo_a[lx] && col < 0; cx--)
                for (int ry = ry_hi; ry >= ry_lo; ry--)
                    if (cel_alpha[cx][ry] > 0)
                    {
                        col = cx;
                        r = ry;
                        break;
                    }
            if (col < 0)
                continue;

            float feather = ((float)x_a[lx] - z) / RAMP_X;
            if (fy < feather)
                feather = fy;
            if (feather <= 0.f)
                continue;

            int c0 = c10_a[lx];
            float ft = ft_a[lx];
            int i00 = lat_val[lat_col0 + c0][r0], i10 = lat_val[lat_col0 + c0 + 1][r0];
            int i01 = lat_val[lat_col0 + c0][r0 + 1], i11 = lat_val[lat_col0 + c0 + 1][r0 + 1];
            float topa = i00 + (i10 - i00) * ft;
            float bota = i01 + (i11 - i01) * ft;
            int a = (int)(topa + (bota - topa) * fr + 0.5f);
            if (a <= 0)
                continue;
            if (feather < 1.f)
                a = (int)(a * feather + 0.5f);
            if (a <= 0)
                continue;
            if (a > 255)
                a = 255;

            int v = (y - geo_dy0) - r * geo_row_step;
            if (v < 0)
                v = 0;
            if (v >= FOG_CEL_H)
                v = FOG_CEL_H - 1;
            int u = (int)(rex - col * BLOCK_WIDTH);
            if (u < 0)
                u = 0;
            if (u >= FOG_CEL_W)
                u = FOG_CEL_W - 1;
            int look = fog_look[col >= HORIZ_BLOCK_COUNT ? HORIZ_BLOCK_COUNT - 1 : col]
                             [r % n_rows] % 20;
            int cel = look % FOG_CEL_COUNT;
            uint32_t src = atlas[(size_t)v * atlas_stride + cel * FOG_CEL_W + u];
            const int *tnt = cel_tint[col][r];
            uint32_t R = ((src & 0xFF) * tnt[0]) / 255;
            uint32_t G = (((src >> 8) & 0xFF) * tnt[1]) / 255;
            uint32_t B = (((src >> 16) & 0xFF) * tnt[2]) / 255;
            row[lx] = ((uint32_t)a << 24) | (B << 16) | (G << 8) | R;
        }
    }

    batch.flush(); // stable GL state before the raw texture re-upload
    pvzgl::glBindTexture(pvzgl::PVZ_GL_TEXTURE_2D, layer_tex.id);
    pvzgl::glTexSubImage2D(pvzgl::PVZ_GL_TEXTURE_2D, 0, 0, 0, layer_w, layer_h,
                           pvzgl::PVZ_GL_RGBA, pvzgl::PVZ_GL_UNSIGNED_BYTE, layer_px.data());
    batch.drawQuad(layer_tex, 0, 0, layer_w, layer_h,
                   layer_x0, layer_y0, W, H);
}

int debug_coverage_alpha(int x, int y)
{
    int n_rows = (int)cells.size();
    if (n_rows == 0 || level.fog_column < 0 || level.fog_column >= HORIZ_BLOCK_COUNT)
        return 0;
    int off_i = (int)blow_offset;
    if (!compute_frame(n_rows, off_i))
        return 0;
    int u, v, col, r;
    if (!pick_cel(x, y, n_rows, off_i, u, v, col, r))
        return 0;
    return field_alpha(x, y, n_rows);
}

void debug_stats(int &tiles, int &alpha_min, int &alpha_max, int &offset)
{
    tiles = 0;
    alpha_min = 255;
    alpha_max = 0;
    if (level.fog_column < 0 || level.fog_column >= HORIZ_BLOCK_COUNT)
    {
        offset = 0;
        return;
    }
    int rmax = grid_rows();
    for (int x = level.fog_column; x < HORIZ_BLOCK_COUNT && x < MAX_FOG_COLS; x++)
        for (int y = 0; y < rmax && y < MAX_FOG_ROWS; y++)
        {
            int a = fog_alpha[x][y];
            if (a > 0)
                tiles++;
            if (a < alpha_min)
                alpha_min = a;
            if (a > alpha_max)
                alpha_max = a;
        }
    offset = (int)blow_offset;
}
} // namespace fogov
