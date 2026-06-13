/**
 * SceneManager.cpp
 * Orchestrates all 7 animated scenes, transitions, HUD, and timing.
 *
 * Scene durations (approximate):
 *   INTRO      – 5s   (robot walks in)
 *   DRAWING    – 20s  (DDA/Bresenham line & circle – roads, fountains)
 *   COLOURING  – 20s  (Scan-Line, Flood Fill, Boundary Fill)
 *   PORTAL     – 20s  (Point Clip, Cohen-Sutherland, Liang-Barsky, S-H)
 *   TRANSFORM  – 25s  (Translation, Rotation, Scaling, Composite)
 *   ISOMETRIC  – 25s  (Isometric Projection)
 *   CINEMATIC  – 25s  (Painter's Algorithm, Parallax, Fireworks)
 */

#include "../include/SceneManager.h"
#include "../include/Renderer.h"
#include "../include/Robot.h"
#include "../include/Algorithms/LineDrawing.h"
#include "../include/Algorithms/CircleDrawing.h"
#include "../include/Algorithms/Filling.h"
#include "../include/Algorithms/Clipping.h"
#include "../include/Algorithms/Transformations.h"
#include "../include/Algorithms/Projection.h"
#include "../include/Algorithms/PaintersAlgorithm.h"

#include <GL/freeglut.h>
#include <ctime>
#include <cmath>
#include <cstdlib>
#include <vector>
#include <string>
#include <sstream>

// ─────────────────────────────────────────────────────────────────────────────
//  Global state
// ─────────────────────────────────────────────────────────────────────────────
static Robot             g_robot;
static std::vector<Renderer::Particle> g_particles;
static float             g_scrollOffset = 0.0f;

// ─────────────────────────────────────────────────────────────────────────────
SceneManager& getSceneManager() {
    static SceneManager sm;
    return sm;
}

// ─────────────────────────────────────────────────────────────────────────────
SceneManager::SceneManager()
    : m_current(SceneID::INTRO),
      m_elapsed(0), m_totalElapsed(0),
      m_fadeAlpha(1.0f), m_fadeIn(true)
{
    m_scenes = {
        {SceneID::INTRO,     "The Little Robot Arrives",   "A digital world awakens",    5.0f },
        {SceneID::DRAWING,   "Scene 1 – Drawing Algorithms", "DDA & Bresenham Lines · Midpoint & Bresenham Circles", 20.0f},
        {SceneID::COLOURING, "Scene 2 – Filling Algorithms", "Scan-Line · Flood Fill · Boundary Fill",              20.0f},
        {SceneID::PORTAL,    "Scene 3 – Clipping Algorithms","Point · Cohen-Sutherland · Liang-Barsky · S-Hodgman", 20.0f},
        {SceneID::TRANSFORM, "Scene 4 – Transformations",    "Translation · Rotation · Scaling · Composite",        25.0f},
        {SceneID::CINEMATIC, "Scene 5 – Grand Finale",        "Painter's Algorithm · Parallax · Fireworks",         25.0f},
    };

    g_robot.x     = -80;
    g_robot.y     = 150;
    g_robot.scale = 1.0f;
}

// ─────────────────────────────────────────────────────────────────────────────
void SceneManager::update(float dtSec)
{
    m_elapsed      += dtSec;
    m_totalElapsed += dtSec;

    g_robot.update(dtSec);
    Renderer::updateParticles(g_particles, dtSec);
    g_scrollOffset += dtSec * 15.0f; // gentle parallax scroll

    // Fade transition
    if (m_fadeIn) {
        m_fadeAlpha -= dtSec * 2.5f;
        if (m_fadeAlpha <= 0) { m_fadeAlpha = 0; m_fadeIn = false; }
    }

    // Current scene duration
    float dur = m_scenes[static_cast<int>(m_current)].durationSec;
    if (m_elapsed >= dur) {
        advanceScene();
    }
}

// ─────────────────────────────────────────────────────────────────────────────
void SceneManager::advanceScene()
{
    int next = static_cast<int>(m_current) + 1;
    if (next >= static_cast<int>(SceneID::COUNT)) {
        // Loop back to cinematic
        next = static_cast<int>(SceneID::CINEMATIC);
    }
    m_current  = static_cast<SceneID>(next);
    m_elapsed  = 0;
    m_fadeAlpha = 1.0f;
    m_fadeIn    = true;
}

// ─────────────────────────────────────────────────────────────────────────────
float SceneManager::progress() const
{
    float dur = m_scenes[static_cast<int>(m_current)].durationSec;
    return clamp(m_elapsed / dur, 0.0f, 1.0f);
}

// ─────────────────────────────────────────────────────────────────────────────
void SceneManager::jumpTo(SceneID scene)
{
    m_current  = scene;
    m_elapsed  = 0;
    m_fadeAlpha = 1.0f;
    m_fadeIn    = true;
}

// =============================================================================
//  RENDER DISPATCH
// =============================================================================
void SceneManager::render()
{
    renderScene(m_current);
    drawHUD();
    drawFadeOverlay();
}

// ─────────────────────────────────────────────────────────────────────────────
void SceneManager::renderScene(SceneID id)
{
    switch (id) {
    case SceneID::INTRO:     renderIntro();     break;
    case SceneID::DRAWING:   renderDrawing();   break;
    case SceneID::COLOURING: renderColouring(); break;
    case SceneID::PORTAL:    renderPortal();    break;
    case SceneID::TRANSFORM: renderTransform(); break;
    case SceneID::CINEMATIC: renderCinematic(); break;
    default: break;
    }
}

// =============================================================================
//  SHARED HELPERS
// =============================================================================

void SceneManager::drawBackground(float /*t*/)
{
    // Day-time gradient sky
    Renderer::drawGradientBackground(
        Color(0.35f, 0.65f, 0.95f),   // top
        Color(0.70f, 0.88f, 1.00f)    // bottom (horizon)
    );
}

void SceneManager::drawGround()
{
    // Green ground strip
    Renderer::drawFilledRect(0, 0, WINDOW_WIDTH, 170, Palette::grass());
    // Ground shadow line
    Renderer::drawLine(0, 170, WINDOW_WIDTH, 170, Color(0.1f,0.5f,0.1f), 2.0f);

    // Tiled pavement strip
    Color pave1(0.62f,0.60f,0.55f), pave2(0.55f,0.52f,0.48f);
    for (int i = 0; i < WINDOW_WIDTH; i += 40) {
        Renderer::drawFilledRect((float)i, 130, 38, 40,
                                  (i/40)%2==0 ? pave1 : pave2);
    }
}

void SceneManager::drawStars(float density, float twinkle)
{
    srand(42);
    int count = (int)(density * 200);
    for (int i = 0; i < count; ++i) {
        float sx = (float)(rand() % WINDOW_WIDTH);
        float sy = (float)(rand() % WINDOW_HEIGHT);
        float bright = 0.6f + 0.4f * std::sin(twinkle + i);
        Renderer::drawPoint(sx, sy, 1.5f, Color(bright, bright, bright));
    }
    srand((unsigned)time(nullptr));
}

