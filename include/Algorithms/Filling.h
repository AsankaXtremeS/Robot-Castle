/**
 * Filling.h
 * From-scratch implementations of polygon/region filling algorithms:
 *   • Scan-Line Polygon Fill  (edge table + active edge list)
 *   • Flood Fill              (4-connected, stack-based)
 *   • Boundary Fill           (4-connected boundary-stop, stack-based)
 */

#pragma once
#include "../Common.h"
#include <vector>
#include <functional>

// ─────────────────────────────────────────────────────────────────────────────
//  Scan-Line Polygon Fill
//  Builds an edge table from the polygon vertices, sweeps horizontal scan-lines
//  from yMin to yMax, fills between pairs of intersections on the active edge
//  list.  Standard even-odd fill rule.
//
//  Parameters:
//    poly     – polygon to fill
//    progress – fraction of scan-lines drawn [0..1] for animation
// ─────────────────────────────────────────────────────────────────────────────
void scanLineFill(const Polygon2D& poly, float progress = 1.0f);

// ─────────────────────────────────────────────────────────────────────────────
//  Flood Fill  (4-connected)
//  Seeds from (seedX, seedY) and replaces all pixels matching bgColor with
//  fillColor.  Uses an explicit stack to avoid recursion stack overflows on
//  large regions.
//
//  Parameters:
//    seedX,seedY   – starting pixel
//    fillColor     – colour to paint
//    bgColor       – colour to replace
//    getPixel      – callback: returns colour at (x,y)
//    setPixel      – callback: sets colour at (x,y)
//    bounds        – axis-aligned bounding box to constrain the fill
// ─────────────────────────────────────────────────────────────────────────────
void floodFill(int seedX, int seedY,
               const Color& fillColor, const Color& bgColor,
               std::function<Color(int,int)> getPixel,
               std::function<void(int,int,Color)> setPixel,
               const ClipWindow& bounds);

// ─────────────────────────────────────────────────────────────────────────────
//  Boundary Fill  (4-connected)
//  Seeds from (seedX, seedY) and fills until a pixel matching borderColor is
//  encountered.  Uses an explicit stack.
// ─────────────────────────────────────────────────────────────────────────────
void boundaryFill(int seedX, int seedY,
                  const Color& fillColor, const Color& borderColor,
                  std::function<Color(int,int)> getPixel,
                  std::function<void(int,int,Color)> setPixel,
                  const ClipWindow& bounds);

// ─────────────────────────────────────────────────────────────────────────────
//  Utility: draw a filled polygon directly with OpenGL (uses scan-line fill)
// ─────────────────────────────────────────────────────────────────────────────
void drawFilledPolygon(const std::vector<Point2D>& verts,
                       const Color& col, float progress = 1.0f);

// Helper: draw a filled rectangle using scan-line approach
void drawFilledRect(float x, float y, float w, float h,
                    const Color& col, float progress = 1.0f);
