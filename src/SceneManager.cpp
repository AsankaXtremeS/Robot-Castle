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
        {SceneID::ISOMETRIC, "Scene 5 – Isometric Projection","Building the 2.5D Kingdom",                          25.0f},
        {SceneID::CINEMATIC, "Scene 6 – Grand Finale",        "Painter's Algorithm · Parallax · Fireworks",         25.0f},
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
    case SceneID::ISOMETRIC: renderIsometric(); break;
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
    Renderer::drawAlgorithmBadge(algLabel);
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
    // Left tower
    Renderer::drawRect(200, 170, 80, 230, Palette::castleGray());
    // Right tower
    Renderer::drawRect(700, 170, 80, 230, Palette::castleGray());
    // Connecting wall
    Renderer::drawFilledRect(280, 350, 420, 50, Color(0.45f,0.45f,0.5f));
    // Road
    Renderer::drawFilledRect(0, 125, WINDOW_WIDTH, 45, Palette::road());

    // ── Scan-line fill castle towers ──────────────────────────────────────
    if (p > 0.0f) {
        float sfP = clamp(p * 5.0f, 0.0f, 1.0f);
        // Left tower
        Polygon2D leftTower;
        leftTower.vertices = {{200,170},{280,170},{280,400},{200,400}};
        leftTower.fillColor = Palette::castleGray();
        scanLineFill(leftTower, sfP);

        // Right tower
        Polygon2D rightTower;
        rightTower.vertices = {{700,170},{780,170},{780,400},{700,400}};
        rightTower.fillColor = Palette::castleGray();
        scanLineFill(rightTower, sfP);

        // Roof triangles (Scan-Line)
        Polygon2D leftRoof;
        leftRoof.vertices = {{190,170},{290,170},{240,300}};
        leftRoof.fillColor = Palette::roofRed();
        scanLineFill(leftRoof, sfP);

        Polygon2D rightRoof;
        rightRoof.vertices = {{690,170},{790,170},{740,300}};
        rightRoof.fillColor = Palette::roofRed();
        scanLineFill(rightRoof, sfP);
    }

    // ── Simulate Flood Fill – river ────────────────────────────────────────
    // We simulate by drawing expanding filled region from seed
    if (p > 0.2f) {
        float ffP = clamp((p - 0.2f) * 4.0f, 0.0f, 1.0f);
        // River shape (drawn as animated expanding polygon)
        std::vector<Point2D> riverVerts = {
            {350, 80}, {450, 80}, {500, 50},
            {1100,50}, {1150,80},{1100,120},
            {500, 120},{450,110},{350,120}
        };
        Polygon2D river;
        river.vertices  = riverVerts;
        river.fillColor = Palette::water();
        scanLineFill(river, ffP); // visual proxy for flood fill

        // Ripple circles on river (Midpoint circles)
        if (ffP > 0.6f) {
            float ripP = clamp((ffP-0.6f)*2.5f,0.0f,1.0f);
            Color ripCol(0.5f, 0.7f, 1.0f, 0.6f);
            midpointCircle(700, 85, 20, ripCol, ripP);
            midpointCircle(900, 85, 15, ripCol, ripP);
        }

        // Label
        Renderer::drawText("Flood Fill: River", 355, 90, Color(1,1,1,ffP));
    }

    // ── Boundary Fill – windows ────────────────────────────────────────────
    if (p > 0.5f) {
        float bfP = clamp((p - 0.5f) * 4.0f, 0.0f, 1.0f);
        // Simulate boundary fill by drawing filled windows with animated alpha
        Color winCol = Palette::windowBlue();
        winCol.a = bfP;
        // Left tower windows
        Renderer::drawFilledRect(215, 310, 20, 25, winCol);
        Renderer::drawFilledRect(245, 310, 20, 25, winCol);
        // Right tower windows
        Renderer::drawFilledRect(715, 310, 20, 25, winCol);
        Renderer::drawFilledRect(745, 310, 20, 25, winCol);
        // Gate arch (boundary fill shape)
        Renderer::drawFilledRect(420, 350, 100, 50, Color(0.3f,0.15f,0.05f,bfP));

        // Garden – boundary fill approximation
        Polygon2D garden;
        garden.vertices  = {{290,350},{420,350},{420,400},{290,400}};
        garden.fillColor = Color(0.2f, 0.75f, 0.3f, bfP);
        scanLineFill(garden, bfP);

        // Flowers
        if (bfP > 0.7f) {
            float flP = clamp((bfP-0.7f)*3.0f,0.0f,1.0f);
            for (int fi = 0; fi < 5; ++fi) {
                float fx = 300 + fi * 25;
                midpointCircle((int)fx, 375, 8, Palette::pinkFlower(), flP, true);
            }
        }
        Renderer::drawText("Boundary Fill: Windows & Garden", 295, 325, Color(1,1,1,bfP));
    }

    // Robot points at kingdom
    g_robot.x = 100;
    g_robot.y = 150;
    g_robot.pointAt(490, 300);
    g_robot.draw();

    // Algorithm label
    std::string algLabel;
    if      (p < 0.2f) algLabel = "Scan-Line Polygon Fill";
    else if (p < 0.5f) algLabel = "Flood Fill Algorithm";
    else                algLabel = "Boundary Fill Algorithm";
    Renderer::drawAlgorithmBadge(algLabel);
}

