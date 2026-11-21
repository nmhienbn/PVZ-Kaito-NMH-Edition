#include "atlas_font.hpp"

#include <SDL_image.h>

#include <array>
#include <cctype>
#include <cstdio>
#include <cstring>
#include <memory>

namespace
{

const char *PVZ1_FONT_DIR = "./resources/fonts/pvz1font/";

struct DefList
{
    std::vector<std::string> strs;          // CharList / KerningPairs
    std::vector<int> ints;                  // WidthList / KerningValues
    std::vector<std::array<int, 4>> rects4; // RectList
    std::vector<std::array<int, 2>> pairs2; // OffsetList
};

bool ends_with(const std::string &s, const char *suffix)
{
    size_t n = strlen(suffix);
    return s.size() >= n && s.compare(s.size() - n, n, suffix) == 0;
}

// Integer run starting at s[i] (optional '-'). Returns false when s[i] does
// not start a number; advances i past the digits on success.
bool take_int(const std::string &s, size_t &i, int &v)
{
    size_t j = i;
    bool neg = false;
    if (j < s.size() && s[j] == '-')
    {
        neg = true;
        j++;
    }
    if (j >= s.size() || !isdigit((unsigned char)s[j]))
        return false;
    v = 0;
    while (j < s.size() && isdigit((unsigned char)s[j]))
        v = v * 10 + (s[j++] - '0');
    if (neg)
        v = -v;
    i = j;
    return true;
}

// Token between matching quotes ('...' or "...") at s[i] == quote; empty
// optional. Advances i past the closing quote. Unterminated -> rest of body.
std::string take_quoted(const std::string &s, size_t &i)
{
    char q = s[i++];
    size_t j = s.find(q, i);
    std::string tok = j == std::string::npos ? s.substr(i) : s.substr(i, j - i);
    i = (j == std::string::npos ? s.size() : j + 1);
    return tok;
}

void parse_define_body(const std::string &name, const std::string &body, DefList &out)
{
    if (ends_with(name, "RectList"))
    {
        // ( (x,y,w,h), (x,y,w,h), ... ): groups of exactly 4 ints in parens.
        for (size_t i = 0; i < body.size(); i++)
        {
            if (body[i] != '(')
                continue;
            i++;
            std::array<int, 4> r;
            int n = 0;
            while (n < 4)
            {
                while (i < body.size() && (isspace((unsigned char)body[i]) || body[i] == ','))
                    i++;
                if (i >= body.size() || !take_int(body, i, r[n]))
                    break;
                n++;
            }
            if (n == 4)
                out.rects4.push_back(r);
        }
    }
    else if (ends_with(name, "OffsetList"))
    {
        for (size_t i = 0; i < body.size(); i++)
        {
            if (body[i] != '(')
                continue;
            i++;
            std::array<int, 2> r;
            int n = 0;
            while (n < 2)
            {
                while (i < body.size() && (isspace((unsigned char)body[i]) || body[i] == ','))
                    i++;
                if (i >= body.size() || !take_int(body, i, r[n]))
                    break;
                n++;
            }
            if (n == 2)
                out.pairs2.push_back(r);
        }
    }
    else if (ends_with(name, "WidthList") || ends_with(name, "KerningValues"))
    {
        for (size_t i = 0; i < body.size(); i++)
            if (isdigit((unsigned char)body[i]) || body[i] == '-')
            {
                int v;
                if (take_int(body, i, v))
                    out.ints.push_back(v);
            }
    }
    else // CharList / KerningPairs: quoted tokens, raw bytes (cp1252-safe).
    {
        for (size_t i = 0; i < body.size(); i++)
            if (body[i] == '\'' || body[i] == '"')
                out.strs.push_back(take_quoted(body, i));
    }
}

// Parenthesized group at s[i] == '(' -> its inner content; advances past ')'.
std::string take_group(const std::string &s, size_t &i)
{
    size_t j = s.find(')', i);
    std::string g = j == std::string::npos ? s.substr(i + 1) : s.substr(i + 1, j - i - 1);
    i = (j == std::string::npos ? s.size() : j + 1);
    return g;
}

} // namespace