// ─────────────────────────────────────────────────────────────────────────────
//  HUD: algorithm badge + scene title fade-in
// ─────────────────────────────────────────────────────────────────────────────
void SceneManager::drawHUD()
{
    const SceneInfo& info = m_scenes[static_cast<int>(m_current)];

    // Title appears for first 3 seconds of each scene
    float titleAlpha = clamp(1.0f - (m_elapsed - 2.0f), 0.0f, 1.0f);
    if (titleAlpha > 0) {
        Renderer::drawSceneTitle(info.title, info.subtitle, titleAlpha);
    }

    // Algorithm badge – always visible
    Renderer::drawAlgorithmBadge(info.subtitle, 0.85f);

    // Scene timer / progress bar
    float dur = info.durationSec;
    Renderer::drawProgressBar(10, 10, 250, 6, m_elapsed / dur,
                               Color(0.3f,0.9f,1.0f), Color(0,0,0,0.4f));
}

// ─────────────────────────────────────────────────────────────────────────────
void SceneManager::drawFadeOverlay()
{
    if (m_fadeAlpha <= 0) return;
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.0f, 0.0f, 0.0f, m_fadeAlpha);
    glBegin(GL_QUADS);
    glVertex2f(0,            0);
    glVertex2f(WINDOW_WIDTH, 0);
    glVertex2f(WINDOW_WIDTH, WINDOW_HEIGHT);
    glVertex2f(0,            WINDOW_HEIGHT);
    glEnd();
}

// =============================================================================
//  SCENE 0 – INTRO
//  Robot walks in from the left; title fades in.
// =============================================================================
void SceneManager::renderIntro()
{
    float p = progress();

    // Night-to-day sky gradient
    Color skyT = Color::lerp({0.02f,0.02f,0.15f}, {0.35f,0.65f,0.95f}, p);
    Color skyB = Color::lerp({0.05f,0.05f,0.25f}, {0.70f,0.88f,1.00f}, p);
    Renderer::drawGradientBackground(skyT, skyB);

    // Stars (fade out as day breaks)
    if (p < 0.6f) drawStars(1.0f - p*1.5f, m_elapsed * 2.0f);

    drawGround();

    // Robot walks in from left
    float targetX = WINDOW_WIDTH * 0.35f;
    g_robot.x = -60 + (targetX + 60) * std::min(p * 3.0f, 1.0f);
    g_robot.y = 150;
    if (p < 0.8f) g_robot.startAction(RobotAction::WALK);
    else          g_robot.startAction(RobotAction::IDLE);
    g_robot.draw();

    // "Digital World" title text
    if (p > 0.4f) {
        float ta = clamp((p - 0.4f) * 3.0f, 0.0f, 1.0f);
        Renderer::drawCentredText("Welcome to the Digital Kingdom!",
                                   WINDOW_HEIGHT - 120, Color(1,1,1, ta), 1.0f);
        Renderer::drawCentredText("Watch as algorithms build a world from scratch",
                                   WINDOW_HEIGHT - 100, Color(0.8f,0.9f,1.0f, ta*0.8f));
    }
}