// =============================================================================
//  SCENE 3 – CLIPPING ALGORITHMS
//  Portal window appears; lines/polygons are clipped against it.
// =============================================================================
void SceneManager::renderPortal()
{
    float p = progress();

    // Dark, magical background
    Renderer::drawGradientBackground(
        Color(0.05f, 0.02f, 0.20f),
        Color(0.10f, 0.05f, 0.35f)
    );
    drawStars(1.0f, m_elapsed * 1.5f);

    // ── Define the clip window (the "portal") ─────────────────────────────
    ClipWindow portal(300, 150, 980, 550);

    // Animate portal appearing
    float portalP = clamp(p * 5.0f, 0.0f, 1.0f);
    ClipWindow animPortal(
        portal.xMin + (1 - portalP) * 340,
        portal.yMin + (1 - portalP) * 200,
        portal.xMax - (1 - portalP) * 340,
        portal.yMax - (1 - portalP) * 200
    );

    // Draw portal glow
    Renderer::drawFilledRect(animPortal.xMin-4, animPortal.yMin-4,
                              animPortal.xMax-animPortal.xMin+8,
                              animPortal.yMax-animPortal.yMin+8,
                              Color(0.4f,0.2f,1.0f,0.3f*portalP));
    Renderer::drawClipWindow(animPortal, Color(0.6f,0.3f,1.0f,portalP));
    Renderer::drawText("Clip Window", animPortal.xMin+5, animPortal.yMin+5,
                       Color(0.8f,0.5f,1.0f,portalP));

    // ── Point Clipping – stars ─────────────────────────────────────────────
    if (p > 0.1f) {
        float pcP = clamp((p - 0.1f) * 5.0f, 0.0f, 1.0f);
        srand(99);
        for (int i = 0; i < 80; ++i) {
            float sx = (float)(rand() % WINDOW_WIDTH);
            float sy = 150.0f + (float)(rand() % 430);
            bool inside = pointClip(sx, sy, animPortal);
            Color c = inside
                ? Color(1.0f, 0.95f, 0.3f, pcP)   // gold – inside = visible
                : Color(0.4f, 0.4f,  0.5f, pcP * 0.3f); // dim – outside
            Renderer::drawPoint(sx, sy, inside ? 3.0f : 1.5f, c);
        }
        srand((unsigned)time(nullptr));
        if (pcP > 0.3f)
            Renderer::drawText("Point Clipping: stars inside portal are bright",
                               310, 540, Color(1,1,1,clamp((pcP-0.3f)*2.0f,0,1)));
    }

    // ── Cohen-Sutherland line clipping – roads ────────────────────────────
    if (p > 0.3f) {
        float csP = clamp((p - 0.3f) * 4.0f, 0.0f, 1.0f);
        // Several lines spanning the whole screen
        struct LS { float x1,y1,x2,y2; Color col; };
        std::vector<LS> lines = {
            {50,  200, 1230, 500, Color(0.9f,0.5f,0.1f)},
            {50,  350, 1230, 200, Color(0.1f,0.9f,0.6f)},
            {400, 80,  600,  600, Color(0.9f,0.2f,0.5f)},
            {100, 480, 1150, 160, Color(0.3f,0.6f,1.0f)},
        };

        for (auto& ln : lines) {
            // Draw original (dim)
            Renderer::drawLine(ln.x1,ln.y1, ln.x2,ln.y2,
                               Color(ln.col.r*0.25f,ln.col.g*0.25f,ln.col.b*0.25f,csP*0.5f),
                               1.5f);
            // Clip with Cohen-Sutherland
            float cx1=ln.x1, cy1=ln.y1, cx2=ln.x2, cy2=ln.y2;
            if (cohenSutherlandClip(cx1,cy1,cx2,cy2,animPortal)) {
                Renderer::drawLine(cx1,cy1, cx2,cy2, Color(ln.col.r,ln.col.g,ln.col.b,csP), 3.0f);
            }
        }
        if (csP > 0.4f)
            Renderer::drawText("Cohen-Sutherland: only visible segments kept",
                               310, 158, Color(1,1,1,clamp((csP-0.4f)*2.0f,0,1)));
    }

    // ── Liang-Barsky – laser beams ─────────────────────────────────────────
    if (p > 0.55f) {
        float lbP = clamp((p - 0.55f) * 4.0f, 0.0f, 1.0f);
        struct LS2 { float x1,y1,x2,y2; Color col; };
        std::vector<LS2> lasers = {
            {0,   250, 1280, 450, Color(1.0f,0.2f,0.8f)},
            {0,   450, 1280, 250, Color(0.2f,1.0f,0.5f)},
            {640, 100, 640,  620, Color(1.0f,0.8f,0.1f)},
        };
        for (auto& ln : lasers) {
            Renderer::drawLine(ln.x1,ln.y1,ln.x2,ln.y2,
                               Color(ln.col.r*0.15f,ln.col.g*0.15f,ln.col.b*0.15f,lbP*0.4f),
                               1.0f);
            float cx1=ln.x1,cy1=ln.y1,cx2=ln.x2,cy2=ln.y2;
            if (liangBarskyClip(cx1,cy1,cx2,cy2,animPortal)) {
                glLineWidth(3.0f);
                glBegin(GL_LINES);
                glColor4f(ln.col.r,ln.col.g,ln.col.b,lbP);
                glVertex2f(cx1,cy1);
                glColor4f(1,1,1,lbP);
                glVertex2f(cx2,cy2);
                glEnd();
                glLineWidth(1.0f);
            }
        }
        if (lbP > 0.4f)
            Renderer::drawText("Liang-Barsky: parametric laser clipping",
                               310, 158, Color(0.5f,1.0f,0.5f,clamp((lbP-0.4f)*2.0f,0,1)));
    }

    // ── Sutherland-Hodgman polygon clipping – cloud ────────────────────────
    if (p > 0.75f) {
        float shP = clamp((p - 0.75f) * 5.0f, 0.0f, 1.0f);

        // A floating cloud polygon
        float cPhase = m_elapsed * 0.8f;
        std::vector<Point2D> cloud = {
            {200 + std::sin(cPhase)*30, 320},
            {320 + std::cos(cPhase)*20, 250},
            {460, 280 + std::sin(cPhase)*20},
            {550, 310},
            {600 + std::sin(cPhase)*25, 400},
            {500, 460},
            {350, 470},
            {200, 430},
            {150 + std::cos(cPhase)*20, 380}
        };

        // Original (outside clip window, dim)
        Renderer::drawPolygon(cloud, Color(0.4f,0.4f,0.6f,shP*0.4f));

        // Clip with Sutherland-Hodgman
        std::vector<Point2D> clipped = sutherlandHodgmanClip(cloud, animPortal);
        if (!clipped.empty()) {
            Renderer::drawFilledPolygon(clipped, Color(0.8f,0.8f,1.0f,shP*0.5f));
            Renderer::drawPolygon(clipped, Color(0.9f,0.9f,1.0f,shP));
        }
        if (shP > 0.4f)
            Renderer::drawText("Sutherland-Hodgman: polygon cloud clipped to portal",
                               310, 540, Color(1,1,1,clamp((shP-0.4f)*2.0f,0,1)));
    }

    // Robot stands beside portal
    g_robot.x = 180;
    g_robot.y = 150;
    g_robot.startAction(RobotAction::WAVE);
    g_robot.draw();

    // Algorithm label
    std::string algLabel;
    if      (p < 0.3f)  algLabel = "Point Clipping Algorithm";
    else if (p < 0.55f) algLabel = "Cohen-Sutherland Line Clipping";
    else if (p < 0.75f) algLabel = "Liang-Barsky Line Clipping";
    else                 algLabel = "Sutherland-Hodgman Polygon Clipping";
    Renderer::drawAlgorithmBadge(algLabel);
}

