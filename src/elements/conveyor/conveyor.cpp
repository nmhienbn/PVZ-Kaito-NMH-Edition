#include "conveyor.hpp"
#include "draw/rsdl.hpp"
#include "elements/Map/Map.hpp"
#include "elements/SeedPacket/SeedPacket.hpp"
#include "elements/button/button.hpp"
#include "elements/elements.hpp"
#include "elements/plants/plants.hpp"
#include "game_param.hpp"
#include "music/music.hpp"
#include "support.hpp"

extern Window win;
extern Elements game_characters;
extern Map cells;

ConveyorBelt belt;

// Belt strip geometry (repvz SeedBank conveyor layout: packet slot i sits at
// mX = i * 50 + 91 at mY = 8 in the original 800px window, up to
// SEEDBANK_MAX = 10 slots — Board::GetSeedPacketPositionX, SeedPacket.cpp
// Draw, GameConstants.h:21). The clone window is 1050 wide with the turbo/
// menu icons at 955+, so the 10 slots are scaled down to fit 91..945,
// leaving a 10-px breathing gap to TURBO_ICON ("belt sits right against the
// buttons" report). Literal 945: a linked-time const would race TURBO_ICON's
// own init order.
const int BELT_X1 = 91; // same packet base as the original
const int BELT_Y1 = 8;  // repvz aPacket->mY = 8
const int BELT_XR = 945;
const int BELT_CAP = 10; // repvz SEEDBANK_MAX (GameConstants.h:21)
const int BELT_PACKET_W = (BELT_XR - BELT_X1) / BELT_CAP;   // 85
const int BELT_PACKET_H = BELT_PACKET_W * 174 / 277;        // keep the packet art's aspect: 53
const int BELT_STEP = BELT_PACKET_W; // original: adjacent (step == width)

// Plant-portrait box inside a belt packet (see SeedPacket.hpp's bank box):
// the belt has no sun tag/price, so only a pure margin.
const int BELT_ICON_XOFF = 8, BELT_ICON_YOFF = 2;

const int BELT_SPEED = 1; // px/tick sliding left

void ConveyorBelt::init(const vector<pair<int, int>> &seeds, bool _bowling,
                        int first_packet_clk, float _speed_mul)
{
    active = true;
    bowling = _bowling;
    weights = seeds;
    entries.clear();
    pick = -1;
    last_rolled = -1;
    spawn_clk = first_packet_clk;
    speed_mul = _speed_mul;
}

void ConveyorBelt::reset()
{
    active = false;
    bowling = false;
    weights.clear();
    entries.clear();
    pick = -1;
    last_rolled = -1;
    speed_mul = 1.0f;
}

/*repvz TodAnimateCurve(0, full, total, weight, 1, CURVE_LINEAR): a conveyor
seed fades from its base weight down to 1 as `total` of its kind pile up.*/
static int weight_fade(int weight, int total, int full)
{
    if (total > full)
        total = full;
    float w = weight + (1.0f - weight) * total / full;
    return max(1, (int)w);
}

/*Weighted plant roll with the original's belt damping, repvz
Challenge::UpdateConveyorBelt (Challenge.cpp:1862-1926):
  + gravebuster: weight 0 once the lawn has no graves left to chew;
  + lilypad / flowerpot: linear fade of the weight over the count of that
    type already on the lawn + queued on the belt (curve depth 18 / 35);
  + belts offering > 2 seed types: a type with 3 packets queued drops to
    weight 5, with 4+ to weight 1, and a repeat of the previous roll halves
    its weight;
  + the rolled type is remembered as the next roll's "previous".*/
int ConveyorBelt::roll_plant()
{
    // Counts this roll needs (repvz CountOfTypeOnConveyorBelt /
    // CountPlantByType / GetGraveStonesCount).
    auto on_belt = [&](int type) {
        int n = 0;
        for (const auto &e : entries)
            n += e.plant == type;
        return n;
    };
    auto planted = [&](int type) {
        int n = 0;
        for (const auto &p : game_characters.plants)
            n += p->get_type() == type;
        return n;
    };
    auto graves = [&]() {
        int n = 0;
        for (const auto &row : cells)
            for (const auto &block : row)
                n += block.has_grave;
        return n;
    };

    vector<int> adj(weights.size());
    int total = 0;
    for (int i = 0; i < (int)weights.size(); i++)
    {
        int type = weights[i].first;
        int w = weights[i].second;
        int in_bank = on_belt(type);
        int all = planted(type) + in_bank;
        if (type == GRAVEBUSTER_TYPE)
        {
            w = 0; // no fresh gravebuster once nothing is left to bust
            if (graves() > all)
                w = weights[i].second;
        }
        else if (type == LILYPAD_TYPE)
            w = weight_fade(w, all, 18);
        else if (type == FLOWERPOT_TYPE)
            w = weight_fade(w, all, 35);

        if ((int)weights.size() > 2)
        {
            if (in_bank >= 4)
                w = 1;
            else if (in_bank >= 3)
                w = 5;
            else if (type == last_rolled)
                w /= 2;
        }
        adj[i] = w;
        total += w;
    }
    if (total <= 0) // e.g. every damped curve bottomed out: fall back raw
        for (int i = 0; i < (int)weights.size(); i++)
            total += adj[i] = weights[i].second;

    int ticket = rand(0, total - 1);
    int idx = (int)weights.size() - 1;
    for (int i = 0; i < (int)weights.size(); i++)
        if ((ticket -= adj[i]) < 0)
        {
            idx = i;
            break;
        }
    int type = weights[idx].first;
    last_rolled = type;
    return type;
}

