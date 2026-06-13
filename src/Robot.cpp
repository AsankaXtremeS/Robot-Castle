/**
 * Robot.cpp
 * The cute robot character – drawn using OpenGL primitives and custom 3×3
 * transformation matrices for animated limb movement.
 */

#include "../include/Robot.h"
#include "../include/Renderer.h"
#include "../include/Algorithms/Transformations.h"
#include <GL/freeglut.h>
#include <cmath>

// ─────────────────────────────────────────────────────────────────────────────
Robot::Robot()
    : x(100), y(150), scale(1.0f), facingDir(1.0f),
      action(RobotAction::IDLE), actionTime(0),
      m_legAngle(0), m_armAngle(0), m_headBob(0), m_eyeGlow(0)
{}

// ─────────────────────────────────────────────────────────────────────────────
void Robot::update(float dt)
{
    actionTime += dt;

    switch (action) {
    case RobotAction::IDLE:
        m_legAngle = 0;
        m_armAngle = std::sin(actionTime * 1.5f) * 5.0f;
        m_headBob  = std::sin(actionTime * 2.0f) * 2.0f;
        break;

    case RobotAction::WALK:
        m_legAngle = std::sin(actionTime * 6.0f) * 20.0f;
        m_armAngle = std::sin(actionTime * 6.0f) * 15.0f;
        m_headBob  = std::abs(std::sin(actionTime * 6.0f)) * 3.0f;
        break;

    case RobotAction::POINT:
        m_armAngle = -30.0f;
        m_legAngle = 0;
        m_headBob  = 0;
        break;

    case RobotAction::WAVE:
        m_armAngle = -45.0f + std::sin(actionTime * 8.0f) * 20.0f;
        m_legAngle = 0;
        break;

    case RobotAction::CELEBRATE:
        m_armAngle = -60.0f + std::sin(actionTime * 10.0f) * 30.0f;
        m_legAngle = std::sin(actionTime * 10.0f) * 15.0f;
        m_headBob  = std::abs(std::sin(actionTime * 10.0f)) * 5.0f;
        break;

    case RobotAction::DRAW:
        m_armAngle = -40.0f + std::sin(actionTime * 3.0f) * 10.0f;
        m_legAngle = 0;
        m_headBob  = std::sin(actionTime * 2.0f) * 2.0f;
        break;
    }

    m_eyeGlow = 0.5f + 0.5f * std::sin(actionTime * 4.0f);
}