// =============================================================================
//  SCENE 4 – TRANSFORMATIONS
//  Houses translate, windmill rotates, trees scale, composites happen.
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

    // ── ROTATION – windmill ────────────────────────────────────────────────
    if (p > 0.2f) {
        float rP  = clamp((p-0.2f)*4.0f, 0.0f, 1.0f);

        // Tower: base at ground (y=170), hub at top of tower
        float cx = 300.0f;
        float towerBase = 170.0f;   // ground level (y coordinate)
        float towerH    = 200.0f;   // tower height in pixels
        float hubY      = towerBase + towerH; // hub is at TOP of tower (higher Y)

        // Windmill pole (tower body)
        Renderer::drawFilledRect(cx - 14, towerBase, 28, towerH, Palette::castleGray());

        // Slow spin: 45 degrees per second
        float angle = m_elapsed * 45.0f;

        // Each blade: thin tapered rectangle defined at ORIGIN facing right (+X),
        // then rotated into place around the hub.
        // Blade shape (at origin): root near (0,0), tip at (bladeLen, 0)
        float bladeLen = 72.0f;
        for (int b = 0; b < 4; ++b) {
            float bladeAngle = angle + b * 90.0f;
            // Composite: rotate around origin, then translate to hub
            Matrix3x3 R = compositeTransform({
                rotationMatrix(bladeAngle),
                translationMatrix(cx, hubY)
            });
            // Blade polygon: tapered – wide at root, narrow at tip
            std::vector<Point2D> blade = {
                {  4.0f,  -6.0f },
                { bladeLen, -3.0f },
                { bladeLen,  3.0f },
                {  4.0f,   6.0f }
            };
            auto rb = transformPoints(blade, R);
            Renderer::drawFilledPolygon(rb, Color(0.92f, 0.87f, 0.58f, rP));
            Renderer::drawPolygon(rb, Color(0.70f, 0.60f, 0.30f, rP));
        }

        // Centre hub circle
        Renderer::drawFilledCircle(cx, hubY, 11, Color(0.6f, 0.40f, 0.15f, rP));
        Renderer::drawCircle(cx, hubY, 11, Color(0.85f, 0.65f, 0.25f, rP));

        if (rP > 0.1f)
            Renderer::drawText("Rotation: R(theta) matrix around pivot",
                               cx - 55, hubY - 20, Color(1.0f, 0.85f, 0.3f, rP));
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
    Renderer::drawAlgorithmBadge(algLabel);
}

