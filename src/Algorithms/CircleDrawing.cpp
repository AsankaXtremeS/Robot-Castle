/**
 * CircleDrawing.cpp
 * Implementation of Midpoint Circle and Bresenham Circle algorithms.
 *
 * Both exploit 8-way symmetry – only the first octant is computed; the other
 * seven mirror points are derived by sign-swapping x and y.
 */

#include "../include/Algorithms/CircleDrawing.h"
#include <GL/freeglut.h>
#include <cmath>

// ─────────────────────────────────────────────────────────────────────────────
//  Plot all 8 symmetric points of a circle
// ─────────────────────────────────────────────────────────────────────────────
static void plot8(int cx, int cy, int px, int py, const Color& col) {
    glColor4f(col.r, col.g, col.b, col.a);
    glVertex2i(cx + px, cy + py);
    glVertex2i(cx - px, cy + py);
    glVertex2i(cx + px, cy - py);
    glVertex2i(cx - px, cy - py);
    glVertex2i(cx + py, cy + px);
    glVertex2i(cx - py, cy + px);
    glVertex2i(cx + py, cy - px);
    glVertex2i(cx - py, cy - px);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Draw a horizontal span for filled circles
// ─────────────────────────────────────────────────────────────────────────────
static void fillSpan8(int cx, int cy, int px, int py, const Color& col) {
    glColor4f(col.r, col.g, col.b, col.a);
    // Horizontal lines connecting symmetric pairs
    for (int x = cx - px; x <= cx + px; ++x) {
        glVertex2i(x, cy + py);
        glVertex2i(x, cy - py);
    }
    for (int x = cx - py; x <= cx + py; ++x) {
        glVertex2i(x, cy + px);
        glVertex2i(x, cy - px);
    }
}

// =============================================================================
//  MIDPOINT CIRCLE ALGORITHM
//  ─────────────────────────
//  Starts at (0, r) in the first octant.  The decision parameter is
//  initialised to p = 1 – r.
//
//    If p < 0:  next point is (x+1, y)        → p += 2x + 3
//    If p ≥ 0:  next point is (x+1, y-1)     → p += 2x - 2y + 5
//
//  Stops when x ≥ y (one octant complete; symmetry fills the rest).
// =============================================================================
void midpointCircle(int cx, int cy, int radius,
                    const Color& col, float progress, bool filled)
{
    if (radius <= 0) return;

    // Total points in one octant ≈ radius (upper bound)
    int maxPts = radius + 1;
    int drawPts = static_cast<int>(maxPts * clamp(progress, 0.0f, 1.0f));

    int x = 0;
    int y = radius;
    int p = 1 - radius;   // initial decision parameter

    glPointSize(1.5f);
    GLenum mode = filled ? GL_LINES : GL_POINTS;
    glBegin(mode);

    for (int count = 0; x <= y && count <= drawPts; ++count) {
        if (filled)
            fillSpan8(cx, cy, x, y, col);
        else
            plot8(cx, cy, x, y, col);

        ++x;
        if (p < 0) {
            p += 2 * x + 1;
        } else {
            --y;
            p += 2 * (x - y) + 1;
        }
    }

    glEnd();
}

// =============================================================================
//  BRESENHAM CIRCLE ALGORITHM
//  ──────────────────────────
//  Very similar to midpoint but uses the decision variable
//  d = 3 – 2 * radius.
//
//    If d < 0:  d += 4x + 6
//    If d ≥ 0:  d += 4(x - y) + 10, then y--
// =============================================================================
void bresenhamCircle(int cx, int cy, int radius,
                     const Color& col, float progress, bool filled)
{
    if (radius <= 0) return;

    int maxPts = radius + 1;
    int drawPts = static_cast<int>(maxPts * clamp(progress, 0.0f, 1.0f));

    int x = 0;
    int y = radius;
    int d = 3 - 2 * radius;   // Bresenham's initial decision variable

    glPointSize(1.5f);
    GLenum mode = filled ? GL_LINES : GL_POINTS;
    glBegin(mode);

    for (int count = 0; x <= y && count <= drawPts; ++count) {
        if (filled)
            fillSpan8(cx, cy, x, y, col);
        else
            plot8(cx, cy, x, y, col);

        ++x;
        if (d < 0) {
            d += 4 * x + 6;
        } else {
            --y;
            d += 4 * (x - y) + 10;
        }
    }

    glEnd();
}

// =============================================================================
//  CONCENTRIC CIRCLES
//  Draws 'rings' evenly spaced concentric circles up to outerR.
// =============================================================================
void concentricCircles(int cx, int cy, int outerR, int rings,
                       const Color& col, float progress)
{
    if (rings <= 0 || outerR <= 0) return;
    int step = outerR / rings;
    for (int r = step; r <= outerR; r += step) {
        // Use alternating algorithms for visual variety
        float p = clamp((progress * outerR - (outerR - r)) / (float)outerR,
                        0.0f, 1.0f);
        if ((r / step) % 2 == 0)
            midpointCircle(cx, cy, r, col, p, false);
        else
            bresenhamCircle(cx, cy, r, col, p, false);
    }
}