AtlasFont::AtlasFont(const std::string &stem) : mDir(PVZ1_FONT_DIR), mStem(stem)
{
    const std::string path = mDir + "/" + stem + ".txt";
    FILE *f = fopen(path.c_str(), "rb");
    if (f == nullptr)
    {
        fprintf(stderr, "atlas_font: cannot open %s\n", path.c_str());
        return;
    }
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);
    std::string txt((size_t)len, '\0');
    if (fread(&txt[0], 1, (size_t)len, f) != (size_t)len)
        fprintf(stderr, "atlas_font: short read on %s\n", path.c_str());
    fclose(f);

    // 1) Named Define lists (shared across layers).
    std::unordered_map<std::string, DefList> defs;
    std::vector<std::pair<std::string, size_t>> define_at; // name -> body start
    for (size_t p = 0; (p = txt.find("Define ", p)) != std::string::npos; p += 7)
    {
        // "Define" must start a line.
        if (p > 0 && txt[p - 1] != '\n' && txt[p - 1] != '\r')
            continue;
        size_t nb = p + 7, ne = txt.find_first_of("\r\n (", nb);
        define_at.push_back({txt.substr(nb, ne - nb), ne});
    }
    for (size_t d = 0; d < define_at.size(); d++)
    {
        size_t body_end = d + 1 < define_at.size() ? define_at[d + 1].second : std::string::npos;
        // bounded also by the first CreateLayer
        size_t cl = txt.find("\nCreateLayer", define_at[d].second);
        if (cl != std::string::npos && cl < body_end)
            body_end = cl;
        parse_define_body(define_at[d].first,
                          txt.substr(define_at[d].second, body_end - define_at[d].second),
                          defs[define_at[d].first]);
    }

    // 2) Layer commands, in file order.
    std::unordered_map<std::string, size_t> layer_by_name;
    std::unordered_map<std::string, int> inline_w; // pending inline char widths
    size_t pos = 0;
    while ((pos = txt.find('\n', pos)) != std::string::npos)
    {
        pos++;
        size_t eol = txt.find('\n', pos);
        std::string line = txt.substr(pos, (eol == std::string::npos ? txt.size() : eol) - pos);
        // tokenize on whitespace (quoted strings kept whole)
        std::vector<std::string> tok;
        for (size_t i = 0; i < line.size();)
        {
            while (i < line.size() && isspace((unsigned char)line[i]))
                i++;
            if (i >= line.size())
                break;
            std::string t;
            while (i < line.size() && !isspace((unsigned char)line[i]) && line[i] != ';' &&
                   line[i] != '(' && line[i] != ')')
            {
                if (line[i] == '\'' || line[i] == '"')
                    t += take_quoted(line, i);
                else
                    t += line[i++];
            }
            if (!t.empty())
                tok.push_back(t);
            else if (i < line.size())
                i++; // lone delimiter ('(', ')', ';') -- not a token itself
        }
        if (tok.empty())
            continue;

        if (tok[0] == "CreateLayer" && tok.size() >= 2)
        {
            layer_by_name[tok[1]] = mLayers.size();
            mLayers.push_back(Layer());
            mLayers.back().index.assign(256, -1);
            continue;
        }
        if (tok[0].compare(0, 8, "LayerSet") != 0 || tok.size() < 2)
            continue;
        auto lit = layer_by_name.find(tok[1]);
        if (lit == layer_by_name.end())
            continue;
        Layer &L = mLayers[lit->second];
        const std::string &cmd = tok[0];

        if (cmd == "LayerSetImage" && tok.size() >= 3)
        {
            L.image = tok[2];
        }
        else if (cmd == "LayerSetPointSize" && tok.size() >= 3)
        {
            if (mPointSize == 0)
                mPointSize = atoi(tok[2].c_str());
        }
        else if (cmd == "LayerSetCharWidths")
        {
            // Either `L CharList WidthList` or inline `L (' ') (11)` groups.
            size_t p = line.find(tok[1], 8 + 1) + tok[1].size();
            while (p < line.size() && isspace((unsigned char)line[p]))
                p++;
            if (p < line.size() && line[p] == '(')
            {
                while (p < line.size() && line[p] == '(')
                {
                    DefList cg, wg;
                    parse_define_body("xCharList", take_group(line, p), cg);
                    while (p < line.size() && isspace((unsigned char)line[p]))
                        p++;
                    if (p >= line.size() || line[p] != '(')
                        break;
                    parse_define_body("xWidthList", take_group(line, p), wg);
                    for (size_t k = 0; k < cg.strs.size() && k < wg.ints.size(); k++)
                        if (cg.strs[k].size() == 1)
                            inline_w[cg.strs[k]] = wg.ints[k];
                    while (p < line.size() && isspace((unsigned char)line[p]))
                        p++;
                }
            }
            else if (tok.size() >= 4 && defs.count(tok[2]) && defs.count(tok[3]))
            {
                const DefList &cl = defs[tok[2]], &wl = defs[tok[3]];
                size_t n = std::min(cl.strs.size(), wl.ints.size());
                L.glyphs.resize(std::max(L.glyphs.size(), cl.strs.size()));
                for (size_t k = 0; k < n; k++)
                {
                    L.glyphs[k].width = wl.ints[k];
                    if (cl.strs[k].size() == 1)
                        L.index[(uint8_t)cl.strs[k][0]] = (int)k;
                }
            }
        }
        else if (cmd == "LayerSetImageMap" && tok.size() >= 4)
        {
            if (!defs.count(tok[2]) || !defs.count(tok[3]))
                continue;
            const DefList &cl = defs[tok[2]], &rl = defs[tok[3]];
            L.glyphs.resize(std::max(L.glyphs.size(), cl.strs.size()));
            for (size_t k = 0; k < cl.strs.size() && k < rl.rects4.size(); k++)
            {
                const auto &r = rl.rects4[k];
                L.glyphs[k].rect = {r[0], r[1], r[2], r[3]};
                L.glyphs[k].ink_top = 0;
                L.glyphs[k].ink_bot = r[3];
                if (cl.strs[k].size() == 1)
                    L.index[(uint8_t)cl.strs[k][0]] = (int)k;
            }
        }
        else if (cmd == "LayerSetCharOffsets" && tok.size() >= 4)
        {
            if (!defs.count(tok[2]) || !defs.count(tok[3]))
                continue;
            const DefList &cl = defs[tok[2]], &ol = defs[tok[3]];
            for (size_t k = 0; k < cl.strs.size() && k < ol.pairs2.size() && k < L.glyphs.size(); k++)
            {
                L.glyphs[k].ox = ol.pairs2[k][0];
                L.glyphs[k].oy = ol.pairs2[k][1];
            }
        }
        else if (cmd == "LayerSetKerningPairs" && tok.size() >= 4)
        {
            if (!defs.count(tok[2]) || !defs.count(tok[3]))
                continue;
            const DefList &kl = defs[tok[2]], &vl = defs[tok[3]];
            for (size_t k = 0; k < kl.strs.size() && k < vl.ints.size(); k++)
                if (kl.strs[k].size() == 2)
                    L.kern[((uint32_t)(uint8_t)kl.strs[k][0] << 8) | (uint8_t)kl.strs[k][1]] = vl.ints[k];
        }
    }
    for (const auto &kv : inline_w)
        if (kv.first.size() == 1 && kv.first[0] == ' ')
            mSpaceW = kv.second;

    if (mLayers.empty() || mLayers[0].glyphs.empty())
    {
        fprintf(stderr, "atlas_font: %s parsed to no glyphs\n", path.c_str());
        return;
    }
    for (const Layer &L : mLayers)
        for (const Glyph &g : L.glyphs)
            mCellH = std::max(mCellH, g.rect.h);
    // Per-glyph ink bounds come from the layer PNG alpha at upload time
    // (ensure_textures); rect bounds are the fallback before/without it.
    mOk = true;
}

