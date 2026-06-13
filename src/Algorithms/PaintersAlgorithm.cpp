/**
 * PaintersAlgorithm.cpp
 * Depth sorting (Painter's Algorithm) and parallax scrolling.
 */

#include "../include/Algorithms/PaintersAlgorithm.h"
#include "../include/Renderer.h"
#include <GL/freeglut.h>
#include <algorithm>

// =============================================================================
//  SORT BY DEPTH  (back-to-front = descending depth value)
//  ──────────────
//  The Painter's Algorithm gets its name from the analogy of an oil painter
//  who paints the distant background first, then progressively closer objects
//  on top.  We achieve this by sorting layers so the highest depth value
//  (furthest) is rendered first.
// =============================================================================
void sortByDepth(std::vector<PaintLayer>& layers)
{
    // std::stable_sort preserves order for layers at equal depth
    std::stable_sort(layers.begin(), layers.end(),
        [](const PaintLayer& a, const PaintLayer& b) {
            return a.depth > b.depth; // further = larger depth → draw first
        });
}

// =============================================================================
//  RENDER ALL LAYERS IN SORTED ORDER
// =============================================================================
void renderSorted(std::vector<PaintLayer>& layers, bool showDepthLabels)
{
    sortByDepth(layers);

    for (auto& layer : layers) {
        if (layer.drawFn) layer.drawFn();

        // Optional educational depth label
        if (showDepthLabels && !layer.name.empty()) {
            // Draw a small badge above the layer indicating its depth
            // We use a fixed y position per depth value for clarity
            float labelY = 50.0f + layer.depth * 30.0f;
            Renderer::drawText("[" + layer.name + " d=" +
                               std::to_string((int)layer.depth) + "]",
                               10.0f, labelY, layer.avgColor, 1.0f);
        }
    }
}

// =============================================================================
//  PARALLAX SCROLLING
//  ───────────────────
//  Shifts each layer's draw position by an amount proportional to its depth.
//  Far layers move less (parallax effect simulating real depth perception).
//
//  This is applied by temporarily translating the OpenGL modelview matrix
//  before each layer's draw call.
// =============================================================================
void applyParallax(std::vector<PaintLayer>& layers,
                   float scrollAmount, float maxDepth)
{
    sortByDepth(layers);

    for (auto& layer : layers) {
        // Parallax factor: deeper layers move less
        float factor = (maxDepth - layer.depth) / maxDepth;
        float offset  = scrollAmount * factor;

        glPushMatrix();
        glTranslatef(offset, 0.0f, 0.0f);
        if (layer.drawFn) layer.drawFn();
        glPopMatrix();
    }
}