// =============================================================================
//  SCENE 1 – DRAWING ALGORITHMS
//  DDA and Bresenham lines draw roads.
//  Midpoint and Bresenham circles draw fountains and wheels.
// =============================================================================
void SceneManager::renderDrawing()
{
    float p = progress();

    // ── Top-down blueprint background ─────────────────────────────────────
    Renderer::drawGradientBackground(
        Color(0.06f, 0.10f, 0.18f),
        Color(0.09f, 0.14f, 0.24f)
    );

    // Blueprint grid lines
    Color gridCol(0.15f, 0.35f, 0.55f, 0.5f);
    for (int gx = 0; gx < WINDOW_WIDTH; gx += 60)
        Renderer::drawLine((float)gx, 0, (float)gx, WINDOW_HEIGHT, gridCol, 1.0f);
    for (int gy = 0; gy < WINDOW_HEIGHT; gy += 60)
        Renderer::drawLine(0, (float)gy, WINDOW_WIDTH, (float)gy, gridCol, 1.0f);

    // ── Phase 1 (p 0.00-0.35): DDA – horizontal and vertical roads ────────
    {
        float roadP = clamp(p / 0.35f, 0.0f, 1.0f);
        Color roadCol = Palette::road();
        // Horizontal main road
        thickLine(50, 370, 1230, 370, roadCol, 22, roadP, true);
        // Vertical cross-road
        thickLine(640, 20, 640, 740, roadCol, 22, roadP, true);

        if (roadP > 0.1f)
            Renderer::drawText("DDA Road", 60, 395,
                               Color(1.0f, 0.9f, 0.3f, clamp((roadP-0.1f)*2.f, 0.f, 1.f)));
    }

    // ── Phase 2 (p 0.20-0.50): Bresenham – centre dashes ──────────────────
    if (p > 0.20f) {
        float dashP = clamp((p - 0.20f) / 0.30f, 0.0f, 1.0f);
        Color dashCol(1.0f, 0.9f, 0.2f);
        // Horizontal dashes
        for (int xi = 60; xi < 1230; xi += 80) {
            float segP = clamp(dashP - (xi - 60) / 1300.0f, 0.0f, 1.0f);
            bresenhamLine(xi, 370, xi + 40, 370, dashCol, segP);
        }
        // Vertical dashes
        for (int yi = 40; yi < 740; yi += 80) {
            float segP = clamp(dashP - (yi - 40) / 800.0f, 0.0f, 1.0f);
            bresenhamLine(640, yi, 640, yi + 40, dashCol, segP);
        }
    }

    // ── Phase 3 (p 0.35-0.62): Bresenham – castle walls (top-down rect) ───
    if (p > 0.35f) {
        float wP = clamp((p - 0.35f) / 0.27f, 0.0f, 1.0f);
        Color wallCol = Palette::castleGray();

        // Castle centred at road intersection (640, 370)
        int cx = 640, cy = 370, hw = 170, hh = 130;

        // Outer perimeter
        bresenhamLine(cx-hw, cy-hh, cx+hw, cy-hh, wallCol, wP);
        bresenhamLine(cx+hw, cy-hh, cx+hw, cy+hh, wallCol, wP);
        bresenhamLine(cx+hw, cy+hh, cx-hw, cy+hh, wallCol, wP);
        bresenhamLine(cx-hw, cy+hh, cx-hw, cy-hh, wallCol, wP);

        // Inner courtyard
        int iw = 85, ih = 65;
        bresenhamLine(cx-iw, cy-ih, cx+iw, cy-ih, wallCol, wP);
        bresenhamLine(cx+iw, cy-ih, cx+iw, cy+ih, wallCol, wP);
        bresenhamLine(cx+iw, cy+ih, cx-iw, cy+ih, wallCol, wP);
        bresenhamLine(cx-iw, cy+ih, cx-iw, cy-ih, wallCol, wP);

        // Corner tower squares (DDA)
        if (wP > 0.55f) {
            float bP = clamp((wP - 0.55f) / 0.45f, 0.0f, 1.0f);
            int tw = 30;
            int txs[] = {cx - hw,      cx + hw - tw};
            int tys[] = {cy - hh,      cy + hh - tw};
            for (int ti = 0; ti < 2; ++ti)
            for (int tj = 0; tj < 2; ++tj) {
                int tx = txs[ti], ty = tys[tj];
                ddaLine(tx,    ty,    tx+tw, ty,    wallCol, bP);
                ddaLine(tx+tw, ty,    tx+tw, ty+tw, wallCol, bP);
                ddaLine(tx+tw, ty+tw, tx,    ty+tw, wallCol, bP);
                ddaLine(tx,    ty+tw, tx,    ty,    wallCol, bP);
            }
        }
        if (wP > 0.2f)
            Renderer::drawText("Bresenham Castle", cx - 55, cy + hh + 16,
                               Color(0.85f, 0.85f, 0.95f, wP));
    }

    // ── Phase 4 (p 0.50-0.75): Midpoint Circle – fountain (NW quadrant) ───
    if (p > 0.50f) {
        float fP = clamp((p - 0.50f) / 0.25f, 0.0f, 1.0f);
        int fcx = 200, fcy = 175;

        midpointCircle(fcx, fcy, 60, Palette::water(),      fP, false);
        midpointCircle(fcx, fcy, 44, Palette::water(),      fP, true);
        midpointCircle(fcx, fcy, 10, Palette::goldAccent(), fP, true);

        // Radiating jets (top-down)
        if (fP > 0.5f) {
            float jetP = clamp((fP - 0.5f) * 2.0f, 0.0f, 1.0f);
            for (int ji = 0; ji < 8; ++ji) {
                float a = (float)ji * PI / 4.0f;
                ddaLine(fcx, fcy,
                        fcx + (int)(55 * std::cos(a)),
                        fcy + (int)(55 * std::sin(a)),
                        Palette::water(), jetP);
            }
        }
        if (fP > 0.3f)
            Renderer::drawText("Midpoint Circle", fcx - 48, fcy + 76,
                               Color(0.4f, 0.9f, 1.0f, fP));
    }

    // ── Phase 5 (p 0.65-1.0): Bresenham Circle – cart (NE quadrant) ───────
    if (p > 0.65f) {
        float cP = clamp((p - 0.65f) / 0.35f, 0.0f, 1.0f);
        int bx = 1060, by = 175;

        // Cart body (top-down rectangle)
        Renderer::drawFilledRect(bx - 60, by - 30, 120, 60, Palette::woodBrown());

        // Four wheels at corners
        int wr = 22;
        int wxs[] = {bx - 42, bx + 42};
        int wys[] = {by - 22, by + 22};
        for (int wi = 0; wi < 2; ++wi)
        for (int wj = 0; wj < 2; ++wj) {
            int wx = wxs[wi], wy = wys[wj];
            bresenhamCircle(wx, wy, wr, Palette::woodBrown(), cP, false);
            if (cP > 0.5f) {
                float sP = clamp((cP - 0.5f) * 2.0f, 0.0f, 1.0f);
                for (int s = 0; s < 6; ++s) {
                    float a = PI / 3.0f * s;
                    ddaLine(wx, wy,
                            wx + (int)((wr-4) * std::cos(a)),
                            wy + (int)((wr-4) * std::sin(a)),
                            Color(0.7f, 0.5f, 0.2f), sP);
                }
            }
        }
        if (cP > 0.3f)
            Renderer::drawText("Bresenham Circle", bx - 52, by + 62,
                               Color(0.4f, 0.9f, 1.0f, cP));
    }

    // ── TOP VIEW camera indicator (bottom-right corner) ────────────────────
    {
        float ix = WINDOW_WIDTH - 90.0f, iy = 55.0f;
        Renderer::drawFilledCircle(ix, iy, 20, Color(0.2f, 0.7f, 1.0f, 0.8f));
        Renderer::drawFilledCircle(ix, iy,  8, Color(0.05f, 0.1f, 0.2f, 1.0f));
        Renderer::drawLine(ix, iy + 20, ix, iy + 34, Color(0.8f, 0.9f, 1.0f), 2.0f);
        Renderer::drawFilledCircle(ix, iy + 38, 4, Color(1.0f, 0.9f, 0.3f));
        Renderer::drawText("TOP VIEW", ix - 26, iy - 36,
                           Color(0.5f, 0.9f, 1.0f, 0.9f));
    }

    // ── Single algorithm badge per frame (bottom-left, no duplicates) ──────
    std::string algLabel;
    if      (p < 0.20f) algLabel = "DDA Line Drawing";
    else if (p < 0.35f) algLabel = "Bresenham Line Drawing";
    else if (p < 0.50f) algLabel = "Bresenham Line (Castle Walls)";
    else if (p < 0.65f) algLabel = "Midpoint Circle Algorithm";
    else                algLabel = "Bresenham Circle Algorithm";
    m_scenes[static_cast<int>(m_current)].subtitle = algLabel;
}

