/*
Conveyor-belt seed dispenser (conveyor & bowling levels):
a horizontal strip along the top of the screen, left of the turbo/menu icons.
New packets slide in from the right at a rate inversely proportional to how
many packets are queued.

The queue holds up to BELT_CAP = 10 packets (repvz SEEDBANK_MAX,
GameConstants.h:21); to fit ten slots into the 91..945 px strip of the clone's
1050-px window the belt packets are scaled down from the bank's
(they keep the original packet art's 277:174 aspect).

Pick semantics mirror the original SeedPacket::MouseDown + Deactivate:
clicking a packet lifts it but the slot stays, drawn darkened (held); clicking
the dark slot (or pressing its number key) puts the packet back. Only
planting/throwing consumes it.
*/
#pragma once
#include "elements/plants/plant_type.hpp"
#include <vector>
using namespace std;

// Belt strip geometry (clone window 1050x600). The strip ends 10 px left of
// TURBO_ICON so the last packet can never sit under (or flush against) the
// turbo/menu icons, and clicks in the icon corner are not belt clicks.
// Packets are scaled so the full SEEDBANK_MAX (10, repvz GameConstants.h:21)
// queue fits the strip.
extern const int BELT_X1;       // left edge of the first packet slot
extern const int BELT_Y1;       // packet row top
extern const int BELT_XR;       // right edge of the interactive strip
extern const int BELT_PACKET_W; // belt packet size (scaled to fit the cap)
extern const int BELT_PACKET_H;
extern const int BELT_STEP;     // spacing between settled slots
extern const int BELT_CAP;      // max queued packets (== 10, SEEDBANK_MAX)

// Plant-portrait box inside a belt packet: (x + BELT_ICON_XOFF,
// y + BELT_ICON_YOFF, BELT_PACKET_W - 2*xoff, BELT_PACKET_H - 2*yoff).
extern const int BELT_ICON_XOFF, BELT_ICON_YOFF;

class ConveyorBelt
{
public:
    bool active = false;
    bool bowling = false; // true: picked packets launch rolling nuts instead of planting

    void init(const vector<pair<int, int>> &seeds, bool _bowling,
              int first_packet_clk = 120, float speed_mul = 1.0f);
    void reset();
    void update();  // slide entries, maybe spawn a new packet
    void display(); // render queued packets (the held one follows the cursor)

    bool try_pick(const int &mouse_x, const int &mouse_y); // click a packet (or its dark slot to put it back)
    void cancel_pick();                                    // put the picked packet back
    int take_pick();                                       // consume it, returns plant type
    void push(const int &plant_type);                      // drop a packet on the belt at once
    void toggle_slot(int slot);                            // 1-9 keys: pick slot / put it back

    int packet_count() const; // queued packets (held ones count in, they still occupy their slot)
    int packet_x(const int &i) const;
    bool is_held(const int &i) const;

private:
    struct Entry
    {
        int plant;
        int x;
        bool held; // lifted into the cursor; the dark slot keeps its place
    };
    vector<pair<int, int>> weights;
    vector<Entry> entries;
    int pick = -1; // index into entries, -1: nothing picked
    int spawn_clk = 0;
    int last_rolled = -1;    // repvz Challenge::mLastConveyorSeedType
    float speed_mul = 1.0f;  // repvz aConveyorSpeedMultiplier (boss 0.875, ...)

    int roll_plant();
    int spawn_interval() const;
    int slide_in_x(const int &index) const; // spawn x of the next packet (repvz AddSeed offset)
};

extern ConveyorBelt belt;
