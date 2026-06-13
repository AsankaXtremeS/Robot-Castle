/**
 * PaintersAlgorithm.h
 * Painter's Algorithm for depth-sorting and back-to-front rendering.
 *
 * The Painter's Algorithm renders objects from furthest to nearest (like a
 * painter covering the canvas), so closer objects naturally occlude farther
 * ones without needing a z-buffer.
 */

#pragma once
#include "../Common.h"
#include <vector>
#include <functional>
#include <string>

// ─────────────────────────────────────────────────────────────────────────────
//  A renderable layer with a depth value and a draw callback.
// ─────────────────────────────────────────────────────────────────────────────
struct PaintLayer {
    float                    depth;    // higher depth = further away (drawn first)
    std::string              name;     // label for debug/overlay display
    Color                    avgColor; // representative colour (for label)
    std::function<void()>    drawFn;   // callback that renders this layer

    PaintLayer() : depth(0) {}
    PaintLayer(float d, const std::string& n,
               const Color& col, std::function<void()> fn)
        : depth(d), name(n), avgColor(col), drawFn(fn) {}
};

// ─────────────────────────────────────────────────────────────────────────────
//  Sort layers by depth descending (furthest first) – in-place.
//  This is the core of the Painter's Algorithm: Z-sort before rasterisation.
// ─────────────────────────────────────────────────────────────────────────────
void sortByDepth(std::vector<PaintLayer>& layers);

// ─────────────────────────────────────────────────────────────────────────────
//  Render all layers in sorted order (back-to-front).
//  Optionally draws a depth label above each layer for educational display.
// ─────────────────────────────────────────────────────────────────────────────
void renderSorted(std::vector<PaintLayer>& layers,
                  bool showDepthLabels = false);

// ─────────────────────────────────────────────────────────────────────────────
//  Parallax scrolling helper – offsets each layer horizontally by
//  (depth / maxDepth) * scrollAmount, giving a convincing depth illusion.
// ─────────────────────────────────────────────────────────────────────────────
void applyParallax(std::vector<PaintLayer>& layers,
                   float scrollAmount, float maxDepth);
