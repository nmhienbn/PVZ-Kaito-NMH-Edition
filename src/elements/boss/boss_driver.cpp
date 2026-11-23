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
bool g_tried = false, g_ok = false, g_enabled = true;

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
    // Same retail pivot anchor as reanimz::drawSkinSegment's Zomboss body
    // draw (the boss doc's own geom: scale 1.0, no image offset), with the
    // SAME bossDocCompensation deltas the body call passes -- the driver
    // rides the head transform on top of that origin, so sharing the pair
    // keeps the dome glued to the body at every scale mul.
    int ddx, ddy;
    reanimz::bossDocCompensation(scale_mul, ddx, ddy);
    const float s = 1.0f * scale_mul; // retail mScaleZombie of the boss: 1.0
    float ox = (float)x_location + 45.0f - 30.0f * s + (float)ddx;
    float oy = (float)y_location + 132.0f - 120.0f * s + (float)ddy;
    reanim::DrawStats st = g_renderer->Draw(
        g_driver_doc, dlocal, ox, oy, s,
        [](const std::string &n) { return kHide.count(n) > 0; }, opt);
    return st.partsDrawn;
}

} // namespace bossdriver
