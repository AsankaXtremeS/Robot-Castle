/**
 * Robot.h
 * The cute robot character – assembled from transformed primitives.
 * Supports walking, pointing, waving, and celebration animations.
 */

#pragma once
#include "Common.h"

// ─────────────────────────────────────────────────────────────────────────────
enum class RobotAction {
    IDLE,
    WALK,
    POINT,
    WAVE,
    CELEBRATE,
    DRAW        // arm sweeps drawing motion
};

// ─────────────────────────────────────────────────────────────────────────────
class Robot {
public:
    Robot();

    float x, y;          // world position (centre-bottom of robot)
    float scale;         // size multiplier
    float facingDir;     // +1 = right, -1 = left
    RobotAction action;
    float actionTime;    // elapsed time in current action

    void update(float dt);
    void draw() const;

    // High-level motion helpers
    void walkTo(float targetX, float targetY, float dt);
    void pointAt(float tx, float ty);
    void startAction(RobotAction a);

private:
    // Animated joint angles (degrees)
    float m_legAngle;
    float m_armAngle;
    float m_headBob;
    float m_eyeGlow;     // 0-1 pulsing glow

    // Draw sub-parts (all positions relative to robot origin)
    void drawBody(float ox, float oy) const;
    void drawHead(float ox, float oy) const;
    void drawArm(float ox, float oy, bool leftSide) const;
    void drawLeg(float ox, float oy, bool leftSide) const;
    void drawAntenna(float ox, float oy) const;
};
