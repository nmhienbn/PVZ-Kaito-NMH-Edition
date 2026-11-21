#pragma once

// Loader + sampler for PopCap "reanim" animation files (decompiled XML form,
// as found e.g. in the PvZ1 resource dumps).
//
// A reanim file is a list of tracks (one per body part); each track holds a
// per-frame transform. Only fields that change are stored per frame, so the
// loader resolves them into a fully-keyed state per frame (values carry over
// from the previous frame; PopCap defaults: x=y=0, sx=sy=1, kx=ky=0, a=1,
// f=0, no image).
//
// Transform semantics (verified against PopStudio's reanim<->XFL converter):
//   a = cos(kx) * sx ; b = sin(kx) * sx
//   c = -sin(ky) * sy ; d = cos(ky) * sy    (kx, ky in degrees)
// with Flash convention:  X = a*u + c*v + x ; Y = b*u + d*v + y
// mapping image-local pixel coords (u,v) into reanim space. kx == ky is plain
// rotation, differing kx/ky gives shear. f == -1 hides the part.

#include <string>
#include <vector>
#include <map>

namespace reanim {

struct Transform {
    float x = 0.0f, y = 0.0f;
    float sx = 1.0f, sy = 1.0f;
    float kx = 0.0f, ky = 0.0f; // skew angles, degrees
    float a = 1.0f;             // alpha multiplier 0..1
    float f = 0.0f;             // -1 = hidden this frame
    int   image = -1;           // index into ReanimDoc::images, -1 = no image
};

struct Track {
    std::string name;
    std::vector<Transform> frames; // fully resolved per-frame states
    bool hasAnyImage = false;
};

struct Segment {
    int first = 0, last = 0;
};

struct ReanimDoc {
    float fps = 12.0f;
    int frameCount = 0;
    std::vector<Track> tracks;
    std::vector<std::string> images;                 // raw ids, e.g. IMAGE_REANIM_ZOMBIE_BODY
    std::map<std::string, Segment> segments;         // "anim_walk" -> frame range
};

bool LoadReanimXml(const std::string& path, ReanimDoc& out, std::string& err);

// Linear interpolation of numeric fields between floor(frame)/+1;
// f and image are stepped (taken from the lower frame).
// When loopFirst/loopLast bound a looping segment, interpolation past
// loopLast wraps back to loopFirst instead of bleeding into the next
// segment (whose first pose would distort the loop's last frames).
// loopBlend widens the seam ease from its default single frame to a
// cross-blend of the window's last loopBlend frames into the first pose,
// for 2Pak takes whose authored ends do not meet (unclosed walk/eat loops).
Transform SampleTrack(const Track& track, float frame,
                      int loopFirst = -1, int loopLast = -1,
                      float loopBlend = 1.0f);

// True when frame `last` duplicates frame `first` on every part track
// (2Pak exports close each loop with a copy of its first pose). A loop like
// that should play over [first, last): showing `last` as a full frame doubles
// the first pose at every wrap (visible as a "khựng"/hitch). The duplicate
// frame is still reachable as the wrap interpolation target in SampleTrack.
bool SegmentEndsOnLoopSeam(const ReanimDoc& doc, int first, int last);

// "IMAGE_REANIM_ZOMBIE_BODY" -> "zombie_body" (lowercase, prefix stripped),
// used as lookup key for the part image files.
std::string ImageFileStem(const std::string& imageId);

} // namespace reanim
