#include "boss_driver.hpp"

#include "draw/rsdl.hpp"
#include "reanim/reanim.hpp"
#include "reanim/reanim_sdl.hpp"
#include "reanim/reanim_zombie.hpp" // zombieTimeMul(): match drawSkinSegment's clock

#include <algorithm>
#include <cmath>
#include <string>
#include <unordered_set>

extern Window win;

namespace bossdriver {
namespace {

reanim::SdlReanimRenderer *g_renderer = nullptr;
reanim::ReanimDoc g_boss_doc;   // only for the Boss_head anchor transform
reanim::ReanimDoc g_driver_doc; // the dome driver
const reanim::Track *g_head_track = nullptr;
float g_anchor_dx = 0.0f, g_anchor_dy = 0.0f; // zomboss doc-space anchor
bool g_tried = false, g_ok = false, g_enabled = true;

// Values mirrored from reanim_zombie.cpp's initSkin calibration (fixed
// ZOMBIE_SCALE / MASS_CENTER_X / ART_BOTTOM_Y): the driver overlay must ride
// on the exact same doc-space anchor drawSkinSegment computed for the
// Zomboss body, or it drifts off the dome.
const float ZOMBIE_SCALE = 1.01f;
const float MASS_CENTER_X = 61.0f;
const float ART_BOTTOM_Y = 136.0f;

reanim::DrawStats measure(reanim::ReanimDoc &doc, int first, int last,
                          bool feet_only)
{
    reanim::DrawOptions m;
    m.measureOnly = true;
    reanim::DrawStats acc;
    for (int fr = first; fr <= last; fr += 4)
    {
        reanim::DrawStats st = g_renderer->Draw(
            doc, (float)fr, 0, 0, 1.0f,
            [&](const std::string &n) {
                if (!feet_only)
                    return false;
                std::string l = n;
                for (char &c : l)
                    c = (char)tolower((unsigned char)c);
                return l.find("foot") == std::string::npos &&
                       l.find("toe") == std::string::npos;
            },
            m);
        if (!st.hasBounds)
            continue;
        if (!acc.hasBounds)
            acc = st;
        else
        {
            acc.minX = std::min(acc.minX, st.minX);
            acc.minY = std::min(acc.minY, st.minY);
            acc.maxX = std::max(acc.maxX, st.maxX);
            acc.maxY = std::max(acc.maxY, st.maxY);
        }
    }
    return acc;
}

bool init()
{
    static reanim::SdlReanimRenderer renderer(win.sprite_renderer());
    g_renderer = &renderer;
    if (!renderer.BindDirectory("./resources/reanim"))
        return false;
    std::string err;
    if (!reanim::LoadReanimXml("./resources/reanim/Zombie_boss.reanim",
                               g_boss_doc, err) ||
        !reanim::LoadReanimXml("./resources/reanim/Zombie_Boss_driver.reanim",
                               g_driver_doc, err))
    {
        SDL_Log("boss driver: %s", err.c_str());
        return false;
    }
    for (const reanim::Track &tr : g_boss_doc.tracks)
    {
        std::string l = tr.name;
        for (char &c : l)
            c = (char)tolower((unsigned char)c);
        if (l == "boss_head")
            g_head_track = &tr;
    }
    if (g_head_track == nullptr)
    {
        SDL_Log("boss driver: Zombie_boss.reanim has no Boss_head track");
        return false;
    }
    auto it = g_boss_doc.segments.find("anim_idle");
    if (it == g_boss_doc.segments.end())
        return false;
    reanim::DrawStats acc = measure(g_boss_doc, it->second.first,
                                    it->second.last, false);
    reanim::DrawStats feet = measure(g_boss_doc, it->second.first,
                                     it->second.last, true);
    if (!acc.hasBounds)
        return false;
    float floor_u = feet.hasBounds ? feet.maxY : acc.maxY;
    g_anchor_dx = MASS_CENTER_X - ZOMBIE_SCALE * (acc.minX + acc.maxX) * 0.5f;
    g_anchor_dy = ART_BOTTOM_Y - ZOMBIE_SCALE * floor_u;
    return true;
}

} // namespace

void set_enabled(bool on) { g_enabled = on; }
bool enabled() { return g_enabled; }

bool ready()
{
    if (!g_tried)
    {
        g_tried = true;
        g_ok = init();
    }
    return g_ok;
}

int draw(const char *boss_seg, float boss_anim_t, bool boss_loop,
         int x_location, int y_location, float scale_mul,
         bool cold, bool attacked)
{
    if (!g_enabled || !ready())
        return 0;

    // Boss frame resolution, replicated from reanimz::drawSkinSegment so the
    // sampled head transform matches the body visible in this frame.
    auto segIt = g_boss_doc.segments.find(boss_seg);
    if (segIt == g_boss_doc.segments.end())
        return 0;
    int first = segIt->second.first, last = segIt->second.last;
    float len = (float)(last - first + 1);
    float local = boss_anim_t < 0 ? 0 : boss_anim_t;
    local *= reanimz::zombieTimeMul();
    if (boss_loop)
        local = fmodf(local, len);
    else if (local > len - 1)
        local = len - 1;
    float abs_frame = (float)first + local;
    reanim::Transform head =
        reanim::SampleTrack(*g_head_track, abs_frame, first, last);

    // The driver doc's anim_* labels all span the whole dump (flattened
    // conversion) -- the doc itself is one continuous bobbing loop.
    int n = g_driver_doc.frameCount;
    float dlocal = fmodf(boss_anim_t * reanimz::zombieTimeMul(),
                         (float)(n > 1 ? n - 1 : 1));
    static const std::unordered_set<std::string> kHide = {"flag", "flagpole"};
    reanim::DrawOptions opt;
    opt.tint = cold ? SDL_Color{110, 170, 255, 255} : SDL_Color{255, 255, 255, 255};
    opt.flash = attacked;
    opt.loopWrapFirst = 0;
    opt.loopWrapLast = n - 1;
    opt.attachFrom = &head;
    float ox = (float)x_location + g_anchor_dx * scale_mul;
    float oy = (float)y_location + g_anchor_dy * scale_mul;
    reanim::DrawStats st = g_renderer->Draw(
        g_driver_doc, dlocal, ox, oy, ZOMBIE_SCALE * scale_mul,
        [](const std::string &n) { return kHide.count(n) > 0; }, opt);
    return st.partsDrawn;
}

} // namespace bossdriver
