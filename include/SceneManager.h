/**
 * SceneManager.h
 * Manages the sequence of animated scenes, timing, and transitions.
 *
 * Scenes:
 *   0 – Intro       (robot enters)
 *   1 – Drawing     (roads, fountains – DDA, Bresenham Line & Circle)
 *   2 – Colouring   (fills – Scan-Line, Flood, Boundary)
 *   3 – Portal      (clipping – Point, Cohen-Sutherland, Liang-Barsky, SH)
 *   4 – Transform   (translation, rotation, scaling)
 *   5 – Isometric   (iso projection)
 *   6 – Cinematic   (Painter's, parallax, fireworks)
 */

#pragma once
#include "Common.h"
#include <string>
#include <functional>
#include <vector>

// ─────────────────────────────────────────────────────────────────────────────
enum class SceneID {
    INTRO      = 0,
    DRAWING    = 1,
    COLOURING  = 2,
    PORTAL     = 3,
    TRANSFORM  = 4,
    ISOMETRIC  = 5,
    CINEMATIC  = 6,
    COUNT      = 7
};

// ─────────────────────────────────────────────────────────────────────────────
struct SceneInfo {
    SceneID     id;
    std::string title;
    std::string subtitle;
    float       durationSec;   // how long this scene runs
};

// ─────────────────────────────────────────────────────────────────────────────
class SceneManager {
public:
    SceneManager();

    // Called each frame from the GLUT timer
    void update(float dtSec);

    // Called each frame from the display callback
    void render();

    // Jump directly to a scene (for testing)
    void jumpTo(SceneID scene);

    // Current scene progress [0..1]
    float progress() const;

    // Current scene elapsed time in seconds
    float elapsed() const { return m_elapsed; }

    // Total elapsed seconds across all scenes
    float totalElapsed() const { return m_totalElapsed; }

    SceneID  currentScene() const { return m_current; }
    bool     isTransitioning() const { return m_fadeAlpha > 0.0f && m_fadeIn; }

private:
    SceneID m_current;
    float   m_elapsed;        // seconds since current scene started
    float   m_totalElapsed;   // total project time
    float   m_fadeAlpha;      // 0=transparent, 1=black (for fade transitions)
    bool    m_fadeIn;         // true during fade-in, false during fade-out

    std::vector<SceneInfo> m_scenes;

    void renderScene(SceneID id);
    void drawFadeOverlay();
    void drawHUD();           // scene title + algorithm label
    void advanceScene();

    // ── Scene render functions ──────────────────────────────────────────────
    void renderIntro();
    void renderDrawing();
    void renderColouring();
    void renderPortal();
    void renderTransform();
    void renderIsometric();
    void renderCinematic();

    // ── Shared background helpers ──────────────────────────────────────────
    void drawBackground(float t);
    void drawGround();
    void drawStars(float density, float twinkle);
};

// Global singleton accessor
SceneManager& getSceneManager();
