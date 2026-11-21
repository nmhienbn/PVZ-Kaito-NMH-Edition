#pragma once
// Fog overlay for the night-pool (area 4) fog missions + Plantern / Blover
// interaction, modeled on PvZ1's Board::UpdateFog / DrawFog /
// ClearFogAroundPlant. State lives here (not in Level) so the other area
// agents do not share a hot file with the fog render.
//
//   fogov::blow_away(ticks)  Blover hook: fog is gone for <ticks> game ticks.
//   fogov::is_tile_lit(r, c) true when a Plantern clears the fog on that tile
//                            (for the fog-zombies / visibility agents).
//   fogov::display()         render hook: call AFTER zombies, BEFORE suns/UI.
namespace fogov
{
// Blover clears the fog this long; it then drifts back in (fade).
extern const int FOG_BLOWN_TICKS;   // ~20 s
extern const int FOG_FADE_IN_TICKS; // last ticks of the blow, fog drifts back
// Plantern clears fog in a ~2.3-tile radius around its tile center (px).
extern const int PLANTERN_LIT_RADIUS;

void blow_away(int ticks);
bool is_blown();                    // fog schedules fully hidden right now
bool is_tile_lit(int row, int col); // inside any Plantern's light bubble
void update();                      // one game tick (paused = skip)
void display();                     // draws the fog layer (calls update())

// Debug/verification hook: snapshot the fog grid (tiles with alpha > 0,
// min/max alpha over the fog columns, current horizontal offset in px).
void debug_stats(int &tiles, int &alpha_min, int &alpha_max, int &offset);

// Debug/verification hook: the coverage alpha the layer composer writes at a
// screen point (0 outside any fog cel). Bilinear-smooth by construction, so
// checks can assert no hard jumps across column/row borders.
int debug_coverage_alpha(int x, int y);
} // namespace fogov