void AtlasFont::ensure_textures(Window &win) const
{
    // const: only lazy-upload state changes (layers hold it as mutable).
    for (Layer &L : const_cast<std::vector<Layer> &>(mLayers))
    {
        if (L.tex_tried)
            continue;
        L.tex_tried = true;
        if (L.image.empty())
            continue;
        const std::string png = mDir + "/" + L.image + ".png";
        SDL_Surface *surf = IMG_Load(png.c_str());
        if (surf == nullptr)
        {
            fprintf(stderr, "atlas_font: cannot load %s: %s\n", png.c_str(), IMG_GetError());
            continue;
        }
        // Scan per-glyph ink rows (any alpha) for ink-accurate centering.
        SDL_Surface *rgba = surf->format->format == SDL_PIXELFORMAT_RGBA32
                                ? surf
                                : SDL_ConvertSurfaceFormat(surf, SDL_PIXELFORMAT_RGBA32, 0);
        if (rgba != nullptr)
        {
            for (Glyph &g : L.glyphs)
            {
                int top = g.rect.h, bot = 0;
                for (int y = 0; y < g.rect.h; y++)
                {
                    const uint8_t *row = (const uint8_t *)rgba->pixels +
                                         (size_t)(g.rect.y + y) * rgba->pitch + (size_t)g.rect.x * 4;
                    for (int x = 0; x < g.rect.w; x++)
                        if (row[x * 4 + 3] != 0)
                        {
                            top = std::min(top, y);
                            bot = y + 1;
                            break;
                        }
                }
                if (top < bot)
                {
                    g.ink_top = top;
                    g.ink_bot = bot;
                }
                else
                {
                    g.ink_top = 0;
                    g.ink_bot = 0; // empty glyph (e.g. space-only rect)
                }
            }
            if (rgba != surf)
                SDL_FreeSurface(rgba);
        }
        L.tex = win.sprite_batch().upload(surf);
        if (L.tex.id == 0)
            fprintf(stderr, "atlas_font: upload failed for %s\n", png.c_str());
        SDL_FreeSurface(surf);
    }
}