/*Ticks until the next packet (GOTY Challenge::UpdateConveyorBelt, converted
from its 100 Hz counters to 60 fps ticks), scaled by the level's conveyor
speed multiplier: <=4 queued 400->240, 5-6 425->255, 7-8 500->300, >8
1000->600 (Challenge.cpp:1653-1654), times aConveyorSpeedMultiplier (boss
x0.875, shovel/portal x1.5, invisighoul x2, column x3 — only boss exists in
this clone's special set).*/
int ConveyorBelt::spawn_interval() const
{
    int queued = (int)entries.size();
    int base;
    if (queued > 8)
        base = 600;
    else if (queued > 6)
        base = 300;
    else if (queued > 4)
        base = 255;
    else
        base = 240;
    return (int)(base * speed_mul + 0.5f);
}

/*Where the next packet enters the strip. repvz SeedBank::AddSeed:
mOffsetX = 515 - (SEED_PACKET_WIDTH + 1) * aNumSeeds, clamped to
prev offset + 40, then UpdateConveyorBelt slides offsets to 0. Scaled by the
packet width, and capped so the slide-in never hides under the icons.*/
int ConveyorBelt::slide_in_x(const int &index) const
{
    int x = BELT_X1 + 515 * BELT_PACKET_W / 50 - (BELT_PACKET_W + 1) * index;
    x = min(x, BELT_XR - BELT_PACKET_W);
    if (!entries.empty())
        x = max(x, entries.back().x + 40 * BELT_PACKET_W / 50);
    return max(x, BELT_X1 + index * BELT_STEP);
}

void ConveyorBelt::update()
{
    if (!active)
        return;
    // Slide left; entries stack against the left edge.
    for (int i = 0; i < (int)entries.size(); i++)
    {
        int limit = BELT_X1 + i * BELT_STEP;
        if (entries[i].x > limit)
            entries[i].x = max(limit, entries[i].x - BELT_SPEED);
    }
    // Dispense from the right until the strip is capped.
    if ((int)entries.size() < BELT_CAP && --spawn_clk <= 0)
    {
        int index = (int)entries.size();
        entries.push_back({roll_plant(), slide_in_x(index), false});
        spawn_clk = spawn_interval();
    }
}

void ConveyorBelt::display()
{
    if (!active)
        return;
    int _x, _y;
    SDL_GetMouseState(&_x, &_y);
    for (int i = 0; i < (int)entries.size(); i++)
    {
        int x = entries[i].x, y = BELT_Y1;
        // Same anatomy as a bank packet (SeedPacket::display) minus the sun
        // tag + cost: conveyor packets are free (repvz SeedPacket::Draw skips
        // the cost via aDrawCost = false).
        win.draw_png_height_scaled(SEED_PACKET_DIRECTORY, x, y, BELT_PACKET_W);
        if (bowling)
        {
            // Bowling belt shows the actual rolling nut, not a plant portrait.
            int nut_img = entries[i].plant == CHERRYBOMB_TYPE ? EXPLODEONUT_BOWLING_DIRECTORY : WALLNUT_BOWLING_DIRECTORY;
            win.draw_png_width_scaled(nut_img, x + 18, y + 4, BELT_PACKET_H - 8);
        }
        else
        {
            // Same world-height sizing as the bank packet
            // (SeedPacket::display), scaled by the belt packet's shrink
            // (85px vs the bank's 105) so belt and bank read identically; the
            // belt has no sun tag, so the art box spans the full width.
            draw_plant_icon_world(entries[i].plant,
                                  x + BELT_PACKET_W / 2,
                                  y + BELT_PACKET_H - BELT_ICON_YOFF,
                                  BELT_PACKET_W - 2 * BELT_ICON_XOFF,
                                  BELT_PACKET_H - 2 * BELT_ICON_YOFF,
                                  BELT_PACKET_W / (float)ICON_WIDTH);
        }
        win.draw_png_height_scaled(SEED_PACKET_BOT_BORDER_DIRECTORY, x, y + BELT_PACKET_H - 5, BELT_PACKET_W);
        // Slot hotkey, like the 1-9(0) labels on the bank column.
        if (i < 10)
            win.show_text_outlined(to_string((i + 1) % 10), x + BELT_PACKET_W - 13, y, WHITE, BRIANNE_TTF);
        if (entries[i].held)
        {
            // Held in the cursor: the slot stays, darkened (repvz
            // SeedPacket::Draw aPercentDark = 1.0f when deactivated).
            win.draw_png(BLACK_SCREEN_DIRECTORY, x, y, BELT_PACKET_W, BELT_PACKET_H);
        }
        else if (is_in(x, _x, x + BELT_PACKET_W) && is_in(y, _y, y + BELT_PACKET_H))
        {
            // Hover blink
            win.draw_png(WHITE_SCREEN_DIRECTORY, x, y, BELT_PACKET_W, BELT_PACKET_H);
        }
    }
}

