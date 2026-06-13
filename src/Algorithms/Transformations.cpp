/**
 * Transformations.cpp
 * 3×3 homogeneous matrix implementations for 2-D transformations.
 */

#include "../include/Algorithms/Transformations.h"
#include <cmath>

// =============================================================================
//  TRANSLATION MATRIX
//  ────────────────────
//  Homogeneous form:
//    | 1  0  tx |
//    | 0  1  ty |
//    | 0  0   1 |
//  Multiplying this by a column vector [x, y, 1]ᵀ gives [x+tx, y+ty, 1]ᵀ
// =============================================================================
Matrix3x3 translationMatrix(float tx, float ty)
{
    Matrix3x3 m;
    // Identity initialised by constructor
    m.m[0][2] = tx;
    m.m[1][2] = ty;
    return m;
}

// =============================================================================
//  ROTATION MATRIX
//  ────────────────
//  Counter-clockwise rotation by 'angleDeg' degrees around the origin:
//    | cos(θ) -sin(θ)  0 |
//    | sin(θ)  cos(θ)  0 |
//    |   0       0     1 |
// =============================================================================
Matrix3x3 rotationMatrix(float angleDeg)
{
    float theta = toRad(angleDeg);
    float c = std::cos(theta);
    float s = std::sin(theta);

    Matrix3x3 m;
    m.m[0][0] =  c;  m.m[0][1] = -s;
    m.m[1][0] =  s;  m.m[1][1] =  c;
    return m;
}

// =============================================================================
//  ROTATION AROUND A PIVOT POINT
//  ──────────────────────────────
//  Composite: T(px,py) · R(θ) · T(-px,-py)
//  Steps:
//    1. Translate so pivot is at origin
//    2. Rotate
//    3. Translate back
// =============================================================================
Matrix3x3 rotationAroundPoint(float angleDeg, float px, float py)
{
    return compositeTransform({
        translationMatrix(-px, -py),
        rotationMatrix(angleDeg),
        translationMatrix( px,  py)
    });
}

// =============================================================================
//  SCALING MATRIX
//  ───────────────
//    | sx  0   0 |
//    |  0  sy  0 |
//    |  0  0   1 |
// =============================================================================
Matrix3x3 scalingMatrix(float sx, float sy)
{
    Matrix3x3 m;
    m.m[0][0] = sx;
    m.m[1][1] = sy;
    return m;
}

// =============================================================================
//  SCALING AROUND A PIVOT POINT
// =============================================================================
Matrix3x3 scalingAroundPoint(float sx, float sy, float px, float py)
{
    return compositeTransform({
        translationMatrix(-px, -py),
        scalingMatrix(sx, sy),
        translationMatrix( px,  py)
    });
}

// =============================================================================
//  COMPOSITE TRANSFORMATION
//  ─────────────────────────
//  Multiplies matrices left-to-right.  The effective transform applies them
//  in the order they appear in the vector (first element applied first).
//
//  C = M[0] · M[1] · M[2] · …
//
//  When applied to a point p:  p' = C · p
//    = M[0] · (M[1] · (M[2] · p))   ← right-associative matrix product
// =============================================================================
Matrix3x3 compositeTransform(const std::vector<Matrix3x3>& matrices)
{
    if (matrices.empty()) return Matrix3x3(); // identity

    Matrix3x3 result = matrices[0];
    for (int i = 1; i < (int)matrices.size(); ++i)
        result = result * matrices[i];

    return result;
}

// =============================================================================
//  TRANSFORM A POLYGON
// =============================================================================
Polygon2D transformPolygon(const Polygon2D& poly, const Matrix3x3& mat)
{
    Polygon2D result = poly;
    for (auto& v : result.vertices)
        v = mat.transform(v);
    return result;
}

// =============================================================================
//  TRANSFORM A LIST OF POINTS
// =============================================================================
std::vector<Point2D> transformPoints(const std::vector<Point2D>& pts,
                                      const Matrix3x3& mat)
{
    std::vector<Point2D> out;
    out.reserve(pts.size());
    for (auto& p : pts)
        out.push_back(mat.transform(p));
    return out;
}
