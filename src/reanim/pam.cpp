#include "pam.hpp"

#include "nlohmann/json.hpp"

#include <SDL.h>

#include <cmath>
#include <fstream>

namespace pam {

namespace {

Mat matIdentity() { return Mat{}; }

// Variant transform encodings (PopStudio PamXfl.cs): len 2 = translate,
// len 3 = [angleRadians, tx, ty], len 6 = full matrix.
bool normalizeMat(const nlohmann::json& t, Mat& out) {
    if (!t.is_array()) return false;
    size_t n = t.size();
    if (n == 2) {
        out.tx = t[0].get<float>();
        out.ty = t[1].get<float>();
        return true;
    }
    if (n == 3) {
        float ang = t[0].get<float>();
        float cs = cosf(ang), sn = sinf(ang);
        out.a = cs;
        out.b = sn;
        out.c = -sn;
        out.d = cs;
        out.tx = t[1].get<float>();
        out.ty = t[2].get<float>();
        return true;
    }
    if (n == 6) {
        out.a = t[0].get<float>();
        out.b = t[1].get<float>();
        out.c = t[2].get<float>();
        out.d = t[3].get<float>();
        out.tx = t[4].get<float>();
        out.ty = t[5].get<float>();
        return true;
    }
    return false;
}

bool parseColor(const nlohmann::json& c, ColorMul& out, const char* where) {
    if (!c.is_array() || c.size() < 4) {
        SDL_Log("pam: unexpected color shape at %s, skipping", where);
        return false;
    }
    out.r = c[0].get<float>();
    out.g = c[1].get<float>();
    out.b = c[2].get<float>();
    out.a = c[3].get<float>();
    return true;
}

void parseSprite(const nlohmann::json& js, SpriteDef& out, const PamDoc* /*unused*/,
                 int& stopCount, int& commandCount) {
    out.name = js.value("name", "");
    const auto& wa = js.at("work_area");
    if (wa.is_array() && wa.size() >= 1)
        out.workStart = wa[0].get<int>();
    for (const auto& jf : js.at("frame")) {
        FrameOps f;
        if (jf.contains("label") && !jf["label"].is_null()) {
            f.label = jf["label"].get<std::string>();
            f.hasLabel = true;
        }
        f.stop = jf.value("stop", false);
        if (f.stop) stopCount++;
        if (jf.contains("command") && jf["command"].is_array()) {
            f.commandCount = (int)jf["command"].size();
            commandCount += f.commandCount;
        }
        for (const auto& jr : jf.at("remove")) {
            // PopStudio writes {"index": n}; tolerate bare ints.
            f.removes.push_back(jr.is_object() ? jr.at("index").get<int>()
                                               : jr.get<int>());
        }
        for (const auto& ja : jf.at("append")) {
            AppendOp a;
            a.index = ja.at("index").get<int>();
            a.resource = ja.at("resource").get<int>();
            a.sprite = ja.value("sprite", false);
            a.preloadFrames = ja.value("preload_frames", 0);
            a.timescale = ja.value("timescale", 1.0f);
            f.appends.push_back(a);
        }
        for (const auto& jc : jf.at("change")) {
            ChangeOp c;
            c.index = jc.at("index").get<int>();
            if (jc.contains("transform") && !jc["transform"].is_null())
                c.hasTransform = normalizeMat(jc["transform"], c.transform);
            if (jc.contains("color") && !jc["color"].is_null())
                c.hasColor = parseColor(jc["color"], c.color, "frame change");
            c.animFrameNum = jc.value("anim_frame_num", 0);
            // src_rect is never emitted for our atlases; a warning-that-counts
            // is enough if it shows up.
            if (jc.contains("src_rect") && !jc["src_rect"].is_null())
                SDL_Log("pam: src_rect change ignored (slot %d)", c.index);
            f.changes.push_back(c);
        }
        out.frames.push_back(std::move(f));
    }
}

} // namespace

Mat Multiply(const Mat& p, const Mat& c) {
    Mat o;
    o.a = p.a * c.a + p.c * c.b;
    o.b = p.b * c.a + p.d * c.b;
    o.c = p.a * c.c + p.c * c.d;
    o.d = p.b * c.c + p.d * c.d;
    o.tx = p.a * c.tx + p.c * c.ty + p.tx;
    o.ty = p.b * c.tx + p.d * c.ty + p.ty;
    return o;
}

ColorMul Multiply(const ColorMul& p, const ColorMul& c) {
    return ColorMul{p.r * c.r, p.g * c.g, p.b * c.b, p.a * c.a};
}

bool LoadPamJson(const std::string& path, PamDoc& out, std::string& err) {
    std::ifstream in(path);
    if (!in) {
        err = "cannot open " + path;
        return false;
    }
    nlohmann::json j;
    try {
        in >> j;
    } catch (const std::exception& e) {
        err = path + ": " + e.what();
        return false;
    }
    out = PamDoc();
    out.version = j.value("version", 0);
    out.frameRate = j.value("frame_rate", 30.0f);
    if (j.contains("position") && j["position"].is_array() && j["position"].size() >= 2) {
        out.originX = j["position"][0].get<float>();
        out.originY = j["position"][1].get<float>();
    }
    if (j.contains("size") && j["size"].is_array() && j["size"].size() >= 2) {
        out.sizeW = j["size"][0].get<float>();
        out.sizeH = j["size"][1].get<float>();
    }
    for (const auto& ji : j.at("image")) {
        ImageDef img;
        std::string name = ji.value("name", "");
        size_t bar = name.find('|');
        if (bar == std::string::npos) {
            img.name = name;
            img.resourceId = name;
        } else {
            img.name = name.substr(0, bar);
            img.resourceId = name.substr(bar + 1);
        }
        if (ji.contains("size") && ji["size"].is_array() && ji["size"].size() >= 2) {
            img.w = ji["size"][0].get<float>();
            img.h = ji["size"][1].get<float>();
        }
        if (ji.contains("transform") && !ji["transform"].is_null())
            normalizeMat(ji["transform"], img.origin);
        out.images.push_back(std::move(img));
    }
    for (const auto& js : j.at("sprite")) {
        SpriteDef s;
        parseSprite(js, s, &out, out.stopCount, out.commandCount);
        out.sprites.push_back(std::move(s));
    }
    parseSprite(j.at("main_sprite"), out.mainSprite, &out, out.stopCount,
                out.commandCount);

    // Labels on the main sprite define segments: a label runs to the frame
    // before the next label, or to the last frame.
    const auto& frames = out.mainSprite.frames;
    for (size_t i = 0; i < frames.size(); i++) {
        if (!frames[i].hasLabel) continue;
        Segment seg;
        seg.first = (int)i;
        size_t next = frames.size();
        for (size_t k = i + 1; k < frames.size(); k++) {
            if (frames[k].hasLabel) {
                next = k;
                break;
            }
        }
        seg.last = (int)next - 1;
        out.segments[frames[i].label] = seg;
    }
    if (out.stopCount || out.commandCount)
        SDL_Log("pam: %s uses %d stop / %d command markers (ignored)",
                path.c_str(), out.stopCount, out.commandCount);
    return true;
}

bool PamPlayer::SegmentRange(const std::string& name, Segment& out) const {
    auto it = doc.segments.find(name);
    if (it == doc.segments.end()) return false;
    out = it->second;
    return true;
}

void PamPlayer::EvalMain(float frame, std::vector<DrawPart>& out) const {
    EvalSprite(-1, frame, matIdentity(), ColorMul{}, out);
}

void PamPlayer::EvalSegment(const Segment& seg, float offset,
                            std::vector<DrawPart>& out) const {
    int len = seg.last - seg.first + 1;
    float k = offset;
    if (len > 0) {
        k = fmodf(k, (float)len);
        if (k < 0) k += (float)len;
    }
    EvalSprite(-1, (float)seg.first + k, matIdentity(), ColorMul{}, out);
}

// Live display-list element while replaying frames 0..f.
struct Element {
    int resource = 0;
    bool sprite = false;
    Mat transform;             // identity at append
    ColorMul color;            // [1,1,1,1] at append
    float timescale = 1.0f;
    int baseChildFrame = 0;    // preload_frames / last anim_frame_num
    int baseParentFrame = 0;   // parent frame when baseChildFrame was set
};

Mat lerpMat(const Mat& a, const Mat& b, float s) {
    Mat m;
    m.a = a.a + (b.a - a.a) * s;
    m.b = a.b + (b.b - a.b) * s;
    m.c = a.c + (b.c - a.c) * s;
    m.d = a.d + (b.d - a.d) * s;
    m.tx = a.tx + (b.tx - a.tx) * s;
    m.ty = a.ty + (b.ty - a.ty) * s;
    return m;
}

ColorMul lerpColor(const ColorMul& a, const ColorMul& b, float s) {
    return ColorMul{a.r + (b.r - a.r) * s, a.g + (b.g - a.g) * s,
                    a.b + (b.b - a.b) * s, a.a + (b.a - a.a) * s};
}

void PamPlayer::EvalSprite(int spriteIndex, float frame, const Mat& parent,
                           const ColorMul& parentColor,
                           std::vector<DrawPart>& out, int depth) const {
    if (depth > 8) return; // malformed cyclic sprite refs
    const SpriteDef& s =
        spriteIndex < 0 ? doc.mainSprite : doc.sprites[spriteIndex];
    int count = (int)s.frames.size();
    if (count == 0) return;
    float cf = frame;
    if (cf < 0) cf = 0;
    if (cf > (float)(count - 1)) cf = (float)(count - 1);
    int f0 = (int)floorf(cf);
    float blend = cf - (float)f0;
    int f1 = f0 + 1 < count ? f0 + 1 : f0;

    auto applyOps = [&](std::map<int, Element>& list, int f) {
        const FrameOps& ops = s.frames[f];
        for (const AppendOp& a : ops.appends) {
            Element e;
            e.resource = a.resource;
            e.sprite = a.sprite;
            e.timescale = a.timescale;
            e.baseChildFrame = a.preloadFrames;
            e.baseParentFrame = f;
            list[a.index] = e;
        }
        for (int idx : ops.removes)
            list.erase(idx);
        for (const ChangeOp& c : ops.changes) {
            auto it = list.find(c.index);
            if (it == list.end()) continue; // change for a missing slot
            if (c.hasTransform) it->second.transform = c.transform;
            if (c.hasColor) it->second.color = c.color;
            if (c.animFrameNum != 0) {
                it->second.baseChildFrame = c.animFrameNum;
                it->second.baseParentFrame = f;
            }
        }
    };

    std::map<int, Element> list; // slot -> element, ascending draw order
    for (int i = 0; i <= f0; i++) applyOps(list, i);

    // Fractional frames interpolate each live slot's transform/color between
    // the two surrounding keyframes (retail Flash playback behavior), while
    // the display LIST itself stays the f0 one: appended/removed slots still
    // appear/disappear exactly at the frame boundary (no crossfade, same as
    // reanim's stepped image field).
    std::map<int, Element> next;
    if (blend > 0.0f && f1 != f0) {
        next = list;
        applyOps(next, f1);
    }

    for (const auto& kv : list) {
        Element e = kv.second;
        if (blend > 0.0f) {
            auto it1 = next.find(kv.first);
            if (it1 != next.end()) {
                e.transform = lerpMat(e.transform, it1->second.transform, blend);
                e.color = lerpColor(e.color, it1->second.color, blend);
            }
        }
        Mat world = Multiply(parent, e.transform);
        ColorMul color = Multiply(parentColor, e.color);
        if (!e.sprite) {
            if (e.resource < 0 || e.resource >= (int)doc.images.size()) continue;
            const ImageDef& img = doc.images[e.resource];
            DrawPart d;
            d.image = e.resource;
            d.world = Multiply(world, img.origin);
            d.color = color;
            out.push_back(d);
            continue;
        }
        if (e.resource < 0 || e.resource >= (int)doc.sprites.size()) continue;
        const SpriteDef& child = doc.sprites[e.resource];
        int childCount = (int)child.frames.size();
        if (childCount == 0) continue;
        // Local frame: base + timescale per advanced parent frame, looped
        // inside [workStart, childCount-1].
        float local =
            e.baseChildFrame + (cf - (float)e.baseParentFrame) * e.timescale;
        int start = child.workStart;
        int len = childCount - start;
        if (len <= 0) continue;
        float rel = fmodf(local - (float)start, (float)len);
        if (rel < 0) rel += (float)len;
        EvalSprite(e.resource, (float)start + rel, world, color, out, depth + 1);
    }
}

} // namespace pam
