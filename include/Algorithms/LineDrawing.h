/**
 * LineDrawing.h
 * From-scratch implementations of:
 *   • DDA (Digital Differential Analyser) Line Algorithm
 *   • Bresenham's Line Drawing Algorithm
 *
 * Both functions draw directly via glBegin/glVertex2i and accept an optional
 * "progress" parameter (0–1) so the line can be animated pixel-by-pixel.
 */

#pragma once
#include "../Common.h"

// ─────────────────────────────────────────────────────────────────────────────
//  DDA Line Algorithm
//  Computes floating-point increments dx/steps and dy/steps, then rounds each
//  sample to the nearest pixel.  Simple and readable but uses floats.
//
//  Parameters:
//    x1,y1   – start point (screen pixels)
//    x2,y2   – end point   (screen pixels)
//    col     – draw colour
//    progress – fraction of the line to draw [0..1]  (1 = full line)
// ─────────────────────────────────────────────────────────────────────────────
void ddaLine(int x1, int y1, int x2, int y2,
             const Color& col, float progress = 1.0f);

// ─────────────────────────────────────────────────────────────────────────────
//  Bresenham's Line Algorithm
//  Uses only integer arithmetic (no floating-point) via an incremental decision
//  variable that selects which pixel is closest to the ideal line at each step.
//
//  Parameters: same as ddaLine
// ─────────────────────────────────────────────────────────────────────────────
void bresenhamLine(int x1, int y1, int x2, int y2,
                   const Color& col, float progress = 1.0f);

// ─────────────────────────────────────────────────────────────────────────────
//  Utility: draw a thick line using repeated parallel lines
// ─────────────────────────────────────────────────────────────────────────────
void thickLine(int x1, int y1, int x2, int y2,
               const Color& col, int thickness, float progress = 1.0f,
               bool useDDA = true);
