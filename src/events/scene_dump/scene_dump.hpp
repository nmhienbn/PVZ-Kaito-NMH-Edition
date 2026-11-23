#pragma once

/*
PVZ_SCENE_DUMP debug scene dumper (see scene_dump.cpp).

When the env var PVZ_SCENE_DUMP=<out.png> is set, main() calls
scene_dump::run_if_requested() once, which builds a fixed day front-yard
scene through the normal game entity factories/renderers, draws a debug
overlay, saves the frame as a PNG (+ a .txt entity report) and returns true
so main exits with 0. Unset: nothing in this module runs and every game path
is byte-identical to before.
*/
namespace scene_dump
{
// True only while one entity's display call is being measured; the quad hook
// in GlSpriteBatch::pushQuad records into the current bounding box then.
// Always false during normal play.
extern bool g_capture_active;

// Begin/end one measured draw call. capture_begin() resets the box.
void capture_begin();
void capture_end();

// GlSpriteBatch::pushQuad hook: union one quad's four corners into the box
// while g_capture_active (no-op otherwise).
void capture_quad(const float xs[4], const float ys[4]);

// Entry point (see the module comment). Returns true when a dump ran.
bool run_if_requested();
} // namespace scene_dump
