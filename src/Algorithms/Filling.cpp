/**
 * Filling.cpp
 * Scan-Line Polygon Fill, Flood Fill, and Boundary Fill implementations.
 */

#include "../include/Algorithms/Filling.h"
#include <GL/freeglut.h>
#include <algorithm>
#include <vector>
#include <stack>
#include <cmath>

// ─────────────────────────────────────────────────────────────────────────────
//  Edge table entry for scan-line fill
// ─────────────────────────────────────────────────────────────────────────────
struct Edge {
    int   yMax;      // upper y of the edge
    float xOfYMin;   // x-coordinate at the lower y of the edge
    float invSlope;  // 1/m (x increment per scan-line)

    // Sort by xOfYMin for the active edge list
    bool operator<(const Edge& o) const { return xOfYMin < o.xOfYMin; }
};

// =============================================================================
//  SCAN-LINE POLYGON FILL
//  ──────────────────────
//  Algorithm:
//    1. Build an "edge table" (ET): for each polygon edge compute yMax,
//       xAtYMin, and 1/slope.  Skip horizontal edges.
//    2. For each scan-line y from yMin to yMax:
//         a. Move edges from ET whose yMin == y into the Active Edge List (AEL).
//         b. Sort AEL by x.
//         c. Fill pixels between pairs of x-intersections (even-odd rule).
//         d. Remove edges where yMax == y.
//         e. Update x of remaining AEL edges by += 1/slope.
// =============================================================================
void scanLineFill(const Polygon2D& poly, float progress)
{
    const auto& verts = poly.vertices;
    int n = static_cast<int>(verts.size());
    if (n < 3) return;

    // Find y extent
    int yMin = INT_MAX, yMax = INT_MIN;
    for (auto& v : verts) {
        yMin = std::min(yMin, (int)std::floor(v.y));
        yMax = std::max(yMax, (int)std::ceil(v.y));
    }

    // Build edge table indexed by yMin of each edge
    std::vector<std::vector<Edge>> ET(yMax - yMin + 1);
    for (int i = 0; i < n; ++i) {
        int j = (i + 1) % n;
        float y0 = verts[i].y, y1 = verts[j].y;
        float x0 = verts[i].x, x1 = verts[j].x;

        if ((int)y0 == (int)y1) continue; // skip horizontal

        if (y0 > y1) { std::swap(y0,y1); std::swap(x0,x1); }

        Edge e;
        e.yMax      = (int)std::ceil(y1);
        e.xOfYMin   = x0;
        e.invSlope  = (x1 - x0) / (y1 - y0);

        int idx = (int)std::floor(y0) - yMin;
        if (idx >= 0 && idx < (int)ET.size())
            ET[idx].push_back(e);
    }

    // Determine how many scan-lines to draw (animation)
    int totalLines = yMax - yMin;
    int drawLines  = static_cast<int>(totalLines * clamp(progress, 0.0f, 1.0f));

    const Color& col = poly.fillColor;
    glColor4f(col.r, col.g, col.b, col.a);
    glBegin(GL_LINES);

    std::vector<Edge> AEL; // active edge list

    for (int y = yMin; y <= yMin + drawLines && y <= yMax; ++y) {
        int idx = y - yMin;
        // Add new edges
        if (idx < (int)ET.size())
            for (auto& e : ET[idx]) AEL.push_back(e);

        // Remove edges where yMax <= current y
        AEL.erase(std::remove_if(AEL.begin(), AEL.end(),
            [&](const Edge& e){ return e.yMax <= y; }), AEL.end());

        // Sort by x
        std::sort(AEL.begin(), AEL.end());

        // Fill between pairs
        for (int k = 0; k + 1 < (int)AEL.size(); k += 2) {
            int xL = (int)std::ceil(AEL[k].xOfYMin);
            int xR = (int)std::floor(AEL[k+1].xOfYMin);
            glVertex2i(xL, y);
            glVertex2i(xR, y);
        }

        // Update x for next scan-line
        for (auto& e : AEL) e.xOfYMin += e.invSlope;
    }
    glEnd();
}

