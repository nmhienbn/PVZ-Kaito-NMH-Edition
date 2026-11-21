#include "reanim.hpp"

#include <cstdio>
#include <cstdlib>
#include <cctype>
#include <cmath>
#include <unordered_map>

namespace reanim {

namespace {

bool readTag(const std::string& blk, const char* tag, std::string& out) {
    std::string open = std::string("<") + tag + ">";
    size_t p = blk.find(open);
    if (p == std::string::npos) return false;
    p += open.size();
    std::string close = std::string("</") + tag + ">";
    size_t e = blk.find(close, p);
    if (e == std::string::npos) return false;
    out = blk.substr(p, e - p);
    return true;
}

bool readFile(const std::string& path, std::string& out) {
    FILE* f = fopen(path.c_str(), "rb");
    if (!f) return false;
    fseek(f, 0, SEEK_END);
    long n = ftell(f);
    fseek(f, 0, SEEK_SET);
    out.resize(n > 0 ? (size_t)n : 0);
    size_t rd = n > 0 ? fread(&out[0], 1, (size_t)n, f) : 0;
    fclose(f);
    out.resize(rd);
    return true;
}

float toFloat(const std::string& s) { return strtof(s.c_str(), nullptr); }

} // namespace

bool LoadReanimXml(const std::string& path, ReanimDoc& out, std::string& err) {
    std::string s;
    if (!readFile(path, s)) {
        err = "cannot open " + path;
        return false;
    }
    std::string v;
    if (readTag(s, "fps", v)) out.fps = toFloat(v);

    std::unordered_map<std::string, int> imageIds;
    auto internImage = [&](const std::string& id) -> int {
        auto it = imageIds.find(id);
        if (it != imageIds.end()) return it->second;
        int idx = (int)out.images.size();
        imageIds.emplace(id, idx);
        out.images.push_back(id);
        return idx;
    };

    size_t p = 0;
    while ((p = s.find("<track>", p)) != std::string::npos) {
        size_t e = s.find("</track>", p);
        if (e == std::string::npos) break;
        std::string tb = s.substr(p + 7, e - (p + 7));
        p = e + 9;

        Track tr;
        readTag(tb, "name", tr.name);

        Transform st; // carry-over state, starts at PopCap defaults
        size_t q = 0;
        for (;;) {
            size_t openFull = tb.find("<t>", q);
            size_t openEmpty = tb.find("<t/>", q);
            if (openFull == std::string::npos && openEmpty == std::string::npos) break;
            std::string content;
            if (openEmpty != std::string::npos &&
                (openFull == std::string::npos || openEmpty < openFull)) {
                q = openEmpty + 4;
            } else {
                size_t e2 = tb.find("</t>", openFull);
                if (e2 == std::string::npos) break;
                content = tb.substr(openFull + 3, e2 - (openFull + 3));
                q = e2 + 4;
            }
            if (readTag(content, "x", v))  st.x = toFloat(v);
            if (readTag(content, "y", v))  st.y = toFloat(v);
            if (readTag(content, "sx", v)) st.sx = toFloat(v);
            if (readTag(content, "sy", v)) st.sy = toFloat(v);
            if (readTag(content, "kx", v)) st.kx = toFloat(v);
            if (readTag(content, "ky", v)) st.ky = toFloat(v);
            if (readTag(content, "f", v))  st.f = toFloat(v);
            if (readTag(content, "a", v))  st.a = toFloat(v);
            if (readTag(content, "i", v))
                st.image = v.empty() ? -1 : internImage(v);
            tr.frames.push_back(st);
        }
        for (const auto& fr : tr.frames)
            if (fr.image >= 0) { tr.hasAnyImage = true; break; }
        out.frameCount = (std::max)(out.frameCount, (int)tr.frames.size());
        out.tracks.push_back(std::move(tr));
    }

    // Pseudo-tracks (no image anywhere, name not starting with '_') mark
    // animation segments: their visible frame range IS the segment.
    for (const auto& tr : out.tracks) {
        if (tr.hasAnyImage || tr.name.empty() || tr.name[0] == '_') continue;
        int first = -1, last = -1;
        for (int i = 0; i < (int)tr.frames.size(); i++) {
            if (tr.frames[i].f >= 0.0f) {
                if (first < 0) first = i;
                last = i;
            }
        }
        if (first >= 0) out.segments[tr.name] = Segment{first, last};
    }
    return true;
}

Transform SampleTrack(const Track& track, float frame, int loopFirst, int loopLast,
                      float loopBlend) {
    const auto& fr = track.frames;
    if (fr.empty()) return Transform{};
    float hi = (float)fr.size() - 1.0f;
    float cf = frame < 0.0f ? 0.0f : (frame > hi ? hi : frame);
    bool wrap = loopFirst >= 0 && loopLast >= loopFirst;
    // Wider wrap cross-blend (unclosed 2Pak takes only, callers opt in per
    // segment): the window's last loopBlend frames fade toward the loop's
    // first pose, so a several-step pose gap at the wrap spreads over the
    // fade instead of popping in one frame. loopBlend == 1 falls through to
    // the exact original one-frame seam path below.
    if (wrap && loopBlend > 1.0f) {
        float zoneStart = (float)loopLast + 1.0f - loopBlend;
        if (cf > zoneStart) {
            float s = (cf - zoneStart) / loopBlend;
            if (s > 1.0f) s = 1.0f;
            // A side keeps the normal advance (its own 1-frame seam ease
            // handles the tail bit past loopLast); B side is the static
            // first pose, which the fade reaches exactly at the wrap.
            Transform A = SampleTrack(track, cf, loopFirst, loopLast, 1.0f);
            const Transform& B = fr[loopFirst];
            Transform r;
            r.x = A.x + (B.x - A.x) * s;
            r.y = A.y + (B.y - A.y) * s;
            r.sx = A.sx + (B.sx - A.sx) * s;
            r.sy = A.sy + (B.sy - A.sy) * s;
            r.kx = A.kx + (B.kx - A.kx) * s;
            r.ky = A.ky + (B.ky - A.ky) * s;
            r.a = A.a + (B.a - A.a) * s;
            // stepped fields follow the dominant side: no half-shown images
            r.f = s < 0.5f ? A.f : B.f;
            r.image = s < 0.5f ? A.image : B.image;
            return r;
        }
    }
    // Looped playback keeps cf inside (loopLast, loopLast+1): fractional frames
    // there wrap-blend into the first frame (the seam), not the next segment.
    // Only frames past the seam snap back to the end pose.
    if (wrap && cf >= (float)loopLast + 1.0f)
        cf = (float)loopLast;
    int i0 = (int)floorf(cf);
    int i1 = i0 + 1 < (int)fr.size() ? i0 + 1 : i0;
    if (wrap && i1 > loopLast)
        i1 = loopFirst; // wrap the loop: blend into the segment's first frame
    float s = cf - (float)i0;
    const Transform& A = fr[i0];
    const Transform& B = fr[i1];
    Transform r;
    r.x = A.x + (B.x - A.x) * s;
    r.y = A.y + (B.y - A.y) * s;
    r.sx = A.sx + (B.sx - A.sx) * s;
    r.sy = A.sy + (B.sy - A.sy) * s;
    r.kx = A.kx + (B.kx - A.kx) * s;
    r.ky = A.ky + (B.ky - A.ky) * s;
    r.a = A.a + (B.a - A.a) * s;
    r.f = A.f;
    r.image = A.image;
    return r;
}

std::string ImageFileStem(const std::string& imageId) {
    std::string s = imageId;
    if (s.size() > 13 && s.compare(0, 13, "IMAGE_REANIM_") == 0)
        s.erase(0, 13);
    for (char& c : s) c = (char)tolower((unsigned char)c);
    return s;
}

bool SegmentEndsOnLoopSeam(const ReanimDoc& doc, int first, int last) {
    if (last <= first || first < 0) return false;
    bool any = false;
    for (const Track& tr : doc.tracks) {
        if (!tr.hasAnyImage) continue;
        if ((int)tr.frames.size() <= last) continue;
        const Transform& A = tr.frames[first];
        const Transform& B = tr.frames[last];
        // Only the endpoints' RENDERED pose counts: parts hidden by f=-1 at
        // both ends (e.g. the repeater-only eyebrow track on the shared
        // peashooter doc) may sit anywhere offscreen without making the
        // visible endpoint poses differ.
        bool visA = A.image >= 0 && A.f > -0.5f;
        bool visB = B.image >= 0 && B.f > -0.5f;
        if (!visA && !visB) continue;
        any = true;
        if (visA != visB) return false;
        if (A.image != B.image) return false;
        // 2Pak's duplicate-seam frames carry export rounding of ~0.3px; treat
        // sub-pixel pose equality as "same pose" (trimming such a frame is
        // visually free even on the rare false positive).
        const float P_EPS = 0.5f;  // px / deg
        const float S_EPS = 0.01f; // scale / alpha
        if (fabsf(A.x - B.x) > P_EPS || fabsf(A.y - B.y) > P_EPS ||
            fabsf(A.kx - B.kx) > P_EPS || fabsf(A.ky - B.ky) > P_EPS ||
            fabsf(A.sx - B.sx) > S_EPS || fabsf(A.sy - B.sy) > S_EPS ||
            fabsf(A.a - B.a) > S_EPS)
            return false;
    }
    return any;
}

} // namespace reanim
