/**
 * Renderer.cpp
 * Low-level OpenGL rendering helpers.
 */

#include "../include/Renderer.h"
#include <GL/freeglut.h>
#include <cmath>
#include <algorithm>
#include <cstdlib>

namespace Renderer {

// ─────────────────────────────────────────────────────────────────────────────
void setColor(const Color& c) {
    glColor4f(c.r, c.g, c.b, c.a);
}

// ─────────────────────────────────────────────────────────────────────────────
void drawGradientBackground(const Color& top, const Color& bottom)
{
    glBegin(GL_QUADS);
    setColor(bottom);
    glVertex2f(0,              0);
    glVertex2f(WINDOW_WIDTH,   0);
    setColor(top);
    glVertex2f(WINDOW_WIDTH,   WINDOW_HEIGHT);
    glVertex2f(0,              WINDOW_HEIGHT);
    glEnd();
}

// ─────────────────────────────────────────────────────────────────────────────
void drawPoint(float x, float y, float size, const Color& c) {
    glPointSize(size);
    glBegin(GL_POINTS);
    setColor(c);
    glVertex2f(x, y);
    glEnd();
    glPointSize(1.0f);
}

// ─────────────────────────────────────────────────────────────────────────────
void drawLine(float x1, float y1, float x2, float y2,
              const Color& c, float width)
{
    glLineWidth(width);
    glBegin(GL_LINES);
    setColor(c);
    glVertex2f(x1, y1);
    glVertex2f(x2, y2);
    glEnd();
    glLineWidth(1.0f);
}

// ─────────────────────────────────────────────────────────────────────────────
void drawRect(float x, float y, float w, float h, const Color& c) {
    glBegin(GL_LINE_LOOP);
    setColor(c);
    glVertex2f(x,     y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x,     y + h);
    glEnd();
}

// ─────────────────────────────────────────────────────────────────────────────
void drawFilledRect(float x, float y, float w, float h, const Color& c) {
    glBegin(GL_QUADS);
    setColor(c);
    glVertex2f(x,     y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x,     y + h);
    glEnd();
}

// ─────────────────────────────────────────────────────────────────────────────
void drawCircle(float cx, float cy, float r, const Color& c, int segments) {
    glBegin(GL_LINE_LOOP);
    setColor(c);
    for (int i = 0; i < segments; ++i) {
        float a = 2.0f * PI * i / segments;
        glVertex2f(cx + r * std::cos(a), cy + r * std::sin(a));
    }
    glEnd();
}

// ─────────────────────────────────────────────────────────────────────────────
void drawFilledCircle(float cx, float cy, float r, const Color& c, int segments) {
    glBegin(GL_TRIANGLE_FAN);
    setColor(c);
    glVertex2f(cx, cy);
    for (int i = 0; i <= segments; ++i) {
        float a = 2.0f * PI * i / segments;
        glVertex2f(cx + r * std::cos(a), cy + r * std::sin(a));
    }
    glEnd();
}

// ─────────────────────────────────────────────────────────────────────────────
void drawTriangle(float x1,float y1, float x2,float y2, float x3,float y3,
                  const Color& c) {
    glBegin(GL_LINE_LOOP);
    setColor(c);
    glVertex2f(x1,y1); glVertex2f(x2,y2); glVertex2f(x3,y3);
    glEnd();
}

// ─────────────────────────────────────────────────────────────────────────────
void drawFilledTriangle(float x1,float y1, float x2,float y2,
                        float x3,float y3, const Color& c) {
    glBegin(GL_TRIANGLES);
    setColor(c);
    glVertex2f(x1,y1); glVertex2f(x2,y2); glVertex2f(x3,y3);
    glEnd();
}

// ─────────────────────────────────────────────────────────────────────────────
void drawPolygon(const std::vector<Point2D>& verts, const Color& c) {
    glBegin(GL_LINE_LOOP);
    setColor(c);
    for (auto& v : verts) glVertex2f(v.x, v.y);
    glEnd();
}

// ─────────────────────────────────────────────────────────────────────────────
void drawFilledPolygon(const std::vector<Point2D>& verts, const Color& c) {
    glBegin(GL_POLYGON);
    setColor(c);
    for (auto& v : verts) glVertex2f(v.x, v.y);
    glEnd();
}

// ─────────────────────────────────────────────────────────────────────────────
//  Text rendering helpers
// ─────────────────────────────────────────────────────────────────────────────
void drawText(const std::string& text, float x, float y,
              const Color& c, float /*scale*/)
{
    setColor(c);
    glRasterPos2f(x, y);
    for (char ch : text)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, ch);
}

void drawLargeText(const std::string& text, float x, float y, const Color& c)
{
    setColor(c);
    glRasterPos2f(x, y);
    for (char ch : text)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, ch);
}