// =============================================================================
//  SCENE 2 – FILLING ALGORITHMS
//  Scan-Line fill for buildings, Flood Fill for river, Boundary Fill for windows.
// =============================================================================
void SceneManager::renderColouring()
{
    float p = progress();
    drawBackground(p);
    drawGround();

    // ── Castle walls outlines (drawn quickly) ──────────────────────────────
    // Towers: base y=170 (ground level), top y=430
    // Left tower outline
    Renderer::drawRect(180, 170, 100, 260, Palette::castleGray());
    // Right tower outline
    Renderer::drawRect(700, 170, 100, 260, Palette::castleGray());
    // Connecting arch wall (spans between towers near the top)
    Renderer::drawFilledRect(280, 390, 420, 40, Color(0.42f, 0.42f, 0.48f));
    // Road through castle gate
    Renderer::drawFilledRect(0, 125, WINDOW_WIDTH, 45, Palette::road());

    // ── Phase 1: Scan-line fill – castle towers ───────────────────────────
    if (p > 0.0f) {
        float sfP = clamp(p * 5.0f, 0.0f, 1.0f);

        // Left tower body (scan-line fill)
        Polygon2D leftTower;
        leftTower.vertices = {{180,170},{280,170},{280,430},{180,430}};
        leftTower.fillColor = Palette::castleGray();
        scanLineFill(leftTower, sfP);

        // Right tower body (scan-line fill)
        Polygon2D rightTower;
        rightTower.vertices = {{700,170},{800,170},{800,430},{700,430}};
        rightTower.fillColor = Palette::castleGray();
        scanLineFill(rightTower, sfP);

        // ── Roof triangles ON TOP of towers (scan-line) ──────────────────
        Polygon2D leftRoof;
        leftRoof.vertices = {{165, 430}, {295, 430}, {230, 540}};
        leftRoof.fillColor = Palette::roofRed();
        scanLineFill(leftRoof, sfP);

        Polygon2D rightRoof;
        rightRoof.vertices = {{685, 430}, {815, 430}, {750, 540}};
        rightRoof.fillColor = Palette::roofRed();
        scanLineFill(rightRoof, sfP);

        // Battlement notches along top of connecting wall
        if (sfP > 0.6f) {
            float bP = clamp((sfP - 0.6f) / 0.4f, 0.0f, 1.0f);
            Color bCol = Color(0.50f, 0.50f, 0.56f, bP);
            for (int bx = 290; bx < 690; bx += 40) {
                Renderer::drawFilledRect((float)bx, 430, 20, 22, bCol);
            }
        }
    }

    // ── Phase 2: Flood Fill – river (moved down to flow through the grass) 
    if (p > 0.2f) {
        float ffP = clamp((p - 0.2f) * 4.0f, 0.0f, 1.0f);

        // River flows through grass at y=35-105
        std::vector<Point2D> riverVerts = {
            {  0, 35}, {120, 47}, {280, 35},
            {980, 35}, {1100,47}, {1280,35},
            {1280,105}, {1100,93}, {980, 105},
            {280, 105}, {120, 93}, {0, 105}
        };
        Polygon2D river;
        river.vertices  = riverVerts;
        river.fillColor = Palette::water();
        scanLineFill(river, ffP);

        // Ripple circles on river
        if (ffP > 0.5f) {
            float ripP = clamp((ffP - 0.5f) * 2.5f, 0.0f, 1.0f);
            Color ripCol(0.6f, 0.82f, 1.0f, 0.7f);
            midpointCircle(400,  70, 18, ripCol, ripP);
            midpointCircle(760,  70, 14, ripCol, ripP);
            midpointCircle(1060, 70, 16, ripCol, ripP);
        }

        Renderer::drawText("Flood Fill: River", 20, 85,
                           Color(1.0f, 1.0f, 1.0f, ffP));
    }

    // ── Phase 3: Boundary Fill – windows, gate, garden (moved down to ground level)
    if (p > 0.5f) {
        float bfP = clamp((p - 0.5f) * 4.0f, 0.0f, 1.0f);

        // Windows in UPPER section of each tower (y=340–375)
        Color winCol = Palette::windowBlue();
        winCol.a = bfP;
        Renderer::drawFilledRect(195, 340, 28, 35, winCol);
        Renderer::drawFilledRect(237, 340, 28, 35, winCol);
        Renderer::drawRect(195, 340, 28, 35, Color(0.3f, 0.5f, 0.8f, bfP));
        Renderer::drawRect(237, 340, 28, 35, Color(0.3f, 0.5f, 0.8f, bfP));
        Renderer::drawFilledRect(715, 340, 28, 35, winCol);
        Renderer::drawFilledRect(757, 340, 28, 35, winCol);
        Renderer::drawRect(715, 340, 28, 35, Color(0.3f, 0.5f, 0.8f, bfP));
        Renderer::drawRect(757, 340, 28, 35, Color(0.3f, 0.5f, 0.8f, bfP));

        // Gate door (grounded, base at y=170, top at y=290 where arch begins)
        Renderer::drawFilledRect(430, 170, 120, 120,
                                 Color(0.22f, 0.10f, 0.04f, bfP));
        Renderer::drawFilledTriangle(430, 290, 550, 290, 490, 320,
                                     Color(0.22f, 0.10f, 0.04f, bfP));
        Renderer::drawFilledCircle(490, 230, 6, Color(0.85f, 0.65f, 0.10f, bfP));

        // Courtyard garden between towers, at ground level (y=170-215)
        Polygon2D garden;
        garden.vertices  = {{280,170},{700,170},{700,215},{280,215}};
        garden.fillColor = Color(0.18f, 0.72f, 0.28f, bfP * 0.85f);
        scanLineFill(garden, bfP);

        // Flowers with stems
        if (bfP > 0.6f) {
            float flP = clamp((bfP - 0.6f) * 2.5f, 0.0f, 1.0f);
            int flowerXs[] = {310, 355, 400, 445, 540, 585, 630, 670};
            for (int fx : flowerXs) {
                Renderer::drawLine((float)fx, 172.0f, (float)fx, 192.0f,
                                   Color(0.12f, 0.55f, 0.12f, flP), 1.5f);
                midpointCircle(fx, 198, 7, Palette::pinkFlower(), flP, true);
            }
        }

        Renderer::drawText("Boundary Fill: Windows & Garden", 285, 235,
                           Color(1.0f, 1.0f, 1.0f, bfP));
    }

    // Robot points at the castle entrance
    g_robot.x = 80;
    g_robot.y = 170;
    g_robot.pointAt(490, 230);
    g_robot.draw();

    // Algorithm badge (one per frame)
    std::string algLabel;
    if      (p < 0.2f) algLabel = "Scan-Line Polygon Fill";
    else if (p < 0.5f) algLabel = "Flood Fill Algorithm";
    else                algLabel = "Boundary Fill Algorithm";
    m_scenes[static_cast<int>(m_current)].subtitle = algLabel;
}