// ─────────────────────────────────────────────────────────────────────────────
void Robot::walkTo(float targetX, float /*targetY*/, float dt)
{
    float dx = targetX - x;
    float speed = 80.0f * scale;
    if (std::abs(dx) > 2.0f) {
        facingDir = (dx > 0) ? 1.0f : -1.0f;
        x += (dx > 0 ? 1 : -1) * std::min(speed * dt, std::abs(dx));
        startAction(RobotAction::WALK);
    } else {
        startAction(RobotAction::IDLE);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
void Robot::pointAt(float tx, float /*ty*/)
{
    facingDir = (tx > x) ? 1.0f : -1.0f;
    startAction(RobotAction::POINT);
}

// ─────────────────────────────────────────────────────────────────────────────
void Robot::startAction(RobotAction a)
{
    if (action != a) { action = a; actionTime = 0; }
}

// ─────────────────────────────────────────────────────────────────────────────
//  Draw helpers – all coordinates are relative to the robot's local origin
// ─────────────────────────────────────────────────────────────────────────────

// Draw body (torso)
void Robot::drawBody(float ox, float oy) const
{
    float bw = 30 * scale, bh = 36 * scale;
    // Main torso – rounded rectangle approximation
    Renderer::drawFilledRect(ox - bw/2, oy, bw, bh, Palette::robotBody());
    // Chest panel
    Renderer::drawFilledRect(ox - bw*0.3f, oy + bh*0.2f,
                              bw*0.6f, bh*0.4f, Color(0.2f,0.4f,0.75f));
    // Panel details
    Renderer::drawFilledCircle(ox, oy + bh*0.45f, 4*scale, Palette::robotEye());
    // Outline
    Renderer::drawRect(ox - bw/2, oy, bw, bh, Color(0.2f,0.3f,0.5f));
}

// Draw head
void Robot::drawHead(float ox, float oy) const
{
    float hw = 26 * scale, hh = 22 * scale;
    float hx = ox - hw/2, hy = oy + m_headBob * scale;

    // Head box
    Renderer::drawFilledRect(hx, hy, hw, hh, Palette::robotBody());
    Renderer::drawRect(hx, hy, hw, hh, Color(0.2f,0.3f,0.5f));

    // Antenna
    Renderer::drawLine(ox, hy + hh, ox, hy + hh + 10*scale,
                       Palette::robotJoint(), 2.0f);
    Renderer::drawFilledCircle(ox, hy + hh + 12*scale,
                                4*scale, Palette::yellow());

    // Eyes
    float eyeY = hy + hh * 0.6f;
    float eyeR = 5 * scale;
    // Eye whites
    Renderer::drawFilledCircle(ox - hw*0.25f, eyeY, eyeR, Color(0.9f,0.95f,1.0f));
    Renderer::drawFilledCircle(ox + hw*0.25f, eyeY, eyeR, Color(0.9f,0.95f,1.0f));
    // Pupils (glow)
    Color eyeCol = Color(0.1f, m_eyeGlow, 0.9f);
    Renderer::drawFilledCircle(ox - hw*0.25f + facingDir*1.5f*scale,
                                eyeY, eyeR*0.55f, eyeCol);
    Renderer::drawFilledCircle(ox + hw*0.25f + facingDir*1.5f*scale,
                                eyeY, eyeR*0.55f, eyeCol);

    // Mouth – wavy line (happy expression)
    float mY = hy + hh * 0.2f;
    for (int i = -4; i < 4; ++i) {
        float mx1 = ox + i * 3 * scale;
        float mx2 = ox + (i+1) * 3 * scale;
        float my1 = mY + std::sin(i * 1.2f) * 2 * scale;
        float my2 = mY + std::sin((i+1) * 1.2f) * 2 * scale;
        Renderer::drawLine(mx1, my1, mx2, my2, Color(0.15f,0.25f,0.5f), 2.0f);
    }
}

// Draw one arm
void Robot::drawArm(float ox, float oy, bool leftSide) const
{
    float side    = leftSide ? -1.0f : 1.0f;
    float shoulderX = ox + side * 17 * scale;
    float shoulderY = oy + 30 * scale;

    float angle   = (leftSide ? -m_armAngle : m_armAngle) * facingDir;
    float armLen  = 22 * scale;
    float handR   = 6 * scale;

    float endX = shoulderX + std::sin(toRad(angle)) * armLen * side;
    float endY = shoulderY + std::cos(toRad(angle)) * armLen * -1;

    // Upper arm
    Renderer::drawLine(shoulderX, shoulderY, endX, endY,
                       Palette::robotJoint(), 5.0f * scale);
    // Hand (circle)
    Renderer::drawFilledCircle(endX, endY, handR, Palette::robotBody());
    Renderer::drawCircle(endX, endY, handR, Palette::robotJoint(), 16);
    // Shoulder joint
    Renderer::drawFilledCircle(shoulderX, shoulderY, 5*scale, Palette::robotJoint());
}

// Draw one leg
void Robot::drawLeg(float ox, float oy, bool leftSide) const
{
    float side  = leftSide ? -1.0f : 1.0f;
    float hipX  = ox + side * 9 * scale;
    float hipY  = oy;
    float angle = (leftSide ? m_legAngle : -m_legAngle);
    float legLen = 24 * scale;

    float kneeX = hipX + std::sin(toRad(angle)) * legLen * 0.5f;
    float kneeY = hipY - legLen * 0.5f;
    float footX = kneeX + std::sin(toRad(angle * 0.5f)) * legLen * 0.5f;
    float footY = kneeY - legLen * 0.5f;

    // Thigh
    Renderer::drawLine(hipX, hipY, kneeX, kneeY,
                       Palette::robotJoint(), 4.5f * scale);
    // Shin
    Renderer::drawLine(kneeX, kneeY, footX, footY,
                       Palette::robotBody(), 4.0f * scale);
    // Foot
    Renderer::drawFilledRect(footX - 8*scale, footY - 5*scale,
                              16*scale, 5*scale, Palette::robotJoint());
    // Knee joint
    Renderer::drawFilledCircle(kneeX, kneeY, 4*scale, Palette::robotJoint());
}

// ─────────────────────────────────────────────────────────────────────────────
void Robot::draw() const
{
    glPushMatrix();
    glTranslatef(x, y, 0);
    glScalef(facingDir, 1.0f, 1.0f); // flip for direction

    float ox = 0, oy = 0; // local robot origin

    // Legs (behind body)
    drawLeg(ox, oy, true);
    drawLeg(ox, oy, false);

    // Body
    float bodyH = 36 * scale;
    drawBody(ox, oy);

    // Head
    drawHead(ox, oy + bodyH);

    // Arms (on top of body)
    drawArm(ox, oy, true);
    drawArm(ox, oy, false);

    glPopMatrix();
}