// =============================================================================
//  SCENE 5 – ISOMETRIC PROJECTION
//  The flat kingdom converts to a 2.5D isometric view.
// =============================================================================
void SceneManager::renderIsometric()
{
    float p = progress();

    Renderer::drawGradientBackground(
        Color(0.15f,0.30f,0.60f),
        Color(0.50f,0.75f,0.95f)
    );

    // Isometric projection setup
    IsoProjection iso(55.0f, WINDOW_WIDTH * 0.5f, WINDOW_HEIGHT * 0.38f);

    // ── Ground grid ────────────────────────────────────────────────────────
    float gridP = clamp(p * 3.0f, 0.0f, 1.0f);
    drawIsoGrid(iso, 12, 8, Color(0.25f,0.72f,0.35f), Color(0.22f,0.65f,0.30f), gridP);

    // ── Buildings ──────────────────────────────────────────────────────────
    if (p > 0.2f) {
        float bP = clamp((p-0.2f)*4.0f, 0.0f, 1.0f);

        // Castle towers (isometric boxes)
        drawIsoBox(iso, 0,0,0, 2,3,2, Color(0.6f,0.6f,0.65f),Color(0.45f,0.45f,0.5f),Color(0.5f,0.5f,0.55f), bP);
        drawIsoBox(iso, 4,0,0, 2,3,2, Color(0.6f,0.6f,0.65f),Color(0.45f,0.45f,0.5f),Color(0.5f,0.5f,0.55f), bP);
        drawIsoBox(iso, 0,0,4, 2,3,2, Color(0.6f,0.6f,0.65f),Color(0.45f,0.45f,0.5f),Color(0.5f,0.5f,0.55f), bP);
        drawIsoBox(iso, 4,0,4, 2,3,2, Color(0.6f,0.6f,0.65f),Color(0.45f,0.45f,0.5f),Color(0.5f,0.5f,0.55f), bP);

        // Main keep
        drawIsoBox(iso, 1.5f,0,1.5f, 3,4.5f,3,
                   Color(0.70f,0.60f,0.55f),Color(0.55f,0.45f,0.40f),Color(0.62f,0.52f,0.47f), bP);
    }

    // ── Houses ────────────────────────────────────────────────────────────
    if (p > 0.4f) {
        float hP = clamp((p-0.4f)*4.0f, 0.0f, 1.0f);
        drawIsoBox(iso, -3,0,1, 2,2,2, Color(0.85f,0.75f,0.55f),Color(0.65f,0.55f,0.35f),Color(0.75f,0.65f,0.45f), hP);
        drawIsoBox(iso,  7,0,2, 2,2,2, Color(0.75f,0.80f,0.90f),Color(0.60f,0.65f,0.75f),Color(0.68f,0.73f,0.83f), hP);
        drawIsoBox(iso, -2,0,5, 2,2.5f,2, Color(0.88f,0.70f,0.60f),Color(0.68f,0.50f,0.40f),Color(0.78f,0.60f,0.50f), hP);
    }

    // ── Trees ─────────────────────────────────────────────────────────────
    if (p > 0.6f) {
        float tP  = clamp((p-0.6f)*4.0f, 0.0f, 1.0f);
        // Tree trunks
        drawIsoBox(iso, -1,0,2, 0.4f,1.5f,0.4f, Color(0.55f,0.35f,0.15f),Color(0.40f,0.25f,0.10f),Color(0.48f,0.30f,0.12f), tP);
        drawIsoBox(iso,  7,0,0, 0.4f,1.5f,0.4f, Color(0.55f,0.35f,0.15f),Color(0.40f,0.25f,0.10f),Color(0.48f,0.30f,0.12f), tP);
        // Foliage (wider/taller box)
        drawIsoBox(iso, -1.3f,1.5f,1.7f, 1.0f,1.5f,1.0f, Color(0.2f,0.75f,0.3f),Color(0.15f,0.6f,0.25f),Color(0.18f,0.68f,0.28f), tP);
        drawIsoBox(iso,  6.7f,1.5f,-0.3f, 1.0f,1.5f,1.0f, Color(0.2f,0.75f,0.3f),Color(0.15f,0.6f,0.25f),Color(0.18f,0.68f,0.28f), tP);
    }

    // ── Robot in iso view ─────────────────────────────────────────────────
    Point2D rIso = iso.project({-3.5f, 0, 3});
    g_robot.x = rIso.x;
    g_robot.y = rIso.y - 150; // lift off ground plane
    g_robot.scale = 0.7f;
    g_robot.startAction(RobotAction::CELEBRATE);
    g_robot.draw();
    g_robot.scale = 1.0f;

    Renderer::drawAlgorithmBadge("Isometric Projection Matrix");
}

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
    Renderer::drawAlgorithmBadge("Painter's Algorithm (Depth Sort + Parallax)");

    // Show depth order in corner
    if (p < 0.5f) {
        float la = clamp(1.0f - p*2.5f, 0.0f, 1.0f);
        Renderer::drawText("Depth order (back→front):", 10, 90, Color(1,1,1,la));
        const char* order[] = {"Sky(100) → Mountains(80) → BG Trees(60) → Ground(40) → Buildings(30) → FG Trees(15) → Robot(5)"};
        Renderer::drawText(order[0], 10, 72, Color(0.8f,1.0f,0.8f,la));
    }
}