// =============================================================================
//  SCENE 3 – CLIPPING ALGORITHMS
//  Portal window appears; lines/polygons are clipped against it.
// =============================================================================
void SceneManager::renderPortal()
{
    float p = progress();

    // Deep cosmic background
    Renderer::drawGradientBackground(
        Color(0.02f, 0.01f, 0.10f),
        Color(0.05f, 0.02f, 0.18f)
    );
    drawStars(0.8f, m_elapsed * 0.8f);

    // ── Define the "Royal Viewport" clip window ────────────────────────────
    ClipWindow portal(200, 160, 1080, 580);

    // Animate the viewport growing from centre
    float portalP = clamp(p * 5.0f, 0.0f, 1.0f);
    float shrink  = (1.0f - portalP);
    ClipWindow vp(
        portal.xMin + shrink * 440,
        portal.yMin + shrink * 210,
        portal.xMax - shrink * 440,
        portal.yMax - shrink * 210
    );

    // Draw viewport interior (semi-transparent purple energy shield)
    Renderer::drawFilledRect(vp.xMin, vp.yMin,
                             vp.xMax - vp.xMin, vp.yMax - vp.yMin,
                             Color(0.15f, 0.08f, 0.38f, 0.35f * portalP));

    // Outer neon glow border
    for (int g = 1; g <= 4; ++g) {
        float alpha = (0.15f / (float)g) * portalP;
        Renderer::drawFilledRect(vp.xMin - g * 2, vp.yMin - g * 2,
                                 vp.xMax - vp.xMin + g * 4, vp.yMax - vp.yMin + g * 4,
                                 Color(0.60f, 0.20f, 1.00f, alpha));
    }
    Renderer::drawClipWindow(vp, Color(0.80f, 0.40f, 1.00f, portalP));

    // Viewport corner labels
    if (portalP > 0.5f) {
        float la = clamp((portalP - 0.5f) * 2.0f, 0.0f, 1.0f);
        Renderer::drawText("ROYAL VIEWPORT",
                           vp.xMin + 12, vp.yMax - 22,
                           Color(0.85f, 0.60f, 1.0f, la));
        Renderer::drawText("CLIP ZONE",
                           vp.xMax - 90, vp.yMin + 12,
                           Color(0.85f, 0.60f, 1.0f, la));
    }

    // ═══════════════════════════════════════════════════════════════════════
    //  PHASE 1 (p 0.10-0.40): Point Clipping
    //  Neon Fireflies: Gold/Green inside viewport, cold grey outside
    // ═══════════════════════════════════════════════════════════════════════
    if (p > 0.10f) {
        float pcP = clamp((p - 0.10f) / 0.30f, 0.0f, 1.0f);

        srand(12345);
        for (int i = 0; i < 90; ++i) {
            float seedX = (float)(rand() % 1000) / 1000.0f;
            float seedY = (float)(rand() % 1000) / 1000.0f;
            float speed = 0.5f + (float)(rand() % 100) / 100.0f;

            // Animate firefly floaty movement
            float sx = 50.0f + seedX * (WINDOW_WIDTH - 100) + std::sin(m_elapsed * speed + i) * 25.0f;
            float sy = 100.0f + seedY * (WINDOW_HEIGHT - 200) + std::cos(m_elapsed * speed * 0.8f + i) * 25.0f;

            bool inside = pointClip(sx, sy, vp);
            if (inside) {
                // Gold glowing firefly inside (accepted)
                float pulse = 0.6f + 0.4f * std::sin(m_elapsed * 4.0f + i);
                // Outer glow
                Renderer::drawPoint(sx, sy, 8.0f, Color(1.0f, 0.85f, 0.20f, pcP * pulse * 0.3f));
                // Core
                Renderer::drawPoint(sx, sy, 4.0f, Color(1.0f, 0.95f, 0.60f, pcP * pulse));
            } else {
                // Dim cold blue-grey firefly outside (rejected)
                Renderer::drawPoint(sx, sy, 2.0f, Color(0.25f, 0.30f, 0.45f, pcP * 0.35f));
            }
        }
        srand((unsigned)time(nullptr));

        if (pcP > 0.20f) {
            float la = clamp((pcP - 0.20f) * 2.5f, 0.0f, 1.0f);
            Renderer::drawText(
                "Point Clipping: Fireflies inside viewport glow gold (accepted); outside are dim (rejected)",
                vp.xMin + 12, vp.yMin + 20, Color(1.0f, 0.90f, 0.40f, la));
        }
    }

    // ═══════════════════════════════════════════════════════════════════════
    //  PHASE 2 (p 0.35-0.60): Cohen-Sutherland Line Clipping
    //  Sweeping Neon Lighthouse Searchlights
    // ═══════════════════════════════════════════════════════════════════════
    if (p > 0.35f) {
        float csP = clamp((p - 0.35f) / 0.25f, 0.0f, 1.0f);

        // Lighthouse base at bottom-center of viewport
        float lhX = (vp.xMin + vp.xMax) * 0.5f;
        float lhY = vp.yMin + 15.0f;

        // Sweep angle back and forth
        float sweepBase = 15.0f + 150.0f * (0.5f + 0.5f * std::sin(m_elapsed * 0.9f));

        // Draw a multi-ray cone of light (5 rays separated slightly to form a beam)
        for (int ray = -2; ray <= 2; ++ray) {
            float angleRad = (sweepBase + ray * 1.5f) * 3.14159f / 180.0f;
            float tx = lhX + 900.0f * std::cos(angleRad);
            float ty = lhY + 900.0f * std::sin(angleRad);

            // Draw full background ray extremely faint
            Renderer::drawLine(lhX, lhY, tx, ty, Color(0.1f, 0.05f, 0.2f, csP * 0.2f), 1.0f);

            // Clip the searchlight beam
            float cx1 = lhX, cy1 = lhY, cx2 = tx, cy2 = ty;
            if (cohenSutherlandClip(cx1, cy1, cx2, cy2, vp)) {
                // Draw clipped searchlight beam inside (bright cyan glow)
                float alpha = csP * (1.0f - std::abs(ray) * 0.25f);
                Renderer::drawLine(cx1, cy1, cx2, cy2, Color(0.0f, 0.90f, 1.0f, alpha), 3.0f - std::abs(ray) * 0.5f);
                // Glowing white dots at exit points
                if (std::abs(ray) == 0) {
                    Renderer::drawPoint(cx2, cy2, 7.0f, Color(1, 1, 1, csP));
                    Renderer::drawPoint(cx2, cy2, 12.0f, Color(0.0f, 0.9f, 1.0f, csP * 0.4f));
                }
            }
        }

        // Draw physical lighthouse tower
        Renderer::drawFilledRect(lhX - 10, lhY - 10, 20, 25, Color(0.15f, 0.15f, 0.25f, csP));
        Renderer::drawFilledCircle(lhX, lhY + 15, 8.0f, Color(1.0f, 0.95f, 0.4f, csP));
        Renderer::drawCircle(lhX, lhY + 15, 12.0f, Color(1.0f, 0.8f, 0.2f, csP * 0.5f));

        if (csP > 0.20f) {
            float la = clamp((csP - 0.20f) * 2.5f, 0.0f, 1.0f);
            Renderer::drawText(
                "Cohen-Sutherland: Sweeping lighthouse searchlights clipped at viewport edges",
                vp.xMin + 12, vp.yMin + 38, Color(0.20f, 0.85f, 1.0f, la));
        }
    }

    // ═══════════════════════════════════════════════════════════════════════
    //  PHASE 3 (p 0.57-0.78): Liang-Barsky Line Clipping
    //  Vertical Laser Grid with Parametric t-Pulses
    // ═══════════════════════════════════════════════════════════════════════
    if (p > 0.57f) {
        float lbP = clamp((p - 0.57f) / 0.21f, 0.0f, 1.0f);

        int numLasers = 6;
        for (int i = 0; i < numLasers; ++i) {
            float lx = vp.xMin + (vp.xMax - vp.xMin) * (0.15f + 0.7f * ((float)i / (float)(numLasers - 1)));
            float ly1 = 80.0f;
            float ly2 = 660.0f;

            // Draw unclipped laser path (extremely dim neon red/pink line)
            Renderer::drawLine(lx, ly1, lx, ly2, Color(0.35f, 0.10f, 0.15f, lbP * 0.4f), 1.0f);

            // Clip the laser line
            float cx1 = lx, cy1 = ly1, cx2 = lx, cy2 = ly2;
            if (liangBarskyClip(cx1, cy1, cx2, cy2, vp)) {
                // Draw clipped laser inside viewport (bright neon pink/magenta)
                Renderer::drawLine(cx1, cy1, cx2, cy2, Color(1.0f, 0.15f, 0.45f, lbP), 3.0f);

                // Parametric t energy pulse traveling along the clipped segment
                float t_pulse = std::fmod(m_elapsed * 0.6f + (float)i * 0.18f, 1.0f);
                float px = cx1 + t_pulse * (cx2 - cx1);
                float py = cy1 + t_pulse * (cy2 - cy1);

                // Draw pulse glow
                Renderer::drawPoint(px, py, 10.0f, Color(1.0f, 0.40f, 0.70f, lbP * 0.4f));
                Renderer::drawPoint(px, py, 4.0f, Color(1.0f, 1.0f, 1.0f, lbP));
            }
        }

        if (lbP > 0.20f) {
            float la = clamp((lbP - 0.20f) * 2.5f, 0.0f, 1.0f);
            Renderer::drawText(
                "Liang-Barsky: Grid lasers clipped; glowing energy pulses move parameterically along t [0, 1]",
                vp.xMin + 12, vp.yMin + 56, Color(1.0f, 0.35f, 0.60f, la));
        }
    }

    // ═══════════════════════════════════════════════════════════════════════
    //  PHASE 4 (p 0.76-1.0): Sutherland-Hodgman Polygon Clipping
    //  Rotating Alien Ship
    // ═══════════════════════════════════════════════════════════════════════
    if (p > 0.76f) {
        float shP = clamp((p - 0.76f) / 0.24f, 0.0f, 1.0f);

        // Alien ship center
        float ccX = vp.xMin + (vp.xMax - vp.xMin) * 0.78f;
        float ccY = (vp.yMin + vp.yMax) * 0.5f;

        // Base shapes for the layered alien ship
        std::vector<Point2D> baseBeam = {
            {-20, -22}, {20, -22}, {150, -250}, {-150, -250}
        };
        std::vector<Point2D> baseThruster = {
            {-25, -22}, {-15, -38}, {0, -42}, {15, -38}, {25, -22}
        };
        std::vector<Point2D> baseSaucer = {
            {-80, 0}, {-65, -15}, {-35, -23}, {0, -25}, {35, -23}, {65, -15},
            {80, 0}, {65, 15}, {35, 23}, {0, 25}, {-35, 23}, {-65, 15}
        };
        std::vector<Point2D> baseDome = {
            {-32, 10}, {-30, 25}, {-20, 42}, {0, 50}, {20, 42}, {30, 25}, {32, 10}
        };

        // Rotating lights on the saucer (relative to center)
        std::vector<Point2D> baseLights = {
            {-55, -5}, {-25, -12}, {0, -14}, {25, -12}, {55, -5},
            {-60, 5}, {60, 5}
        };

        // Rotate and scale transformations
        float angle = m_elapsed * 1.3f;
        float scale = 0.5f;

        auto transformAndClip = [&](const std::vector<Point2D>& basePts,
                                    const Color& fillColor,
                                    const Color& borderColor,
                                    bool drawWireframe) {
            std::vector<Point2D> transPts;
            for (auto& pt : basePts) {
                float rx = (pt.x * std::cos(angle) - pt.y * std::sin(angle)) * scale;
                float ry = (pt.x * std::sin(angle) + pt.y * std::cos(angle)) * scale;
                transPts.push_back({ccX + rx, ccY + ry});
            }

            if (drawWireframe) {
                Renderer::drawPolygon(transPts, Color(borderColor.r, borderColor.g, borderColor.b, shP * 0.25f));
            }

            std::vector<Point2D> clipped = sutherlandHodgmanClip(transPts, vp);
            if (!clipped.empty()) {
                Renderer::drawFilledPolygon(clipped, fillColor);
                Renderer::drawPolygon(clipped, borderColor);
            }
        };

        // 1. Draw unclipped/clipped background tractor beam (doesn't rotate with ship, just scales)
        std::vector<Point2D> transBeam;
        for (auto& pt : baseBeam) {
            float rx = pt.x * scale;
            float ry = pt.y * scale;
            transBeam.push_back({ccX + rx, ccY + ry});
        }
        std::vector<Point2D> clippedBeam = sutherlandHodgmanClip(transBeam, vp);
        if (!clippedBeam.empty()) {
            Renderer::drawFilledPolygon(clippedBeam, Color(1.0f, 1.0f, 0.70f, shP * 0.25f));
        }

        // 2. Draw Thruster (green part at bottom)
        transformAndClip(baseThruster, Color(0.0f, 0.95f, 0.20f, shP), Color(0.0f, 0.5f, 0.1f, shP), true);

        // 3. Draw Saucer (dark purple body)
        transformAndClip(baseSaucer, Color(0.15f, 0.12f, 0.28f, shP), Color(0.05f, 0.04f, 0.12f, shP), true);

        // 4. Draw Dome (cyan window)
        transformAndClip(baseDome, Color(0.40f, 0.92f, 0.95f, shP * 0.85f), Color(0.15f, 0.60f, 0.70f, shP), true);

        // 5. Draw Glowing Lights (green beacons on saucer)
        for (auto& pt : baseLights) {
            float rx = (pt.x * std::cos(angle) - pt.y * std::sin(angle)) * scale;
            float ry = (pt.x * std::sin(angle) + pt.y * std::cos(angle)) * scale;
            float lx = ccX + rx;
            float ly = ccY + ry;

            if (pointClip(lx, ly, vp)) {
                float lightPulse = 0.5f + 0.5f * std::sin(m_elapsed * 8.0f + pt.x);
                Renderer::drawPoint(lx, ly, 7.0f, Color(0.0f, 1.0f, 0.3f, shP * lightPulse));
            }
        }

        if (shP > 0.20f) {
            float la = clamp((shP - 0.20f) * 2.5f, 0.0f, 1.0f);
            Renderer::drawText(
                "Sutherland-Hodgman: Floating alien ship clipped edge-by-edge to the viewport boundary",
                vp.xMin + 12, vp.yMin + 74, Color(0.40f, 0.95f, 0.60f, la));
        }
    }

    // Robot waves beside viewport
    g_robot.x = 75;
    g_robot.y = 150;
    g_robot.startAction(RobotAction::WAVE);
    g_robot.draw();

    // Single algorithm badge
    std::string algLabel;
    if      (p < 0.35f)  algLabel = "Point Clipping Algorithm";
    else if (p < 0.57f)  algLabel = "Cohen-Sutherland Line Clipping";
    else if (p < 0.76f)  algLabel = "Liang-Barsky Line Clipping";
    else                  algLabel = "Sutherland-Hodgman Polygon Clipping";
    m_scenes[static_cast<int>(m_current)].subtitle = algLabel;
}