// =============================================================================
//  FLOOD FILL  (4-connected, stack-based)
//  ──────────────────────────────────────
//  Replaces all bgColor pixels connected to the seed with fillColor.
//  Uses an explicit stack to avoid deep recursion on large areas.
// =============================================================================
void floodFill(int seedX, int seedY,
               const Color& fillColor, const Color& bgColor,
               std::function<Color(int,int)> getPixel,
               std::function<void(int,int,Color)> setPixel,
               const ClipWindow& bounds)
{
    // Tolerance for colour comparison
    auto colEq = [](const Color& a, const Color& b) {
        return std::abs(a.r-b.r) < 0.01f &&
               std::abs(a.g-b.g) < 0.01f &&
               std::abs(a.b-b.b) < 0.01f;
    };

    if (!colEq(getPixel(seedX, seedY), bgColor)) return;

    std::stack<std::pair<int,int>> stk;
    stk.push({seedX, seedY});

    const int dx[4] = {1, -1, 0,  0};
    const int dy[4] = {0,  0, 1, -1};

    while (!stk.empty()) {
        auto [x, y] = stk.top(); stk.pop();

        if (x < (int)bounds.xMin || x > (int)bounds.xMax ||
            y < (int)bounds.yMin || y > (int)bounds.yMax) continue;

        if (!colEq(getPixel(x, y), bgColor)) continue;

        setPixel(x, y, fillColor);

        for (int d = 0; d < 4; ++d)
            stk.push({x + dx[d], y + dy[d]});
    }
}

// =============================================================================
//  BOUNDARY FILL  (4-connected, stack-based)
//  ──────────────────────────────────────────
//  Fills from the seed outward, stopping when it hits a borderColor pixel.
// =============================================================================
void boundaryFill(int seedX, int seedY,
                  const Color& fillColor, const Color& borderColor,
                  std::function<Color(int,int)> getPixel,
                  std::function<void(int,int,Color)> setPixel,
                  const ClipWindow& bounds)
{
    auto colEq = [](const Color& a, const Color& b) {
        return std::abs(a.r-b.r) < 0.01f &&
               std::abs(a.g-b.g) < 0.01f &&
               std::abs(a.b-b.b) < 0.01f;
    };

    Color seed = getPixel(seedX, seedY);
    if (colEq(seed, borderColor) || colEq(seed, fillColor)) return;

    std::stack<std::pair<int,int>> stk;
    stk.push({seedX, seedY});

    const int dx[4] = {1, -1, 0,  0};
    const int dy[4] = {0,  0, 1, -1};

    while (!stk.empty()) {
        auto [x, y] = stk.top(); stk.pop();

        if (x < (int)bounds.xMin || x > (int)bounds.xMax ||
            y < (int)bounds.yMin || y > (int)bounds.yMax) continue;

        Color cur = getPixel(x, y);
        if (colEq(cur, borderColor) || colEq(cur, fillColor)) continue;

        setPixel(x, y, fillColor);

        for (int d = 0; d < 4; ++d)
            stk.push({x + dx[d], y + dy[d]});
    }
}

// =============================================================================
//  UTILITY: Draw filled polygon using scan-line fill
// =============================================================================
void drawFilledPolygon(const std::vector<Point2D>& verts,
                       const Color& col, float progress)
{
    Polygon2D poly;
    poly.vertices  = verts;
    poly.fillColor = col;
    scanLineFill(poly, progress);
}

// =============================================================================
//  UTILITY: Draw filled rectangle using scan-line approach
// =============================================================================
void drawFilledRect(float x, float y, float w, float h,
                    const Color& col, float progress)
{
    std::vector<Point2D> verts = {
        {x,     y},
        {x + w, y},
        {x + w, y + h},
        {x,     y + h}
    };
    drawFilledPolygon(verts, col, progress);
}
