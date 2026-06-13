/**
 * Transformations.h
 * From-scratch 3×3 homogeneous transformation matrices for 2-D:
 *   • Translation
 *   • Rotation (about the origin or a given pivot)
 *   • Scaling  (uniform or non-uniform)
 *   • Composite (sequential matrix multiplication)
 */

#pragma once
#include "../Common.h"
#include <vector>

// ─────────────────────────────────────────────────────────────────────────────
//  Build elementary matrices
// ─────────────────────────────────────────────────────────────────────────────

/** Translation matrix: moves a point by (tx, ty) */
Matrix3x3 translationMatrix(float tx, float ty);

/** Rotation matrix: rotates counter-clockwise by 'angleDeg' degrees
 *  around the origin. */
Matrix3x3 rotationMatrix(float angleDeg);

/** Rotation around an arbitrary pivot point (px, py).
 *  Implemented as T(pivot) · R(angle) · T(-pivot) */
Matrix3x3 rotationAroundPoint(float angleDeg, float px, float py);

/** Uniform scaling matrix (sx == sy) */
Matrix3x3 scalingMatrix(float sx, float sy);

/** Scaling around an arbitrary pivot point */
Matrix3x3 scalingAroundPoint(float sx, float sy, float px, float py);

// ─────────────────────────────────────────────────────────────────────────────
//  Composite transformation
//  Multiplies a list of matrices left-to-right, so the transforms are applied
//  in the order they appear in the vector.
// ─────────────────────────────────────────────────────────────────────────────
Matrix3x3 compositeTransform(const std::vector<Matrix3x3>& matrices);

// ─────────────────────────────────────────────────────────────────────────────
//  Apply a transformation matrix to a polygon's vertices
// ─────────────────────────────────────────────────────────────────────────────
Polygon2D transformPolygon(const Polygon2D& poly, const Matrix3x3& mat);

/** Transform a list of Point2D */
std::vector<Point2D> transformPoints(const std::vector<Point2D>& pts,
                                      const Matrix3x3& mat);
