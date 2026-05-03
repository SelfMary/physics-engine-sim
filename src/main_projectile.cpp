#include "Render.h"
#include "PhysicsWorld.h"
#include "Collision.h"
#include "Projectile.h"
#include "Types.h"

#include <vector>
#include <iostream>
#include <cmath>
#include <algorithm>

// ───────────────────────────────
// Mode
// ───────────────────────────────
enum class Mode { PROJECTILE, BASIC };
Mode mode = Mode::PROJECTILE;

int main()
{
    Renderer renderer(1000, 650, "Physics Engine");

    if (!renderer.isRunning()) {
        std::cerr << "Renderer failed\n";
        return -1;
    }

    float worldW = 120.0f;
    float worldH = 60.0f;
    renderer.setWorldView(worldW, worldH, 0.05f, 0.1f);

    // Projectile state
    const vec3d LAUNCH_POS = {5.0f, 0.0f, 0.0f};

    float angle_deg = 45.0f;
    float speed     = 25.0f;

    BodyData ball{};
    ball.mass        = 1.0f;
    ball.restitution = 0.6f;
    ball.radius      = 0.5f;

    std::vector<vec3d> trail;
    std::vector<BounceSegment> bounceSegs;

    float maxHeight      = 0.0f;
    float totalHorizDist = 0.0f;

    float lastBounceX = LAUNCH_POS.x;
    bool wasAirborne  = false;
    bool fired        = false;
    bool showLabels   = false;

    float stopTimer = 0.0f;

    bool hasLanded = false;
    float landingX = 0.0f;

    auto resetBall = [&]() {
        ball.position    = LAUNCH_POS;
        ball.initial_pos = LAUNCH_POS;
        ball.velocity    = {0,0,0};
        ball.initial_vel = {0,0,0};
        ball.force       = {0,0,0};
        ball.distance    = 0.0f;

        trail.clear();
        bounceSegs.clear();

        maxHeight = 0.0f;
        totalHorizDist = 0.0f;

        lastBounceX = LAUNCH_POS.x;
        wasAirborne = false;

        fired = false;
        showLabels = false;

        stopTimer = 0.0f;

        hasLanded = false;
        landingX = 0.0f;
    };

    resetBall();

    // Basic mode
    BodyData ballA{}, ballB{};

    ballA.position = {20.0f, 20.0f, 0.0f};
    ballA.velocity = {8.0f, 0.0f, 0.0f};
    ballA.mass = 1.0f;
    ballA.restitution = 0.8f;
    ballA.radius = 2.0f;

    ballB.position = {80.0f, 20.0f, 0.0f};
    ballB.velocity = {-5.0f, 0.0f, 0.0f};
    ballB.mass = 2.0f;
    ballB.restitution = 0.8f;
    ballB.radius = 3.0f;

    auto resetBasic = [&]() {
        ballA.position = {20.0f, 20.0f, 0.0f};
        ballA.velocity = {8.0f, 0.0f, 0.0f};

        ballB.position = {80.0f, 20.0f, 0.0f};
        ballB.velocity = {-5.0f, 0.0f, 0.0f};
    };

    resetBasic();

    // Timing
    const float dt = 1.0f / 60.0f;
    const float DRAG = 0.1f;

    bool prevSpace=false, prevR=false, prevC=false,
         prevB=false, prevP=false;

    // ───────────────── MAIN LOOP ─────────────────
    while (renderer.isRunning())
    {
        GLFWwindow* win = renderer.getWindow();

        bool curUp    = glfwGetKey(win, GLFW_KEY_UP)    == GLFW_PRESS;
        bool curDown  = glfwGetKey(win, GLFW_KEY_DOWN)  == GLFW_PRESS;
        bool curLeft  = glfwGetKey(win, GLFW_KEY_LEFT)  == GLFW_PRESS;
        bool curRight = glfwGetKey(win, GLFW_KEY_RIGHT) == GLFW_PRESS;
        bool curSpace = glfwGetKey(win, GLFW_KEY_SPACE) == GLFW_PRESS;
        bool curR     = glfwGetKey(win, GLFW_KEY_R)     == GLFW_PRESS;
        bool curC     = glfwGetKey(win, GLFW_KEY_C)     == GLFW_PRESS;
        bool curB     = glfwGetKey(win, GLFW_KEY_B)     == GLFW_PRESS;
        bool curP     = glfwGetKey(win, GLFW_KEY_P)     == GLFW_PRESS;
        bool curZ     = glfwGetKey(win, GLFW_KEY_Z)     == GLFW_PRESS;
        bool curX     = glfwGetKey(win, GLFW_KEY_X)     == GLFW_PRESS;

        // INPUT
        if (mode == Mode::PROJECTILE && !fired)
        {
            // smooth cannon control
            if (curUp)
                angle_deg += 90.0f * dt;

            if (curDown)
                angle_deg -= 90.0f * dt;

            if (curRight)
                speed += 30.0f * dt;

            if (curLeft)
                speed -= 30.0f * dt;

            angle_deg = std::clamp(angle_deg, 5.0f, 85.0f);
            speed     = std::clamp(speed, 1.0f, 60.0f);
        }

        // mode switching
        if (curB && !prevB) { mode = Mode::BASIC; resetBasic(); }
        if (curP && !prevP) { mode = Mode::PROJECTILE; resetBall(); }

        if (curR && !prevR)
            (mode == Mode::PROJECTILE) ? resetBall() : resetBasic();

        // zoom
        if (curZ) {
            worldW += 1.0f; worldH += 0.5f;
            renderer.setWorldView(worldW, worldH, 0.05f, 0.1f);
        }

        if (curX) {
            worldW = fmaxf(20.0f, worldW - 1.0f);
            worldH = fmaxf(10.0f, worldH - 0.5f);
            renderer.setWorldView(worldW, worldH, 0.05f, 0.1f);
        }

        // PROJECTILE CASE
        if (mode == Mode::PROJECTILE)
        {
            if (!fired && curSpace && !prevSpace)
            {
                float rad = angle_deg * 3.14159265f / 180.0f;
                ball.velocity = {
                    speed * cosf(rad),
                    speed * sinf(rad),
                    0.0f
                };
                ball.initial_vel = ball.velocity;

                fired = true;
                wasAirborne = true;
            }

            if (fired)
            {
                ball.force.y += -9.81f * ball.mass;
                Physics::integrate(ball, dt, DRAG);

                maxHeight = std::max(maxHeight, ball.position.y);
                totalHorizDist = ball.position.x - LAUNCH_POS.x;

                // landing detection FIXED
                if (!hasLanded && ball.position.y <= 0.0f && ball.velocity.y <= 0.5f)
                {
                    hasLanded = true;
                    landingX = ball.position.x;
                }

                if (ball.position.y < 0.0f)
                {
                    ball.position.y = 0.0f;
                    ball.velocity.y *= -ball.restitution;
                    ball.velocity.x *= 0.98f;
                }

                trail.push_back(ball.position);
            }

            // RENDER
            renderer.beginFrame();

            renderer.drawGround();
            renderer.drawCannon(LAUNCH_POS.x, LAUNCH_POS.y, angle_deg);

            if (!trail.empty())
                renderer.drawTrail(trail);

            renderer.drawBall(ball, ball.radius);
            renderer.drawHUD(angle_deg, speed);
            renderer.drawStatsBox(maxHeight, totalHorizDist);

            // landing marker
            if (hasLanded)
            {
                float fw, fh;
                glfwGetFramebufferSize(renderer.getWindow(), (int*)&fw, (int*)&fh);

                float px = (landingX / worldW) * fw;
                float py = fh - 20.0f;

                renderer.drawText("LANDING POINT", 20, 80, 1,0.3,0.3,2);
                renderer.drawText(
                    ("RANGE: " + std::to_string((int)(landingX - LAUNCH_POS.x)) + " m"),
                    20, 110, 0.2,1,0.4,2
                );

                renderer.drawText("|", px, py, 1,0.2,0.2,3);
            }

            if (showLabels)
                renderer.drawBounceLabels(bounceSegs);

            renderer.endFrame();
        }

        // BASIC CASE
        else
        {
            ballA.force.y -= 9.81f * ballA.mass;
            ballB.force.y -= 9.81f * ballB.mass;

            Physics::integrate(ballA, dt, DRAG);
            Physics::integrate(ballB, dt, DRAG);

            renderer.beginFrame();
            renderer.drawGround();
            renderer.drawBall(ballA, ballA.radius, 1,0.4,0.1);
            renderer.drawBall(ballB, ballB.radius, 0.2,0.5,1);
            renderer.endFrame();
        }

        prevSpace = curSpace;
        prevR = curR;
        prevC = curC;
        prevB = curB;
        prevP = curP;
    }

    return 0;
}