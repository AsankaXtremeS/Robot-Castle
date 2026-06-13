/**
 * main.cpp
 * "The Little Robot Builds a Digital Kingdom"
 * Computer Graphics Algorithms Demonstration
 *
 * Entry point: sets up a 1280×720 FreeGLUT window with double buffering,
 * registers GLUT callbacks, and starts the animation timer loop.
 *
 * Key controls:
 *   SPACE / Right Arrow → skip to next scene
 *   1-7                 → jump directly to a scene
 *   ESC / Q             → quit
 *   F                   → toggle full-screen
 */

#include <GL/freeglut.h>
#include <cstdlib>
#include <ctime>
#include <iostream>

#include "include/Common.h"
#include "include/SceneManager.h"
#include "include/Renderer.h"

// ─────────────────────────────────────────────────────────────────────────────
//  GLUT Display Callback – called each frame
// ─────────────────────────────────────────────────────────────────────────────
void display()
{
    glClear(GL_COLOR_BUFFER_BIT);

    // Enable blending for transparency effects
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Enable smooth points / lines
    glEnable(GL_POINT_SMOOTH);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

    // Render the current scene
    getSceneManager().render();

    glutSwapBuffers();
}

// ─────────────────────────────────────────────────────────────────────────────
//  GLUT Timer Callback – drives animation at ~60 FPS
// ─────────────────────────────────────────────────────────────────────────────
void timer(int /*value*/)
{
    static int  lastTime = 0;
    int         now      = glutGet(GLUT_ELAPSED_TIME);
    int         dtMs     = (lastTime == 0) ? 16 : now - lastTime;
    lastTime = now;

    float dtSec = dtMs / 1000.0f;
    if (dtSec > 0.1f) dtSec = 0.1f; // cap to prevent large jumps

    getSceneManager().update(dtSec);

    glutPostRedisplay();
    glutTimerFunc(static_cast<unsigned>(FRAME_TIME), timer, 0);
}

// ─────────────────────────────────────────────────────────────────────────────
//  GLUT Reshape Callback
// ─────────────────────────────────────────────────────────────────────────────
void reshape(int w, int h)
{
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // Orthographic 2D projection – pixels map 1:1 at native resolution
    // Scale to fill the window while preserving aspect ratio
    float aspect  = (float)w / (float)h;
    float natAsp  = (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT;

    if (aspect > natAsp) {
        float excess = aspect / natAsp;
        gluOrtho2D(-(WINDOW_WIDTH*(excess-1)/2.0), WINDOW_WIDTH + WINDOW_WIDTH*(excess-1)/2.0,
                    0, WINDOW_HEIGHT);
    } else {
        float excess = natAsp / aspect;
        gluOrtho2D(0, WINDOW_WIDTH,
                    -(WINDOW_HEIGHT*(excess-1)/2.0), WINDOW_HEIGHT + WINDOW_HEIGHT*(excess-1)/2.0);
    }

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

// ─────────────────────────────────────────────────────────────────────────────
//  GLUT Keyboard Callback
// ─────────────────────────────────────────────────────────────────────────────
void keyboard(unsigned char key, int /*x*/, int /*y*/)
{
    SceneManager& sm = getSceneManager();

    switch (key) {
    case 27:  // ESC
    case 'q':
    case 'Q':
        glutDestroyWindow(glutGetWindow());
        exit(0);
        break;

    case ' ':  // Space – next scene
        sm.jumpTo(static_cast<SceneID>((int)sm.currentScene() + 1));
        break;

    case 'f':  // Toggle full-screen
    case 'F':
        glutFullScreen();
        break;

    case '1': sm.jumpTo(SceneID::INTRO);     break;
    case '2': sm.jumpTo(SceneID::DRAWING);   break;
    case '3': sm.jumpTo(SceneID::COLOURING); break;
    case '4': sm.jumpTo(SceneID::PORTAL);    break;
    case '5': sm.jumpTo(SceneID::TRANSFORM); break;
    case '6': sm.jumpTo(SceneID::ISOMETRIC); break;
    case '7': sm.jumpTo(SceneID::CINEMATIC); break;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  GLUT Special Key Callback
// ─────────────────────────────────────────────────────────────────────────────
void specialKey(int key, int /*x*/, int /*y*/)
{
    if (key == GLUT_KEY_RIGHT)
        getSceneManager().jumpTo(
            static_cast<SceneID>((int)getSceneManager().currentScene() + 1));
    if (key == GLUT_KEY_LEFT) {
        int s = (int)getSceneManager().currentScene() - 1;
        if (s < 0) s = 0;
        getSceneManager().jumpTo(static_cast<SceneID>(s));
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  main
// ─────────────────────────────────────────────────────────────────────────────
int main(int argc, char* argv[])
{
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    // ── FreeGLUT initialisation ──────────────────────────────────────────
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_MULTISAMPLE);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutInitWindowPosition(
        (glutGet(GLUT_SCREEN_WIDTH)  - WINDOW_WIDTH)  / 2,
        (glutGet(GLUT_SCREEN_HEIGHT) - WINDOW_HEIGHT) / 2
    );

    int win = glutCreateWindow(
        "The Little Robot Builds a Digital Kingdom  |  CG Algorithms Demo");

    if (win == 0) {
        std::cerr << "Failed to create GLUT window.\n";
        return 1;
    }

    // ── OpenGL state ─────────────────────────────────────────────────────
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glLineWidth(1.0f);
    glPointSize(1.0f);

    // ── Register callbacks ───────────────────────────────────────────────
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKey);
    glutTimerFunc(0, timer, 0);

    // ── Print key-bindings ───────────────────────────────────────────────
    std::cout << "==========================================================\n"
              << " The Little Robot Builds a Digital Kingdom\n"
              << " Computer Graphics Algorithms Demonstration\n"
              << "==========================================================\n"
              << " SPACE / Right Arrow : Next scene\n"
              << " Left Arrow          : Previous scene\n"
              << " 1-7                 : Jump to scene 1-7\n"
              << " F                   : Toggle full-screen\n"
              << " ESC / Q             : Quit\n"
              << "==========================================================\n"
              << " Scenes:\n"
              << "  1. Intro\n"
              << "  2. DDA & Bresenham Line/Circle Drawing\n"
              << "  3. Scan-Line / Flood Fill / Boundary Fill\n"
              << "  4. Point / Cohen-Sutherland / Liang-Barsky / S-H Clipping\n"
              << "  5. Translation / Rotation / Scaling / Composite Transforms\n"
              << "  6. Isometric Projection\n"
              << "  7. Painter's Algorithm + Parallax + Fireworks\n"
              << "==========================================================\n";

    // ── Enter main loop ──────────────────────────────────────────────────
    glutMainLoop();

    return 0;
}
