/**
 * LineDrawing.cpp
 * Implementation of DDA and Bresenham line-drawing algorithms.
 *
 * Both algorithms plot individual pixels via OpenGL point primitives.
 * The 'progress' parameter allows partial drawing for animation effects.
 */

#include "../include/Algorithms/LineDrawing.h"
#include <GL/freeglut.h>
#include <cmath>
#include <cstdlib>

// ─────────────────────────────────────────────────────────────────────────────
//  Internal helper: plot a single screen pixel
// ─────────────────────────────────────────────────────────────────────────────
static void plotPixel(int x, int y, const Color& col) {
    glColor4f(col.r, col.g, col.b, col.a);
    glVertex2i(x, y);
}

// =============================================================================
//  DDA LINE ALGORITHM
//  ─────────────────
//  The Digital Differential Analyser computes the number of steps as the
//  maximum of |Δx| and |Δy|, then increments x and y by (Δx/steps) and
//  (Δy/steps) each step, rounding to the nearest integer pixel.
//
//  Advantage : simple, straightforward
//  Drawback  : floating-point arithmetic, potential rounding accumulation
// =============================================================================
void ddaLine(int x1, int y1, int x2, int y2,
             const Color& col, float progress)
{
    int dx = x2 - x1;
    int dy = y2 - y1;

    // Number of steps = largest dimension delta
    int steps = std::abs(dx) > std::abs(dy) ? std::abs(dx) : std::abs(dy);
    if (steps == 0) return; // same point

    // How many steps to actually draw (for animation)
    int drawSteps = static_cast<int>(steps * clamp(progress, 0.0f, 1.0f));

    float xInc = static_cast<float>(dx) / static_cast<float>(steps);
    float yInc = static_cast<float>(dy) / static_cast<float>(steps);

    float x = static_cast<float>(x1);
    float y = static_cast<float>(y1);

    glPointSize(1.5f);
    glBegin(GL_POINTS);
    for (int i = 0; i <= drawSteps; ++i) {
        plotPixel(static_cast<int>(std::round(x)),
                  static_cast<int>(std::round(y)), col);
        x += xInc;
        y += yInc;
    }
    glEnd();
}

// =============================================================================
//  BRESENHAM'S LINE ALGORITHM
//  ──────────────────────────
//  Uses only integer arithmetic.  A decision variable 'd' tracks the
//  cumulative error.  At each step only one of x or y increments; the other
//  increments only when the error crosses the half-pixel threshold.
//
//  This implementation generalises to all octants via sign-corrected
//  increments and a swap of Δx/Δy roles when |Δy| > |Δx|.
// =============================================================================
void bresenhamLine(int x1, int y1, int x2, int y2,
                   const Color& col, float progress)
{
    int dx = std::abs(x2 - x1);
    int dy = std::abs(y2 - y1);

    int sx = (x1 < x2) ? 1 : -1;   // x step direction
    int sy = (y1 < y2) ? 1 : -1;   // y step direction

    bool steep = dy > dx;            // swap roles when slope > 1
    if (steep) {
        std::swap(dx, dy);
    }

    int err = 2 * dy - dx;          // initial decision variable

    int totalSteps = dx;
    int drawSteps  = static_cast<int>(totalSteps * clamp(progress, 0.0f, 1.0f));

    int cx = x1, cy = y1;

    glPointSize(1.5f);
    glBegin(GL_POINTS);
    for (int i = 0; i <= drawSteps; ++i) {
        plotPixel(cx, cy, col);

        if (err >= 0) {
            // Move in both directions
            if (steep)  cx += sx;
            else        cy += sy;
            err -= 2 * dx;
        }
        // Always step in the primary direction
        if (steep)  cy += sy;
        else        cx += sx;
        err += 2 * dy;
    }
    glEnd();
}

// =============================================================================
//  THICK LINE
//  Draws a line with a given thickness by drawing multiple parallel offset
//  lines using the chosen algorithm.
// =============================================================================
void thickLine(int x1, int y1, int x2, int y2,
               const Color& col, int thickness, float progress,
               bool useDDA)
{
    // Perpendicular direction unit vector
    float dx = static_cast<float>(x2 - x1);
    float dy = static_cast<float>(y2 - y1);
    float len = std::sqrt(dx*dx + dy*dy);
    if (len < 1e-6f) return;

    float px = -dy / len;   // perpendicular x
    float py =  dx / len;   // perpendicular y

    int half = thickness / 2;
    for (int t = -half; t <= half; ++t) {
        int ox = static_cast<int>(px * t);
        int oy = static_cast<int>(py * t);
        if (useDDA)
            ddaLine(x1+ox, y1+oy, x2+ox, y2+oy, col, progress);
        else
            bresenhamLine(x1+ox, y1+oy, x2+ox, y2+oy, col, progress);
    }
}