// =============================================================================
//  SCENE 4 – TRANSFORMATIONS
//  Houses translate, clock hands rotate, trees scale, composites happen.
// =============================================================================
void SceneManager::renderTransform()
{
    float p = progress();
    drawBackground(p);
    drawGround();

    // ── TRANSLATION – house slides in ─────────────────────────────────────
    if (p >= 0.0f) {
        float tP  = clamp(p * 5.0f, 0.0f, 1.0f);
        float easeP = tP * tP * (3 - 2*tP); // smoothstep

        // Translation matrix applied to house vertices
        Matrix3x3 T = translationMatrix(-400 * (1.0f - easeP), 0);

        std::vector<Point2D> houseBase = {
            {650,170},{750,170},{750,300},{650,300}
        };
        std::vector<Point2D> houseRoof = {
            {640,300},{760,300},{700,370}
        };

        auto hb = transformPoints(houseBase, T);
        auto hr = transformPoints(houseRoof, T);

        Renderer::drawFilledPolygon(hb, Palette::castleGray());
        Renderer::drawFilledPolygon(hr, Palette::roofRed());
        // Door
        Point2D d0 = T.transform({685,170}), d1 = T.transform({715,170}),
                d2 = T.transform({715,220}), d3 = T.transform({685,220});
        Renderer::drawFilledPolygon({d0,d1,d2,d3}, Palette::woodBrown());

        if (tP > 0.1f)
            Renderer::drawText("Translation: T(tx,ty) matrix",
                               hb[0].x, hb[0].y+10, Color(1,1,0.3f,tP));
    }

    // ── ROTATION – Royal Clock Tower ───────────────────────────────────────
    if (p > 0.2f) {
        float rP  = clamp((p-0.2f)*4.0f, 0.0f, 1.0f);

        float cx = 300.0f;
        float towerBase = 170.0f;
        float towerH    = 150.0f;
        float clockY    = towerBase + towerH; // center of clock face

        // Grey brick tower
        Color gray = Palette::castleGray();
        Color darkGray(gray.r * 0.9f, gray.g * 0.9f, gray.b * 0.9f, gray.a);
        Color borderGray(gray.r * 0.6f, gray.g * 0.6f, gray.b * 0.6f, gray.a);
        Renderer::drawFilledRect(cx - 20, towerBase, 40, towerH, darkGray);
        Renderer::drawRect(cx - 20, towerBase, 40, towerH, borderGray);

        // Clock roof (triangular cap)
        std::vector<Point2D> roof = {
            {cx - 25, clockY + 25}, {cx + 25, clockY + 25}, {cx, clockY + 55}
        };
        Renderer::drawFilledPolygon(roof, Palette::roofRed());

        // Clock face (circle)
        Renderer::drawFilledCircle(cx, clockY, 26, Color(0.95f, 0.95f, 0.90f, rP));
        Renderer::drawCircle(cx, clockY, 26, Color(0.3f, 0.2f, 0.1f, rP), 32);

        // Clock hands (rotating using 3x3 transformation matrix)
        float minuteAngle = m_elapsed * 120.0f; // 120 deg/s
        float hourAngle   = m_elapsed * 10.0f;  // 10 deg/s

        // Minute hand (longer, thin)
        Matrix3x3 R_min = compositeTransform({
            rotationMatrix(minuteAngle),
            translationMatrix(cx, clockY)
        });
        std::vector<Point2D> minHand = { {0, 0}, {0, 20} };
        auto tMin = transformPoints(minHand, R_min);
        Renderer::drawLine(tMin[0].x, tMin[0].y, tMin[1].x, tMin[1].y, Color(0.1f, 0.1f, 0.1f, rP), 2.5f);

        // Hour hand (shorter, thicker)
        Matrix3x3 R_hour = compositeTransform({
            rotationMatrix(hourAngle),
            translationMatrix(cx, clockY)
        });
        std::vector<Point2D> hourHand = { {0, 0}, {0, 13} };
        auto tHour = transformPoints(hourHand, R_hour);
        Renderer::drawLine(tHour[0].x, tHour[0].y, tHour[1].x, tHour[1].y, Color(0.1f, 0.1f, 0.1f, rP), 4.0f);

        // Center pin
        Renderer::drawFilledCircle(cx, clockY, 3.5f, Color(0.8f, 0.6f, 0.1f, rP));

        if (rP > 0.1f)
            Renderer::drawText("Rotation: R(theta) hands around center pivot",
                               cx - 100, clockY - 55, Color(1.0f, 0.85f, 0.3f, rP));
    }

    // ── SCALING – trees grow ──────────────────────────────────────────────
    if (p > 0.45f) {
        float sP  = clamp((p-0.45f)*4.0f, 0.0f, 1.0f);
        float grow = sP; // 0→1

        float tx = 550, ty = 170;
        Matrix3x3 S = scalingAroundPoint(grow*1.0f, grow*1.5f, tx, ty);

        std::vector<Point2D> treeBase = {
            {tx-15, ty}, {tx+15, ty}, {tx, ty+100}
        };
        std::vector<Point2D> treeTop = {
            {tx-25, ty+70}, {tx+25, ty+70}, {tx, ty+150}
        };
        std::vector<Point2D> treeTrunk = {
            {tx-8, ty}, {tx+8, ty}, {tx+8, ty-40}, {tx-8, ty-40}
        };

        Renderer::drawFilledPolygon(transformPoints(treeTrunk, S), Palette::woodBrown());
        Renderer::drawFilledPolygon(transformPoints(treeBase,  S), Palette::leafGreen());
        Renderer::drawFilledPolygon(transformPoints(treeTop,   S), Palette::leafGreen());

        if (sP > 0.1f)
            Renderer::drawText("Scaling: S(sx,sy) matrix",
                               tx-30, ty+160, Color(0.3f,1.0f,0.4f,sP));
    }

    // ── COMPOSITE TRANSFORMATION – orbiting sun ────────────────────────────
    if (p > 0.65f) {
        float cP     = clamp((p - 0.65f) * 5.0f, 0.0f, 1.0f);

        // Sun self-rotation: slow spin (25 deg/s)
        float selfSpin = m_elapsed * 25.0f;

        // Orbit: sun travels in a circle around a fixed centre, staying on-screen.
        // Orbit centre: right half of screen (850, 320); orbit radius 120px
        float orbitCx  = 850.0f, orbitCy = 320.0f;
        float orbitR   = 120.0f;
        float orbitAng = m_elapsed * 20.0f; // 20 deg/s – one full orbit ~18 s
        float orbitAngRad = orbitAng * PI / 180.0f;
        float sunX = orbitCx + orbitR * std::cos(orbitAngRad);
        float sunY = orbitCy + orbitR * std::sin(orbitAngRad);

        // Scale factor pulses gently between 0.8 and 1.1
        float sz = 0.95f + 0.15f * std::sin(m_elapsed * 1.5f);

        // Composite: Scale(sz) → Rotate(selfSpin) → Translate(sunX, sunY)
        Matrix3x3 comp = compositeTransform({
            scalingMatrix(sz, sz),
            rotationMatrix(selfSpin),
            translationMatrix(sunX, sunY)
        });

        // Sun body (circle at orbitCx+orbitR*cos, using comp for rays)
        Renderer::drawFilledCircle(sunX, sunY, 28.0f * sz,
                                   Color(1.0f, 0.90f, 0.15f, cP));

        // 8 rays – defined at origin, scaled + rotated via comp, then drawn
        for (int r = 0; r < 8; ++r) {
            float baseAngle = (float)r * 2.0f * PI / 8.0f;
            float inner = 32.0f, outer = 60.0f, half = 5.0f;
            std::vector<Point2D> rayPts = {
                { inner * std::cos(baseAngle) + half * std::cos(baseAngle + PI/2.0f),
                  inner * std::sin(baseAngle) + half * std::sin(baseAngle + PI/2.0f) },
                { outer * std::cos(baseAngle),
                  outer * std::sin(baseAngle) },
                { inner * std::cos(baseAngle) - half * std::cos(baseAngle + PI/2.0f),
                  inner * std::sin(baseAngle) - half * std::sin(baseAngle + PI/2.0f) }
            };
            // Apply composite transform (scale x rotate x translate)
            auto tRay = transformPoints(rayPts, comp);
            Renderer::drawFilledPolygon(tRay, Color(1.0f, 0.80f, 0.10f, cP * 0.9f));
        }

        // Orbit path (dim dotted circle for reference)
        Renderer::drawCircle(orbitCx, orbitCy, orbitR,
                             Color(1.0f, 0.9f, 0.3f, cP * 0.25f), 60);

        if (cP > 0.2f)
            Renderer::drawText("Composite: Scale x Rotate x Translate",
                               orbitCx - 100, orbitCy + orbitR + 22,
                               Color(1.0f, 1.0f, 0.2f, cP));
    }

    // Robot celebrates
    g_robot.x = 100;
    g_robot.y = 150;
    g_robot.startAction(RobotAction::CELEBRATE);
    g_robot.draw();

    std::string algLabel;
    if      (p < 0.2f)  algLabel = "Translation Matrix T(tx,ty)";
    else if (p < 0.45f) algLabel = "Rotation Matrix R(theta)";
    else if (p < 0.65f) algLabel = "Scaling Matrix S(sx,sy)";
    else                 algLabel = "Composite Transformations";
    m_scenes[static_cast<int>(m_current)].subtitle = algLabel;
}

