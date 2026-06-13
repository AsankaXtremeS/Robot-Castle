/**
 * Projection.cpp
 * Isometric projection matrix – projects 3-D world coordinates to 2-D screen.
 *
 * Standard isometric projection:
 *   screenX = (x - z) * cos(30°) * scale  + originX
 *   screenY = (x + z) * sin(30°) * scale  - y * scale + originY
 *
 * This can be written as a 2×3 matrix acting on [x,y,z]:
 *   [ cos30    0   -cos30 ] [x]   [screenX - originX]
 *   [ sin30   -1    sin30 ] [y] = [screenY - originY]
 *                           [z]
 */

#include "../include/Algorithms/Projection.h"
#include <GL/freeglut.h>
#include <cmath>

static const float COS30 = std::cos(toRad(30.0f)); // ≈ 0.866
static const float SIN30 = std::sin(toRad(30.0f)); // ≈ 0.500

// =============================================================================
//  PROJECT A SINGLE 3-D POINT
// =============================================================================
Point2D IsoProjection::project(const Point3D& p) const
{
    // Apply isometric projection matrix
    float sx = (p.x - p.z) * COS30 * scale + originX;
    float sy = (p.x + p.z) * SIN30 * scale - p.y * scale + originY;
    return Point2D(sx, sy);
}

// =============================================================================
//  PROJECT A LIST OF POINTS
// =============================================================================
std::vector<Point2D> IsoProjection::projectAll(
    const std::vector<Point3D>& pts) const
{
    std::vector<Point2D> out;
    out.reserve(pts.size());
    for (auto& p : pts) out.push_back(project(p));
    return out;
}

// =============================================================================
//  DRAW AN ISOMETRIC BOX
//  Three visible faces: top, left, right
// =============================================================================
void drawIsoBox(const IsoProjection& proj,
                float x, float y, float z,
                float w, float h, float d,
                const Color& topCol, const Color& leftCol,
                const Color& rightCol, float progress)
{
    // 8 corners of the box
    Point3D corners[8] = {
        {x,   y,   z  },   // 0 front-bottom-left
        {x+w, y,   z  },   // 1 front-bottom-right
        {x+w, y,   z+d},   // 2 back-bottom-right
        {x,   y,   z+d},   // 3 back-bottom-left
        {x,   y+h, z  },   // 4 front-top-left
        {x+w, y+h, z  },   // 5 front-top-right
        {x+w, y+h, z+d},   // 6 back-top-right
        {x,   y+h, z+d},   // 7 back-top-left
    };

    // Project to screen
    Point2D s[8];
    for (int i = 0; i < 8; ++i) s[i] = proj.project(corners[i]);

    // Helper to draw a face
    auto drawFace = [&](int i0, int i1, int i2, int i3, const Color& col, float p) {
        if (p <= 0.0f) return;
        std::vector<Point2D> verts = { s[i0], s[i1], s[i2], s[i3] };
        glColor4f(col.r, col.g, col.b, col.a);
        glBegin(GL_POLYGON);
        for (auto& v : verts) glVertex2f(v.x, v.y);
        glEnd();
        // Outline
        glColor4f(0,0,0,0.5f);
        glBegin(GL_LINE_LOOP);
        for (auto& v : verts) glVertex2f(v.x, v.y);
        glEnd();
    };

    float p1 = clamp(progress * 3.0f,       0.0f, 1.0f); // top face
    float p2 = clamp(progress * 3.0f - 1.0f, 0.0f, 1.0f); // left face
    float p3 = clamp(progress * 3.0f - 2.0f, 0.0f, 1.0f); // right face

    if (p1 > 0) drawFace(4, 5, 6, 7, topCol,   p1); // top
    if (p2 > 0) drawFace(0, 4, 7, 3, leftCol,  p2); // left
    if (p3 > 0) drawFace(1, 5, 6, 2, rightCol, p3); // right
}

// =============================================================================
//  DRAW AN ISOMETRIC TILE GRID
// =============================================================================
void drawIsoGrid(const IsoProjection& proj,
                 int cols, int rows,
                 const Color& tileA, const Color& tileB,
                 float progress)
{
    int total = cols * rows;
    int draw  = static_cast<int>(total * clamp(progress, 0.0f, 1.0f));

    int count = 0;
    for (int r = 0; r < rows && count < draw; ++r) {
        for (int c = 0; c < cols && count < draw; ++c, ++count) {
            const Color& col = ((r + c) % 2 == 0) ? tileA : tileB;

            // Tile corners in world space (flat ground, y=0)
            Point2D s[4] = {
                proj.project({(float)c,   0, (float)r  }),
                proj.project({(float)c+1, 0, (float)r  }),
                proj.project({(float)c+1, 0, (float)r+1}),
                proj.project({(float)c,   0, (float)r+1})
            };

            glColor4f(col.r, col.g, col.b, col.a);
            glBegin(GL_POLYGON);
            for (auto& v : s) glVertex2f(v.x, v.y);
            glEnd();

            glColor4f(0,0,0,0.3f);
            glBegin(GL_LINE_LOOP);
            for (auto& v : s) glVertex2f(v.x, v.y);
            glEnd();
        }
    }
}
