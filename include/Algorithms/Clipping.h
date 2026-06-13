/**
 * Clipping.h
 * From-scratch implementations of all required clipping algorithms:
 *   • Point Clipping
 *   • Cohen-Sutherland Line Clipping
 *   • Liang-Barsky Line Clipping
 *   • Sutherland-Hodgman Polygon Clipping
 */

#pragma once
#include "../Common.h"
#include <vector>

// ─────────────────────────────────────────────────────────────────────────────
//  Point Clipping
//  Returns true if the point lies strictly inside (or on) the clip window.
// ─────────────────────────────────────────────────────────────────────────────
bool pointClip(float x, float y, const ClipWindow& win);

// ─────────────────────────────────────────────────────────────────────────────
//  Cohen-Sutherland Line Clipping
//  Assigns a 4-bit outcode to each endpoint (LEFT|RIGHT|BOTTOM|TOP).
//  Trivially accepts, trivially rejects, or clips iteratively until the line
//  is inside or fully outside the window.
//
//  Returns:
//    true  – a visible portion exists (x1..y2 updated to clipped segment)
//    false – line is completely outside the clip window
// ─────────────────────────────────────────────────────────────────────────────
bool cohenSutherlandClip(float& x1, float& y1, float& x2, float& y2,
                          const ClipWindow& win);

// ─────────────────────────────────────────────────────────────────────────────
//  Liang-Barsky Line Clipping
//  Represents the line as P(t) = P0 + t*(P1-P0) and solves four inequalities
//  for t (one per clip edge).  Finds the entry (tMin) and exit (tMax)
//  parameters using the p/q formulation.
//
//  Returns:
//    true  – visible portion found; x1..y2 updated
//    false – line is completely outside
// ─────────────────────────────────────────────────────────────────────────────
bool liangBarskyClip(float& x1, float& y1, float& x2, float& y2,
                      const ClipWindow& win);

// ─────────────────────────────────────────────────────────────────────────────
//  Sutherland-Hodgman Polygon Clipping
//  Clips a polygon against each of the four edges of the clip window in turn.
//  For each clip edge, applies the inside/outside/intersect test to successive
//  vertex pairs, building an output list.
//
//  Returns the clipped polygon (may be empty if fully outside).
// ─────────────────────────────────────────────────────────────────────────────
std::vector<Point2D> sutherlandHodgmanClip(
    const std::vector<Point2D>& polygon,
    const ClipWindow& win);

// ─────────────────────────────────────────────────────────────────────────────
//  Helper: compute intersection of segment (p1-p2) with an infinite clip edge
// ─────────────────────────────────────────────────────────────────────────────
Point2D intersectEdge(const Point2D& p1, const Point2D& p2,
                       int edge, const ClipWindow& win);

// Cohen-Sutherland outcodes
enum OutCode { INSIDE=0, LEFT=1, RIGHT=2, BOTTOM=4, TOP=8 };
int computeOutCode(float x, float y, const ClipWindow& win);
