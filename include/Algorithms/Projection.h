/**
 * Projection.h
 * Isometric Projection Matrix for 2.5D rendering.
 *
 * Converts 3-D world coordinates (x,y,z) into 2-D screen coordinates using
 * the standard isometric projection matrix.  The classic isometric view uses
 * 30° angles from the horizontal for depth axes.
 *
 * Standard matrix (simplified):
 *   screenX =  (x - z) * cos(30°)
 *   screenY =  (x + z) * sin(30°) - y       (y is vertical/height)
 */

#pragma once
#include "../Common.h"
#include <vector>

// ─────────────────────────────────────────────────────────────────────────────
//  Build the isometric projection matrix (3×3, maps 3-D → 2-D homogeneous)
//  The third column handles the z contribution.
//  We store this as two separate 1×3 row vectors for x and y output.
// ─────────────────────────────────────────────────────────────────────────────

struct IsoProjection {
    float scale;      // pixels per unit
    float originX;    // screen origin x
    float originY;    // screen origin y

    IsoProjection(float scale = 60.0f,
                  float ox = WINDOW_WIDTH  / 2.0f,
                  float oy = WINDOW_HEIGHT / 2.0f)
        : scale(scale), originX(ox), originY(oy) {}

    /** Project a 3-D point into 2-D screen space */
    Point2D project(const Point3D& p) const;

    /** Project a list of 3-D points */
    std::vector<Point2D> projectAll(const std::vector<Point3D>& pts) const;
};

// ─────────────────────────────────────────────────────────────────────────────
//  Draw a 3-D box (cube / rectangular prism) in isometric view.
//  x,y,z = world position of the bottom-front-left corner.
//  w,h,d = width (x), height (y/up), depth (z).
// ─────────────────────────────────────────────────────────────────────────────
void drawIsoBox(const IsoProjection& proj,
                float x, float y, float z,
                float w, float h, float d,
                const Color& topCol, const Color& leftCol,
                const Color& rightCol, float progress = 1.0f);

// ─────────────────────────────────────────────────────────────────────────────
//  Draw an entire isometric tile grid (ground plane)
// ─────────────────────────────────────────────────────────────────────────────
void drawIsoGrid(const IsoProjection& proj,
                 int cols, int rows,
                 const Color& tileA, const Color& tileB,
                 float progress = 1.0f);