/*Click on a queued packet: lift it into the cursor; the slot stays, darkened.
Clicking the dark slot puts the packet back. Picking another packet swaps.*/
bool ConveyorBelt::try_pick(const int &mouse_x, const int &mouse_y)
{
    // The turbo/menu icons own their corner of the top bar.
    if (TURBO_ICON.is_mouse_in(mouse_x, mouse_y) ||
        PAUSE_ICON.is_mouse_in(mouse_x, mouse_y))
        return false;
    if (!is_in(BELT_Y1, mouse_y, BELT_Y1 + BELT_PACKET_H) ||
        !is_in(BELT_X1, mouse_x, BELT_XR))
        return false;
    for (int i = 0; i < (int)entries.size(); i++)
    {
        if (!is_in(entries[i].x, mouse_x, entries[i].x + BELT_PACKET_W))
            continue;
        if (entries[i].held)
        {
            // Put the held packet back on its dark slot.
            cancel_pick();
            return true;
        }
        if (pick != -1)
            entries[pick].held = false;
        pick = i;
        entries[i].held = true;
        SeedPacket::chosen_plant = (PlantType)entries[i].plant;
        play_sound_effect(SEED_LIFT_MUSIC_DIRECTORY);
        return true;
    }
    return true; // clicked on the belt, not on a packet
}

void ConveyorBelt::cancel_pick()
{
    if (pick != -1)
    {
        entries[pick].held = false; // repvz SeedPacket::Activate on cursor drop
        play_sound_effect(BUTTON_CLICK_MUSIC_DIRECTORY);
        pick = -1;
        SeedPacket::chosen_plant = PLANT_COUNT;
    }
}

/*1-9 keys on the belt: pick the packet in this slot; pressing it again (its
slot is dark) puts it back — the same toggle the bank column's hotkeys do.*/
void ConveyorBelt::toggle_slot(int slot)
{
    if (!active || slot < 0 || slot >= (int)entries.size())
        return;
    if (entries[slot].held)
    {
        cancel_pick();
        return;
    }
    if (pick != -1)
        entries[pick].held = false;
    pick = slot;
    entries[slot].held = true;
    SeedPacket::chosen_plant = (PlantType)entries[slot].plant;
    play_sound_effect(SEED_LIFT_MUSIC_DIRECTORY);
}

/*Consume the picked packet; returns its plant type (-1 if none picked).*/
int ConveyorBelt::take_pick()
{
    if (pick == -1)
        return -1;
    int plant = entries[pick].plant;
    entries.erase(entries.begin() + pick); // repvz SeedBank::RemoveSeed
    pick = -1;
    SeedPacket::chosen_plant = PLANT_COUNT;
    return plant;
}

/*Drop one packet onto the belt immediately (Dave hands the player a Wall-nut
when the 1-5 bowling starts).*/
void ConveyorBelt::push(const int &plant_type)
{
    if ((int)entries.size() < BELT_CAP)
    {
        int index = (int)entries.size();
        entries.push_back({plant_type, slide_in_x(index), false});
    }
}

int ConveyorBelt::packet_count() const
{
    return (int)entries.size();
}

int ConveyorBelt::packet_x(const int &i) const
{
    return i >= 0 && i < (int)entries.size() ? entries[i].x : BELT_X1;
}

bool ConveyorBelt::is_held(const int &i) const
{
    return i >= 0 && i < (int)entries.size() && entries[i].held;
}
