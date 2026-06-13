/**
 * Renderer.h
 * Low-level OpenGL rendering helpers used by all scenes.
 * Wraps common drawing operations in a clean API.
 */

#pragma once
#include "Common.h"
#include <string>
#include <vector>

namespace Renderer {

// ── Colour helper ────────────────────────────────────────────────────────────
void setColor(const Color& c);

// ── Gradient background ──────────────────────────────────────────────────────
void drawGradientBackground(const Color& top, const Color& bottom);

// ── Basic primitives ─────────────────────────────────────────────────────────
void drawPoint(float x, float y, float size, const Color& c);
void drawLine(float x1, float y1, float x2, float y2, const Color& c, float width = 1.0f);
void drawRect(float x, float y, float w, float h, const Color& c);
void drawFilledRect(float x, float y, float w, float h, const Color& c);
void drawCircle(float cx, float cy, float r, const Color& c, int segments = 64);
void drawFilledCircle(float cx, float cy, float r, const Color& c, int segments = 64);
void drawTriangle(float x1,float y1, float x2,float y2, float x3,float y3, const Color& c);
void drawFilledTriangle(float x1,float y1, float x2,float y2, float x3,float y3, const Color& c);
void drawPolygon(const std::vector<Point2D>& verts, const Color& c);
void drawFilledPolygon(const std::vector<Point2D>& verts, const Color& c);

// ── Text rendering (GLUT bitmap font) ────────────────────────────────────────
void drawText(const std::string& text, float x, float y,
              const Color& c, float scale = 1.0f);
void drawLargeText(const std::string& text, float x, float y, const Color& c);
void drawCentredText(const std::string& text, float y, const Color& c, float scale = 1.0f);

// ── HUD overlay panels ───────────────────────────────────────────────────────
void drawAlgorithmBadge(const std::string& algoName, float alpha = 1.0f);
void drawSceneTitle(const std::string& title, const std::string& sub,
                    float alpha = 1.0f);
void drawProgressBar(float x, float y, float w, float h,
                     float progress, const Color& fillCol, const Color& bgCol);

// ── Clipping window visualiser ───────────────────────────────────────────────
void drawClipWindow(const ClipWindow& win, const Color& c);

// ── Particle / firework ───────────────────────────────────────────────────────
struct Particle {
    float x, y, vx, vy, life, maxLife;
    Color col;
    float size;
};

void updateParticles(std::vector<Particle>& parts, float dt);
void drawParticles(const std::vector<Particle>& parts);
void spawnFirework(std::vector<Particle>& parts, float x, float y);

// ── Transformation push/pop helpers ─────────────────────────────────────────
void pushTransform(float tx, float ty, float angleDeg = 0, float sx = 1, float sy = 1);
void popTransform();

} // namespace Renderer