int AtlasFont::string_width(const std::string &text) const
{
    if (!mOk || mLayers.empty())
        return 0;
    const Layer &L = mLayers[0];
    int x = 0;
    uint8_t prev = 0;
    for (char ch : text)
    {
        uint8_t c = (uint8_t)ch;
        if (c == ' ')
        {
            x += mSpaceW;
            prev = 0;
            continue;
        }
        int gi = L.index[c];
        if (gi < 0)
        {
            x += mSpaceW;
            prev = 0;
            continue;
        }
        if (prev)
        {
            auto it = L.kern.find(((uint32_t)prev << 8) | c);
            if (it != L.kern.end())
                x += it->second;
        }
        x += L.glyphs[gi].width;
        prev = c;
    }
    return x;
}

void AtlasFont::ink_bounds(const std::string &text, float &top, float &bottom) const
{
    top = 0.0f;
    bottom = (float)mCellH;
    if (!mOk)
        return;
    float mn = 1e9f, mx = -1e9f;
    for (char ch : text)
    {
        uint8_t c = (uint8_t)ch;
        for (const Layer &L : mLayers)
        {
            int gi = L.index[c];
            if (gi < 0)
                continue;
            const Glyph &g = L.glyphs[gi];
            if (g.ink_bot <= g.ink_top)
                continue;
            mn = std::min(mn, (float)(g.oy + g.ink_top));
            mx = std::max(mx, (float)(g.oy + g.ink_bot));
        }
    }
    if (mn <= mx)
    {
        top = mn;
        bottom = mx;
    }
}

void AtlasFont::draw(Window &win, const std::string &text, int x, int y,
                     bool centered, float scale, const RGB &tint, int alpha) const
{
    if (!mOk || text.empty())
        return;
    ensure_textures(win);
    const int a = alpha < 0 ? 0 : alpha > 255 ? 255 : alpha;
    if (centered)
        x -= (int)(string_width(text) * scale / 2.0f + 0.5f);
    for (const Layer &L : mLayers)
    {
        if (L.tex.id == 0)
            continue;
        float pen = 0.0f;
        uint8_t prev = 0;
        for (char ch : text)
        {
            uint8_t c = (uint8_t)ch;
            int gi = L.index[c];
            if (gi < 0)
            {
                pen += (float)mSpaceW;
                prev = 0;
                continue;
            }
            if (prev)
            {
                auto it = L.kern.find(((uint32_t)prev << 8) | c);
                if (it != L.kern.end())
                    pen += (float)it->second;
            }
            const Glyph &g = L.glyphs[gi];
            win.sprite_batch().drawQuad(L.tex, (float)g.rect.x, (float)g.rect.y,
                                        (float)g.rect.w, (float)g.rect.h,
                                        x + (pen + (float)g.ox) * scale,
                                        y + (float)g.oy * scale,
                                        g.rect.w * scale, g.rect.h * scale,
                                        0.0f, false, false,
                                        (uint8_t)tint.red, (uint8_t)tint.green,
                                        (uint8_t)tint.blue, (uint8_t)a);
            pen += (float)g.width;
            prev = c;
        }
    }
}

void AtlasFont::draw_centered(Window &win, const std::string &text, int center_x,
                              int center_y, float scale, const RGB &tint, int alpha) const
{
    ensure_textures(win);
    float it = 0.0f, ib = 0.0f;
    ink_bounds(text, it, ib);
    int y = center_y - (int)((it + ib) * 0.5f * scale + 0.5f);
    draw(win, text, center_x, y, true, scale, tint, alpha);
}

const AtlasFont &AtlasFont::get(const std::string &stem)
{
    static std::unordered_map<std::string, std::unique_ptr<AtlasFont>> fonts;
    std::unique_ptr<AtlasFont> &slot = fonts[stem];
    if (!slot)
        slot.reset(new AtlasFont(stem));
    return *slot;
}

const AtlasFont &AtlasFont::greenInset() { return get("DwarvenTodcraft18GreenInset"); }
const AtlasFont &AtlasFont::greenInsetBright() { return get("DwarvenTodcraft18BrightGreenInset"); }
const AtlasFont &AtlasFont::greenInset36() { return get("DwarvenTodcraft36GreenInset"); }
const AtlasFont &AtlasFont::greenInsetBright36() { return get("DwarvenTodcraft36BrightGreenInset"); }
const AtlasFont &AtlasFont::yellow() { return get("DwarvenTodcraft18Yellow"); }
