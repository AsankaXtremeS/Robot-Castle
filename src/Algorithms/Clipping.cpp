/**
 * Clipping.cpp
 * Point, Cohen-Sutherland, Liang-Barsky, and Sutherland-Hodgman clipping.
 */

#include "../include/Algorithms/Clipping.h"
#include <cmath>
#include <algorithm>

// =============================================================================
//  POINT CLIPPING
//  ──────────────
//  Trivially rejects any point outside the rectangular clip window.
// =============================================================================
bool pointClip(float x, float y, const ClipWindow& win)
{
    return (x >= win.xMin && x <= win.xMax &&
            y >= win.yMin && y <= win.yMax);
}

// =============================================================================
//  COHEN-SUTHERLAND OUTCODES
// =============================================================================
int computeOutCode(float x, float y, const ClipWindow& win)
{
    int code = INSIDE;
    if      (x < win.xMin) code |= LEFT;
    else if (x > win.xMax) code |= RIGHT;
    if      (y < win.yMin) code |= BOTTOM;
    else if (y > win.yMax) code |= TOP;
    return code;
}

// =============================================================================
//  COHEN-SUTHERLAND LINE CLIPPING
//  ────────────────────────────────
//  Assigns a 4-bit outcode to each endpoint.
//    • Trivial accept:  both outcodes == INSIDE
//    • Trivial reject:  bitwise AND of outcodes != 0
//    • Otherwise:       clip against the edge corresponding to a set bit,
//                       compute intersection, update the outside endpoint,
//                       repeat.
// =============================================================================
bool cohenSutherlandClip(float& x1, float& y1, float& x2, float& y2,
                          const ClipWindow& win)
{
    int code1 = computeOutCode(x1, y1, win);
    int code2 = computeOutCode(x2, y2, win);

    while (true) {
        if (!(code1 | code2)) {
            return true;        // trivial accept – both inside
        }
        if (code1 & code2) {
            return false;       // trivial reject – both outside same edge
        }

        // Pick the endpoint that is outside the clip window
        int codeOut = code1 ? code1 : code2;

        float x = 0, y = 0;
        float dx = x2 - x1;
        float dy = y2 - y1;

        // Compute intersection with the relevant clip edge
        if (codeOut & TOP) {
            x = x1 + dx * (win.yMax - y1) / dy;
            y = win.yMax;
        } else if (codeOut & BOTTOM) {
            x = x1 + dx * (win.yMin - y1) / dy;
            y = win.yMin;
        } else if (codeOut & RIGHT) {
            y = y1 + dy * (win.xMax - x1) / dx;
            x = win.xMax;
        } else { // LEFT
            y = y1 + dy * (win.xMin - x1) / dx;
            x = win.xMin;
        }

        // Replace the outside endpoint and re-compute its outcode
        if (codeOut == code1) {
            x1 = x; y1 = y;
            code1 = computeOutCode(x1, y1, win);
        } else {
            x2 = x; y2 = y;
            code2 = computeOutCode(x2, y2, win);
        }
    }
}

// =============================================================================
//  LIANG-BARSKY LINE CLIPPING
//  ────────────────────────────
//  Parametric form: P(t) = P0 + t*(P1-P0),  t ∈ [0,1]
//  Represents each clip boundary as  p*t ≤ q.
//  p < 0 → line enters from this boundary  → update t0 = max(t0, q/p)
//  p > 0 → line exits  at this boundary    → update t1 = min(t1, q/p)
//  p = 0 → line parallel to edge; reject if q < 0
// =============================================================================
bool liangBarskyClip(float& x1, float& y1, float& x2, float& y2,
                      const ClipWindow& win)
{
    float dx = x2 - x1;
    float dy = y2 - y1;

    // p and q values for all 4 clip planes (left,right,bottom,top)
    float p[4] = { -dx,  dx, -dy,  dy };
    float q[4] = { x1 - win.xMin,
                   win.xMax - x1,
                   y1 - win.yMin,
                   win.yMax - y1 };

    float t0 = 0.0f;   // entering parameter
    float t1 = 1.0f;   // exiting  parameter

    for (int i = 0; i < 4; ++i) {
        if (std::abs(p[i]) < 1e-6f) {
            // Parallel to this edge
            if (q[i] < 0) return false; // outside – reject
        } else {
            float r = q[i] / p[i];
            if (p[i] < 0)
                t0 = std::max(t0, r);  // entering
            else
                t1 = std::min(t1, r);  // exiting
        }
    }

    if (t0 > t1) return false; // completely outside

    float nx1 = x1 + t0 * dx;
    float ny1 = y1 + t0 * dy;
    float nx2 = x1 + t1 * dx;
    float ny2 = y1 + t1 * dy;

    x1 = nx1; y1 = ny1;
    x2 = nx2; y2 = ny2;
    return true;
}

// =============================================================================
//  SUTHERLAND-HODGMAN POLYGON CLIPPING
//  ─────────────────────────────────────
//  Clips the polygon against each of the four clip edges in sequence.
//  For each edge, the Sutherland-Hodgman "inside" test is applied to each
//  pair of consecutive vertices:
//    Inside→Inside   : output endpoint
//    Inside→Outside  : output intersection
//    Outside→Inside  : output intersection, then output endpoint
//    Outside→Outside : output nothing
// =============================================================================

// Helper: is point p inside the clip half-plane for the given edge?
// Edges: 0=left, 1=right, 2=bottom, 3=top
static bool insideSH(const Point2D& p, int edge, const ClipWindow& win) {
    switch (edge) {
        case 0: return p.x >= win.xMin; // left
        case 1: return p.x <= win.xMax; // right
        case 2: return p.y >= win.yMin; // bottom
        case 3: return p.y <= win.yMax; // top
    }
    return false;
}

// Helper: compute intersection of segment p1-p2 with clip edge
Point2D intersectEdge(const Point2D& p1, const Point2D& p2,
                       int edge, const ClipWindow& win)
{
    float dx = p2.x - p1.x;
    float dy = p2.y - p1.y;
    float t  = 0.0f;

    switch (edge) {
        case 0: t = (win.xMin - p1.x) / (dx ? dx : 1e-9f); break; // left
        case 1: t = (win.xMax - p1.x) / (dx ? dx : 1e-9f); break; // right
        case 2: t = (win.yMin - p1.y) / (dy ? dy : 1e-9f); break; // bottom
        case 3: t = (win.yMax - p1.y) / (dy ? dy : 1e-9f); break; // top
    }

    return Point2D(p1.x + t * dx, p1.y + t * dy);
}

std::vector<Point2D> sutherlandHodgmanClip(
    const std::vector<Point2D>& polygon,
    const ClipWindow& win)
{
    std::vector<Point2D> output = polygon;

    for (int edge = 0; edge < 4; ++edge) {
        if (output.empty()) break;

        std::vector<Point2D> input = output;
        output.clear();

        for (int i = 0; i < (int)input.size(); ++i) {
            const Point2D& curr = input[i];
            const Point2D& prev = input[(i - 1 + input.size()) % input.size()];

            bool currIn = insideSH(curr, edge, win);
            bool prevIn = insideSH(prev, edge, win);

            if (currIn) {
                if (!prevIn)
                    output.push_back(intersectEdge(prev, curr, edge, win));
                output.push_back(curr);
            } else if (prevIn) {
                output.push_back(intersectEdge(prev, curr, edge, win));
            }
            // Outside → Outside: do nothing
        }
    }

    return output;
}
