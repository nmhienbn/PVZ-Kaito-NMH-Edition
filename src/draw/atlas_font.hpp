#pragma once

// PvZ1 bitmap glyph atlases (repvz ImageFont): each glyph comes pre-baked
// with its effects (green inset fill + dark rind, yellow face + keyline)
// in a PNG atlas; the Sexy SExpr .txt descriptor supplies per-char advance
// widths, atlas rects, draw offsets and kerning pairs. Drawing blits the
// glyph rects through the sprite batch -- the label look is the retail
// bitmap font pixel-for-pixel (hover swaps the whole atlas GreenInset ->
// BrightGreenInset, exactly like DrawStoneButton's font swap), no runtime
// shadow/outline passes.
//
// Descriptor format (see day_decor/forensic/showdown.py for the reference
// Python parser): `Define CharList ( 'A', ...)`, `WidthList`, `RectList
// ( (x,y,w,h), ...)`, `OffsetList ( (ox,oy), ...)`, `KerningPairs
// ("AV",...)` + `KerningValues`, then per layer `CreateLayer X;
// LayerSetImage X 'name'; LayerSetAscent X n; LayerSetCharWidths X CharList
// WidthList; LayerSetCharWidths X (' ') (n); LayerSetKerningPairs X ...;
// LayerSetImageMap X CharList RectList; LayerSetCharOffsets X ...`.
// Files are cp1252: char tokens are matched as raw bytes. Multiple layers
// are drawn in CreateLayer order (e.g. Outline under Main).

#include "draw/rsdl.hpp"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

class AtlasFont
{
public:
    // Lazy singletons over resources/fonts/pvz1font/<stem>.txt (+ the layer
    // PNGs named inside the descriptor, uploaded on first draw).
    static const AtlasFont &greenInset();          // DwarvenTodcraft18GreenInset (stone-button labels)
    static const AtlasFont &greenInsetBright();    // 18 BrightGreenInset (hover state)
    static const AtlasFont &greenInset36();        // 36 GreenInset (Back To Game)
    static const AtlasFont &greenInsetBright36();  // 36 hover
    static const AtlasFont &yellow();              // 18 Yellow (dialog/award/chooser titles)
    // Generic access keyed by descriptor stem; cached per stem.
    static const AtlasFont &get(const std::string &stem);

    bool ok() const { return mOk; }
    int point_size() const { return mPointSize; }
    int cell_height() const { return mCellH; } // full glyph cell, px at scale 1
    int space_width() const { return mSpaceW; }

    // Advance width of the string at scale 1, kerning applied; chars missing
    // from the atlas (and ' ') advance by the descriptor space width.
    int string_width(const std::string &text) const;

    // Ink bounds of the string at scale 1 relative to the cell top passed to
    // draw(): min/max over glyphs (and layers) of the non-transparent pixel
    // rows. Strings with no ink (e.g. all spaces) return top=0, bottom=cell.
    void ink_bounds(const std::string &text, float &top, float &bottom) const;

    // Draw with the glyph cell top-left at (x, y); centered=true re-anchors
    // so the advance box is horizontally centered on x. scale multiplies
    // every advance/rect/offset; tint multiplies the baked glyph colors.
    void draw(Window &win, const std::string &text, int x, int y,
              bool centered = false, float scale = 1.0f,
              const RGB &tint = WHITE, int alpha = 255) const;

    // Draw centered on (center_x, center_y): advance box horizontally, real
    // ink bounds vertically (what the old TTF surface centering did).
    void draw_centered(Window &win, const std::string &text, int center_x, int center_y,
                       float scale = 1.0f, const RGB &tint = WHITE, int alpha = 255) const;

private:
    explicit AtlasFont(const std::string &stem);

    struct Glyph
    {
        int width = 0;    // advance
        SDL_Rect rect = {0, 0, 0, 0};
        int ox = 0, oy = 0;          // draw offset (cell space)
        int ink_top = 0, ink_bot = 0; // alpha bounds within rect (cell space)
    };
    struct Layer
    {
        std::string image;
        std::vector<int> index;              // 256 entries: byte -> glyph index, -1
        std::vector<Glyph> glyphs;
        std::unordered_map<uint32_t, int> kern; // (prev<<8)|cur -> advance dx
        int space_w = 10;
        mutable GlTexture tex;
        mutable bool tex_tried = false;
    };

    std::string mDir;               // resources/fonts/pvz1font
    std::string mStem;
    std::vector<Layer> mLayers;
    int mSpaceW = 10;
    int mCellH = 0;
    int mPointSize = 0;
    bool mOk = false;

    void ensure_textures(Window &win) const;
};
