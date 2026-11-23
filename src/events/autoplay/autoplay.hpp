#pragma once

/*
Headless autoplay/simulation harness.

Enabled with PVZ_AUTOPLAY=<scenario> and PVZ_AUTOPLAY_OUT=<dir>. The requested
scenario is built on the normal day front-yard level, the real per-tick
gameplay logic is run as fast as possible (no frame pacing), and the run is
recorded to <dir>/<scenario>/{track.csv,events.csv,summary.txt} plus PNG
screenshots for the scenarios that ask for them.

Normal gameplay is untouched when the env var is unset: run_if_requested()
returns false immediately.
*/
namespace autoplay
{
/*
@return true when an autoplay run was requested and has completed (the caller
should exit 0), false when PVZ_AUTOPLAY is unset.
*/
bool run_if_requested();
} // namespace autoplay
