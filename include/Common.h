/**
 * Common.h
 * Shared data types, constants, and utility structures used across all
 * modules of "The Little Robot Builds a Digital Kingdom".
 */

#pragma once

#include <vector>
#include <cmath>
#include <string>
#include <algorithm>

// ─────────────────────────────────────────────
//  Window / rendering constants
// ─────────────────────────────────────────────
constexpr int   WINDOW_WIDTH  = 1280;
constexpr int   WINDOW_HEIGHT = 720;
constexpr int   TARGET_FPS    = 60;
constexpr float FRAME_TIME    = 1000.0f / TARGET_FPS; // ms

// ─────────────────────────────────────────────
//  Color (RGBA, 0–1 floats)
// ─────────────────────────────────────────────
struct Color {
    float r, g, b, a;
    Color(float r = 0, float g = 0, float b = 0, float a = 1.0f)
        : r(r), g(g), b(b), a(a) {}

    // Linear interpolation between two colors
    static Color lerp(const Color& c1, const Color& c2, float t) {
        return Color(
            c1.r + (c2.r - c1.r) * t,
            c1.g + (c2.g - c1.g) * t,
            c1.b + (c2.b - c1.b) * t,
            c1.a + (c2.a - c1.a) * t
        );
    }
};

// ─────────────────────────────────────────────
//  2-D point (integer pixels)
// ─────────────────────────────────────────────
struct Point2D {
    float x, y;
    Point2D(float x = 0, float y = 0) : x(x), y(y) {}
};

// ─────────────────────────────────────────────
//  3-D point (used before isometric projection)
// ─────────────────────────────────────────────
struct Point3D {
    float x, y, z;
    Point3D(float x = 0, float y = 0, float z = 0) : x(x), y(y), z(z) {}
};

// ─────────────────────────────────────────────
//  Polygon: ordered list of 2-D vertices
// ─────────────────────────────────────────────
struct Polygon2D {
    std::vector<Point2D> vertices;
    Color                fillColor;
    Color                borderColor;
    float                depth; // z-value for Painter's Algorithm

    Polygon2D() : fillColor(1,1,1), borderColor(0,0,0), depth(0) {}
};

// ─────────────────────────────────────────────
//  Axis-aligned clip window
// ─────────────────────────────────────────────
struct ClipWindow {
    float xMin, yMin, xMax, yMax;
    ClipWindow(float x0 = 0, float y0 = 0,
               float x1 = WINDOW_WIDTH, float y1 = WINDOW_HEIGHT)
        : xMin(x0), yMin(y0), xMax(x1), yMax(y1) {}
};

// ─────────────────────────────────────────────
//  3 × 3 Transformation matrix (row-major)
//  Indices: m[row][col]
// ─────────────────────────────────────────────
struct Matrix3x3 {
    float m[3][3];

    // Identity by default
    Matrix3x3() {
        for (int r = 0; r < 3; ++r)
            for (int c = 0; c < 3; ++c)
                m[r][c] = (r == c) ? 1.0f : 0.0f;
    }

    // Matrix × Matrix
    Matrix3x3 operator*(const Matrix3x3& other) const {
        Matrix3x3 result;
        for (int r = 0; r < 3; ++r)
            for (int c = 0; c < 3; ++c) {
                result.m[r][c] = 0;
                for (int k = 0; k < 3; ++k)
                    result.m[r][c] += m[r][k] * other.m[k][c];
            }
        return result;
    }

    // Transform a 2-D point (homogeneous coordinates)
    Point2D transform(const Point2D& p) const {
        float x = m[0][0] * p.x + m[0][1] * p.y + m[0][2];
        float y = m[1][0] * p.x + m[1][1] * p.y + m[1][2];
        float w = m[2][0] * p.x + m[2][1] * p.y + m[2][2];
        if (w != 0.0f) { x /= w; y /= w; }
        return Point2D(x, y);
    }
};

// ─────────────────────────────────────────────
//  Colour palette used through-out
// ─────────────────────────────────────────────
namespace Palette {
    // Sky & background
    inline Color skyTop()       { return {0.05f, 0.05f, 0.20f}; }
    inline Color skyBottom()    { return {0.20f, 0.45f, 0.85f}; }

    // Kingdom elements
    inline Color castleGray()   { return {0.55f, 0.55f, 0.60f}; }
    inline Color roofRed()      { return {0.85f, 0.20f, 0.20f}; }
    inline Color grass()        { return {0.20f, 0.75f, 0.30f}; }
    inline Color road()         { return {0.45f, 0.42f, 0.40f}; }
    inline Color water()        { return {0.20f, 0.55f, 0.95f}; }
    inline Color goldAccent()   { return {1.0f,  0.80f, 0.10f}; }
    inline Color woodBrown()    { return {0.55f, 0.35f, 0.15f}; }
    inline Color leafGreen()    { return {0.15f, 0.65f, 0.25f}; }
    inline Color windowBlue()   { return {0.50f, 0.80f, 1.0f};  }
    inline Color pinkFlower()   { return {1.0f,  0.55f, 0.75f}; }

    // Robot colours
    inline Color robotBody()    { return {0.30f, 0.60f, 0.90f}; }
    inline Color robotJoint()   { return {0.70f, 0.75f, 0.85f}; }
    inline Color robotEye()     { return {0.10f, 0.90f, 0.80f}; }

    // UI overlay
    inline Color overlayDark()  { return {0.0f,  0.0f,  0.0f,  0.60f}; }
    inline Color white()        { return {1.0f,  1.0f,  1.0f}; }
    inline Color yellow()       { return {1.0f,  0.95f, 0.20f}; }
}

// ─────────────────────────────────────────────
//  Simple PI constant
// ─────────────────────────────────────────────
constexpr float PI = 3.14159265358979323846f;

// ─────────────────────────────────────────────
//  Clamp helper
// ─────────────────────────────────────────────
inline float clamp(float v, float lo, float hi) {
    return std::max(lo, std::min(hi, v));
}

// ─────────────────────────────────────────────
//  Degree ↔ Radian helpers
// ─────────────────────────────────────────────
inline float toRad(float deg) { return deg * PI / 180.0f; }
inline float toDeg(float rad) { return rad * 180.0f / PI; }