void drawCentredText(const std::string& text, float y,
                     const Color& c, float /*scale*/)
{
    // Estimate pixel width
    int w = glutBitmapLength(GLUT_BITMAP_HELVETICA_18,
                             (const unsigned char*)text.c_str());
    float x = (WINDOW_WIDTH - w) * 0.5f;
    drawLargeText(text, x, y, c);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Algorithm badge – shown in the corner to label which algorithm is active
// ─────────────────────────────────────────────────────────────────────────────
void drawAlgorithmBadge(const std::string& algoName, float alpha)
{
    float bx = 10, by = WINDOW_HEIGHT - 40;
    float bw = 340, bh = 30;

    // Dark background panel
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glColor4f(0.0f, 0.0f, 0.0f, 0.6f * alpha);
    glBegin(GL_QUADS);
    glVertex2f(bx, by); glVertex2f(bx+bw, by);
    glVertex2f(bx+bw, by+bh); glVertex2f(bx, by+bh);
    glEnd();

    // Accent left bar
    glColor4f(0.3f, 0.8f, 1.0f, alpha);
    glBegin(GL_QUADS);
    glVertex2f(bx, by); glVertex2f(bx+4, by);
    glVertex2f(bx+4, by+bh); glVertex2f(bx, by+bh);
    glEnd();

    // Label
    drawLargeText("  " + algoName, bx + 8, by + 8,
                  Color(0.3f, 0.9f, 1.0f, alpha));
}

// ─────────────────────────────────────────────────────────────────────────────
void drawSceneTitle(const std::string& title, const std::string& sub,
                    float alpha)
{
    // Shadow
    drawCentredText(title, WINDOW_HEIGHT - 80,
                    Color(0,0,0, 0.5f * alpha));
    // Main title
    drawCentredText(title, WINDOW_HEIGHT - 82,
                    Color(1.0f, 0.95f, 0.2f, alpha));
    // Subtitle
    drawCentredText(sub,   WINDOW_HEIGHT - 60,
                    Color(0.8f, 0.8f, 1.0f, alpha));
}

// ─────────────────────────────────────────────────────────────────────────────
void drawProgressBar(float x, float y, float w, float h,
                     float progress, const Color& fillCol, const Color& bgCol)
{
    drawFilledRect(x, y, w, h, bgCol);
    drawFilledRect(x, y, w * clamp(progress, 0.0f, 1.0f), h, fillCol);
    drawRect(x, y, w, h, Color(1,1,1,0.5f));
}

// ─────────────────────────────────────────────────────────────────────────────
void drawClipWindow(const ClipWindow& win, const Color& c)
{
    glLineWidth(2.0f);
    glLineStipple(1, 0xF0F0); // dashed line
    glEnable(GL_LINE_STIPPLE);
    drawRect(win.xMin, win.yMin,
             win.xMax - win.xMin, win.yMax - win.yMin, c);
    glDisable(GL_LINE_STIPPLE);
    glLineWidth(1.0f);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Particles / fireworks
// ─────────────────────────────────────────────────────────────────────────────
void updateParticles(std::vector<Particle>& parts, float dt)
{
    for (auto& p : parts) {
        p.x   += p.vx * dt;
        p.y   += p.vy * dt;
        p.vy  -= 180.0f * dt;  // gravity
        p.life -= dt;
    }
    // Remove dead particles
    parts.erase(std::remove_if(parts.begin(), parts.end(),
        [](const Particle& p){ return p.life <= 0; }), parts.end());
}

void drawParticles(const std::vector<Particle>& parts)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    for (auto& p : parts) {
        float a = p.life / p.maxLife;
        glPointSize(p.size * a + 1.0f);
        glBegin(GL_POINTS);
        glColor4f(p.col.r, p.col.g, p.col.b, a);
        glVertex2f(p.x, p.y);
        glEnd();
    }
    glPointSize(1.0f);
}

void spawnFirework(std::vector<Particle>& parts, float x, float y)
{
    static const Color cols[] = {
        {1.0f,0.2f,0.2f}, {1.0f,0.9f,0.1f}, {0.2f,0.8f,1.0f},
        {0.8f,0.2f,1.0f}, {0.2f,1.0f,0.4f}, {1.0f,0.5f,0.1f}
    };

    for (int i = 0; i < 60; ++i) {
        float angle  = 2.0f * PI * i / 60.0f + ((float)rand()/RAND_MAX)*0.3f;
        float speed  = 80.0f + (float)rand()/RAND_MAX * 120.0f;
        Particle p;
        p.x      = x;   p.y      = y;
        p.vx     = std::cos(angle) * speed;
        p.vy     = std::sin(angle) * speed;
        p.maxLife = p.life = 1.5f + (float)rand()/RAND_MAX * 0.8f;
        p.col    = cols[rand() % 6];
        p.size   = 2.0f + (float)rand()/RAND_MAX * 2.0f;
        parts.push_back(p);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
void pushTransform(float tx, float ty, float angleDeg, float sx, float sy)
{
    glPushMatrix();
    glTranslatef(tx, ty, 0);
    glRotatef(angleDeg, 0, 0, 1);
    glScalef(sx, sy, 1);
}

void popTransform()
{
    glPopMatrix();
}

} // namespace Renderer
