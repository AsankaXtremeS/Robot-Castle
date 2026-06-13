/**
 * CircleDrawing.h
 * From-scratch implementations of:
 *   • Midpoint Circle Algorithm
 *   • Bresenham's Circle Algorithm
 *
 * Both exploit 8-way symmetry – only the first octant is computed; the other
 * seven are filled in by reflection.  Both accept a "progress" parameter so
 * the circle can be drawn arc-by-arc for animation.
 */

#pragma once
#include "../Common.h"

// ─────────────────────────────────────────────────────────────────────────────
//  Midpoint Circle Algorithm
//  Initialises decision parameter p = 1 – r, then increments x each step and
//  adjusts y only when p ≥ 0.  Eight symmetric points are plotted per step.
//
//  Parameters:
//    cx,cy    – centre (pixels)
//    radius   – radius  (pixels)
//    col      – draw colour
//    progress – fraction of circumference drawn [0..1]
//    filled   – if true, draws filled circles using horizontal spans
// ─────────────────────────────────────────────────────────────────────────────
void midpointCircle(int cx, int cy, int radius,
                    const Color& col, float progress = 1.0f,
                    bool filled = false);

// ─────────────────────────────────────────────────────────────────────────────
//  Bresenham Circle Algorithm
//  Very similar to midpoint; the decision variable is initialised to
//  3 – 2*radius and uses pure integer arithmetic throughout.
//
//  Parameters: same as midpointCircle
// ─────────────────────────────────────────────────────────────────────────────
void bresenhamCircle(int cx, int cy, int radius,
                     const Color& col, float progress = 1.0f,
                     bool filled = false);

// ─────────────────────────────────────────────────────────────────────────────
//  Utility: draw concentric circles (decorative rings)
// ─────────────────────────────────────────────────────────────────────────────
void concentricCircles(int cx, int cy, int outerR, int rings,
                       const Color& col, float progress = 1.0f);
