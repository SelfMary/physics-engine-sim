#include "Render.cpp"
#include "PhysicsWorld.h"
#include "Projectile.h"
#include "Types.h"

#include <vector>
#include <iostream>
#include <cmath>
#include <algorithm>

int main()
{
    Renderer renderer(1000, 650, "Stage 2 - Parameter Sweep (Fixed)");

    if (!renderer.isRunning()) {
        std::cerr << "Renderer failed\n";
        return -1;
    }

    renderer.setWorldView(120, 60, 0.05f, 0.1f);

    const vec3d LAUNCH = {5.0f, 0.0f, 0.0f};

    BodyData ball{};
    ball.mass = 1.0f;
    ball.restitution = 0.6f;
    ball.radius = 0.5f;

    const float dt = 1.0f / 60.0f;
    const float DRAG = 0.02f;
    const float g = 9.81f;

    // ─────────────────────────────
    // sweep settings
    // ─────────────────────────────
    float speed = 30.0f;
    float angle = 45.0f;

    int sweepMode = 0; // 0 speed, 1 angle, 2 grid

    bool fired = false;
    bool landed = false;

    float landingX = 0.0f;

    std::vector<vec3d> trail;

    auto reset = [&]()
    {
        ball.position = LAUNCH;
        ball.velocity = {0,0,0};
        ball.force = {0,0,0};

        trail.clear();

        fired = false;
        landed = false;
        landingX = 0.0f;
    };

    reset();

    while (renderer.isRunning())
    {
        GLFWwindow* win = renderer.getWindow();

        // ───────────────── INPUT ─────────────────
        if (glfwGetKey(win, GLFW_KEY_1)) sweepMode = 0;
        if (glfwGetKey(win, GLFW_KEY_2)) sweepMode = 1;
        if (glfwGetKey(win, GLFW_KEY_3)) sweepMode = 2;

        if (glfwGetKey(win, GLFW_KEY_UP))    angle += 0.5f;
        if (glfwGetKey(win, GLFW_KEY_DOWN))  angle -= 0.5f;
        if (glfwGetKey(win, GLFW_KEY_RIGHT)) speed += 0.5f;
        if (glfwGetKey(win, GLFW_KEY_LEFT))  speed -= 0.5f;

        angle = std::clamp(angle, 5.0f, 85.0f);
        speed = std::clamp(speed, 1.0f, 60.0f);

        // ───────────────── LAUNCH ─────────────────
        if (!fired && glfwGetKey(win, GLFW_KEY_SPACE))
        {
            float r = angle * 3.14159f / 180.0f;

            ball.position = LAUNCH;
            ball.velocity = {
                speed * cosf(r),
                speed * sinf(r),
                0.0f
            };

            fired = true;
        }

        // ───────────────── PHYSICS (same style as your collision demo) ─────────────────
        if (fired)
        {
            ball.force.y = -g * ball.mass;

            Physics::integrate(ball, dt, DRAG);

            trail.push_back(ball.position);

            // landing detection
            if (!landed && ball.position.y <= 0.0f)
            {
                ball.position.y = 0.0f;
                landed = true;
                landingX = ball.position.x;
            }

            // ground response (small bounce damping)
            if (ball.position.y <= 0.0f)
            {
                ball.velocity.y *= -ball.restitution;
                ball.velocity.x *= 0.98f;
            }

            // reset after stop
            if (landed && std::abs(ball.velocity.y) < 0.1f)
            {
                reset();
            }
        }

        // SWEEP LOGIC (KEY PART)
        std::vector<std::string> info;

        if (sweepMode == 0)
        {
            info.push_back("MODE: SPEED SWEEP");

            for (float v = 10; v <= 50; v += 10)
            {
                float r = angle * 3.14159f / 180.0f;

                BodyData test = {};
                test.position = LAUNCH;
                test.velocity = {v*cosf(r), v*sinf(r), 0};

                float x = 0;

                for (int i = 0; i < 300; i++)
                {
                    test.force.y = -g * test.mass;
                    Physics::integrate(test, dt, 0.0f);

                    if (test.position.y <= 0)
                    {
                        x = test.position.x;
                        break;
                    }
                }

                info.push_back("v=" + std::to_string((int)v) +
                               " range=" + std::to_string((int)(x-LAUNCH.x)));
            }
        }

        if (sweepMode == 1)
        {
            info.push_back("MODE: ANGLE SWEEP");

            for (float a = 10; a <= 80; a += 10)
            {
                float r = a * 3.14159f / 180.0f;

                BodyData test = {};
                test.position = LAUNCH;
                test.velocity = {speed*cosf(r), speed*sinf(r), 0};

                float x = 0;

                for (int i = 0; i < 300; i++)
                {
                    test.force.y = -g * test.mass;
                    Physics::integrate(test, dt, 0.0f);

                    if (test.position.y <= 0)
                    {
                        x = test.position.x;
                        break;
                    }
                }

                info.push_back("θ=" + std::to_string((int)a) +
                               " range=" + std::to_string((int)(x-LAUNCH.x)));
            }
        }

        if (sweepMode == 2)
        {
            info.push_back("MODE: GRID SWEEP");

            for (float v = 10; v <= 40; v += 10)
            for (float a = 20; a <= 70; a += 10)
            {
                float r = a * 3.14159f / 180.0f;

                BodyData test = {};
                test.position = LAUNCH;
                test.velocity = {v*cosf(r), v*sinf(r), 0};

                float x = 0;

                for (int i = 0; i < 300; i++)
                {
                    test.force.y = -g * test.mass;
                    Physics::integrate(test, dt, 0.0f);

                    if (test.position.y <= 0)
                    {
                        x = test.position.x;
                        break;
                    }
                }

                info.push_back("v=" + std::to_string((int)v) +
                               " θ=" + std::to_string((int)a) +
                               " r=" + std::to_string((int)(x-LAUNCH.x)));
            }
        }

        // RENDER
        renderer.beginFrame();

        renderer.drawGround();
        renderer.drawBall(ball, ball.radius);
        renderer.drawTrail(trail);
        renderer.drawCannon(LAUNCH.x, LAUNCH.y, angle);

        // HUD box
        renderer.drawText("STAGE 2 - PARAMETER SWEEP", 20, 30, 1,1,1,2);

        renderer.drawText("Angle: " + std::to_string((int)angle), 20, 60, 1,1,0,1);
        renderer.drawText("Speed: " + std::to_string((int)speed), 20, 90, 1,1,0,1);

        renderer.drawText("Press 1/2/3 to change sweep", 20, 120, 0.8,0.8,1,1);

        int y = 160;
        for (auto& s : info)
        {
            renderer.drawText(s, 20, y, 0.8,1,0.8,1);
            y += 25;
        }

        if (landed)
        {
            renderer.drawText(
                "LANDING X: " + std::to_string((int)landingX),
                20, 500, 1,0.3,0.3,2
            );
        }

        renderer.endFrame();
    }

    return 0;
}