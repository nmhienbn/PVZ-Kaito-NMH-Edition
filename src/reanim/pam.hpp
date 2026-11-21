#pragma once

// Loader + timeline evaluator for PvZ2 "PAM" animations, decoded to the
// PopStudio pam.json schema (see tools/Pvz2AssetTool: pam2json).
//
// A PAM file holds shared images, sub-sprites, and a main_sprite timeline.
// Each sprite is a list of frames; a frame only stores display-list ops:
//   append: claim a slot (index) with an image/sprite reference
//   remove: free a slot
//   change: update transform/color/anim_frame_num of the element in a slot
// Slots are sparse ids (0, 4, 8, ...); draw order is ascending slot.
// A slot's transform/color carry over between frames until changed
// (PopCap defaults: identity transform, color [1,1,1,1]).
//
// Transform semantics (verified against PopStudio's PamXfl.cs):
//   len 6 = Flash matrix [a,b,c,d,tx,ty]:  X = a*u + c*v + tx ; Y = b*u + d*v + ty
//   len 2 = pure translate [tx,ty]
//   len 3 = [angleRadians, tx, ty]: a=d=cos, b=sin, c=-sin
// Image draw: local quad (0,0)-(w,h) mapped by image.origin, then the
// display-list element transform, then ancestors: world = parent * child.
//
// Timeline evaluation: sprite at frame f starts from an empty display list
// and applies frames 0..f in order. Fractional f interpolates each live
// slot's transform/color between floor(f) and floor(f)+1 (appended/removed
// slots still change exactly at the frame boundary). A sprite element keeps
// its own local frame: preload_frames at append (or anim_frame_num when a
// change sets one;
// the PAM binary never stores 0, so 0 means "not set"), advanced by the
// element's timescale for each frame the parent advances. The local frame
// loops inside [work_area[0], frames.size()-1]: PopStudio stomps the
// duration field to the sprite's total frame count on decode, so the real
// loop length is frames after the start.
//
// Frame labels on the main sprite define named segments: label L starts at
// its frame and runs to the frame before the next label (or the last frame).
// stop/command markers do nothing for playback; they are counted at load.

#include <map>
#include <string>
#include <vector>

namespace pam {

struct Mat { // [a b c d tx ty], X = a*u + c*v + tx; Y = b*u + d*v + ty
    float a = 1.0f, b = 0.0f, c = 0.0f, d = 1.0f, tx = 0.0f, ty = 0.0f;
};

struct ColorMul {
    float r = 1.0f, g = 1.0f, b = 1.0f, a = 1.0f;
};

struct ImageDef {
    std::string name;       // display part of "display|RESOURCE_ID"
    std::string resourceId; // atlas-map key (after '|'; whole name if no '|')
    float w = 0, h = 0;     // display size in PAM units
    Mat origin;             // image-local pixels -> element space
};

struct AppendOp {
    int index = 0;        // display-list slot
    int resource = 0;     // index into PamDoc::sprites or ::images
    bool sprite = false;  // true: sprites, false: images
    int preloadFrames = 0;
    float timescale = 1.0f;
};

struct ChangeOp {
    int index = 0;
    bool hasTransform = false;
    Mat transform;
    bool hasColor = false;
    ColorMul color;
    int animFrameNum = 0; // != 0: set the child sprite's local frame
};

struct FrameOps {
    bool hasLabel = false;
    std::string label;
    bool stop = false;
    int commandCount = 0;
    std::vector<int> removes;
    std::vector<AppendOp> appends;
    std::vector<ChangeOp> changes;
};

struct SpriteDef {
    std::string name;
    std::vector<FrameOps> frames;
    int workStart = 0; // loop window: [workStart, frames.size()-1]
};

struct Segment {
    int first = 0, last = 0;
};

struct PamDoc {
    int version = 0;
    float frameRate = 30.0f;
    float originX = 0, originY = 0; // "position" field
    float sizeW = 0, sizeH = 0;     // "size" field
    std::vector<ImageDef> images;
    std::vector<SpriteDef> sprites;
    SpriteDef mainSprite;
    std::map<std::string, Segment> segments; // main-sprite labels
    int stopCount = 0;                       // ignored markers, for logging
    int commandCount = 0;
};

bool LoadPamJson(const std::string& path, PamDoc& out, std::string& err);

Mat Multiply(const Mat& parent, const Mat& child);
ColorMul Multiply(const ColorMul& parent, const ColorMul& child);

struct DrawPart {
    int image = 0; // index into PamDoc::images
    Mat world;     // image-local pixels -> PAM space
    ColorMul color;
};

// Resolves a sprite timeline into a flat list of image draws.
class PamPlayer {
public:
    explicit PamPlayer(const PamDoc& d) : doc(d) {}

    bool SegmentRange(const std::string& name, Segment& out) const;

    // Main sprite, `frame` absolute (0..frames-1; fractional frames
    // interpolate between the two surrounding keyframes).
    void EvalMain(float frame, std::vector<DrawPart>& out) const;
    // Main sprite at `seg.first + offset` (offset wraps inside the segment).
    void EvalSegment(const Segment& seg, float offset, std::vector<DrawPart>& out) const;
    // spriteIndex = -1 for the main sprite. External matrix/color (scene
    // placement) can be premultiplied in via parent/parentColor.
    void EvalSprite(int spriteIndex, float frame, const Mat& parent,
                    const ColorMul& parentColor, std::vector<DrawPart>& out,
                    int depth = 0) const;

private:
    const PamDoc& doc;
};

} // namespace pam