// =============================================================================
//  SCENE 5 – ISOMETRIC PROJECTION
//  The flat kingdom converts to a 2.5D isometric view.


// =============================================================================
//  SCENE 6 – CINEMATIC FINALE
//  Painter's Algorithm depth sort, parallax scrolling, fireworks.
// =============================================================================
void SceneManager::renderCinematic()
{
    float p = progress();

    // ── Build depth-sorted paint layers ───────────────────────────────────
    std::vector<PaintLayer> layers;

    // Layer 0: Sky (depth=100 – furthest)
    layers.push_back({100.0f, "Sky", Color(0.35f,0.65f,0.95f), [&](){
        Renderer::drawGradientBackground(
            Color(0.20f,0.40f,0.80f),
            Color(0.55f,0.80f,1.00f)
        );
        drawStars(0.3f, m_elapsed);
    }});

    // Layer 1: Distant mountains (depth=80)
    layers.push_back({80.0f, "Mountains", Color(0.5f,0.6f,0.7f), [&](){
        // Three mountain silhouettes
        std::vector<Point2D> m1={{0,350},{300,600},{600,350},{900,580},{1280,350},{1280,720},{0,720}};
        Renderer::drawFilledPolygon(m1, Color(0.45f,0.50f,0.60f));
        std::vector<Point2D> m2={{0,400},{250,580},{550,380},{800,560},{1100,400},{1280,450},{1280,720},{0,720}};
        Renderer::drawFilledPolygon(m2, Color(0.35f,0.45f,0.55f));
    }});

    // Layer 2: Background trees (depth=60)
    layers.push_back({60.0f, "BG Trees", Color(0.2f,0.6f,0.3f), [&](){
        for (int i = 0; i < 8; ++i) {
            float tx = 80 + i * 170.0f;
            float h  = 100 + (i%3)*40.0f;
            Renderer::drawFilledTriangle(tx-30,320, tx+30,320, tx,320+h,
                                         Color(0.15f,0.55f,0.25f));
            Renderer::drawFilledRect(tx-8,280, 16,40, Color(0.4f,0.25f,0.1f));
        }
    }});

    // Layer 3: Ground (depth=40)
    layers.push_back({40.0f, "Ground", Palette::grass(), [&](){
        Renderer::drawFilledRect(0,0, WINDOW_WIDTH,320, Palette::grass());
        Renderer::drawFilledRect(0,120, WINDOW_WIDTH,50, Palette::road());
    }});

    // Layer 4: Kingdom buildings (depth=30)
    layers.push_back({30.0f, "Buildings", Palette::castleGray(), [&](){
        // Castle
        Renderer::drawFilledRect(400,170, 200,220, Palette::castleGray());
        // Roof
        std::vector<Point2D> rf={{390,390},{610,390},{500,460}};
        Renderer::drawFilledPolygon(rf, Palette::roofRed());
        // Windows
        Renderer::drawFilledRect(430,250, 35,45, Palette::windowBlue());
        Renderer::drawFilledRect(535,250, 35,45, Palette::windowBlue());
        // Side houses
        Renderer::drawFilledRect(200,200, 100,170, Color(0.8f,0.7f,0.5f));
        Renderer::drawFilledTriangle(190,370, 310,370, 250,430, Palette::roofRed());
        Renderer::drawFilledRect(750,200, 100,170, Color(0.7f,0.75f,0.85f));
        Renderer::drawFilledTriangle(740,370, 860,370, 800,430, Palette::roofRed());
    }});

    // Layer 5: Foreground trees (depth=15)
    layers.push_back({15.0f, "FG Trees", Color(0.2f,0.75f,0.3f), [&](){
        auto drawTree = [&](float tx, float ty){
            Renderer::drawFilledRect(tx-10,ty, 20,50, Palette::woodBrown());
            Renderer::drawFilledTriangle(tx-35,ty+50, tx+35,ty+50, tx,ty+120, Palette::leafGreen());
            Renderer::drawFilledTriangle(tx-28,ty+90, tx+28,ty+90, tx,ty+150,
                                         Color(0.15f,0.65f,0.25f));
        };
        drawTree(100, 150); drawTree(1180, 150);
        drawTree(50,  170); drawTree(1220, 170);
    }});

    // Layer 6: Robot (depth=5 – closest)
    layers.push_back({5.0f, "Robot", Palette::robotBody(), [&](){
        g_robot.x = 600 + std::sin(m_elapsed*0.8f)*80;
        g_robot.y = 150;
        g_robot.startAction(RobotAction::CELEBRATE);
        g_robot.draw();
    }});

    // Apply parallax and render
    applyParallax(layers, g_scrollOffset, 100.0f);

    // ── Fireworks ─────────────────────────────────────────────────────────
    if (p > 0.3f) {
        static float nextFW = 0;
        if (m_elapsed > nextFW) {
            float fx = 200 + (float)rand()/RAND_MAX * 880;
            float fy = 300 + (float)rand()/RAND_MAX * 200;
            Renderer::spawnFirework(g_particles, fx, fy);
            nextFW = m_elapsed + 0.4f + (float)rand()/RAND_MAX * 0.8f;
        }
        Renderer::drawParticles(g_particles);
    }

    // ── Final title card ──────────────────────────────────────────────────
    if (p > 0.7f) {
        float ta = clamp((p - 0.7f) * 4.0f, 0.0f, 1.0f);

        // Dark overlay
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(0,0,0,0.55f*ta);
        glBegin(GL_QUADS);
        glVertex2f(0,WINDOW_HEIGHT-180); glVertex2f(WINDOW_WIDTH,WINDOW_HEIGHT-180);
        glVertex2f(WINDOW_WIDTH,WINDOW_HEIGHT-60); glVertex2f(0,WINDOW_HEIGHT-60);
        glEnd();

        Renderer::drawCentredText("Computer Graphics Algorithms in Action",
                                   WINDOW_HEIGHT - 140, Color(1.0f,0.95f,0.2f,ta));
        Renderer::drawCentredText("The Little Robot Builds a Digital Kingdom",
                                   WINDOW_HEIGHT - 110, Color(0.8f,0.9f,1.0f,ta));
    }

    // Depth sort label
    m_scenes[static_cast<int>(m_current)].subtitle = "Painter's Algorithm (Depth Sort + Parallax)";

    // Show depth order in corner
    if (p < 0.5f) {
        float la = clamp(1.0f - p*2.5f, 0.0f, 1.0f);
        Renderer::drawText("Depth order (back→front):", 10, 90, Color(1,1,1,la));
        const char* order[] = {"Sky(100) → Mountains(80) → BG Trees(60) → Ground(40) → Buildings(30) → FG Trees(15) → Robot(5)"};
        Renderer::drawText(order[0], 10, 72, Color(0.8f,1.0f,0.8f,la));
    }
}
