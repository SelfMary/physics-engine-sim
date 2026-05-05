#include "Render.h"
#include "PhysicsWorld.h"
#include "Types.h"

#include <vector>
#include <cmath>
#include <string>
#include <algorithm>
#include <iostream>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ---------------- helpers ----------------
static float deg2rad(float d) { return d * M_PI / 180.0f; }
static float rad2deg(float r) { return r * 180.0f / M_PI; }

// ---------------- main ----------------
int main()
{
    Renderer renderer(1000, 650, "Stage 5 - Inverse Prediction");

    renderer.setWorldView(120, 60, 0.05f, 0.1f);

    const float g = 9.81f;

    // launch + target
    vec3d launch = {5, 0, 0};
    vec3d target = {90, 0, 0};

    BodyData ball{};
    ball.mass = 1.0f;
    ball.radius = 0.5f;

    std::vector<vec3d> trail;

    bool fired = false;

    float fixedSpeed = 30.0f;
    float fixedAngle = 45.0f;

    float angleHigh = 45.0f;
    float angleLow  = 45.0f;
    float requiredSpeed = 0.0f;

    int mode = 1; // 1 angle solve, 2 velocity solve, 3 grid search

    // reset
    auto reset = [&]()
    {
        ball.position = launch;
        ball.velocity = {0,0,0};
        trail.clear();
        fired = false;
    };

    reset();

    // ---------------- inverse physics ----------------
    auto computeAngles = [&](float v)
    {
        float dx = target.x - launch.x;
        float dy = target.y - launch.y;

        float v2 = v * v;
        float disc = v2 * v2 - g * (g * dx * dx + 2 * dy * v2);

        if (disc < 0)
        {
            angleHigh = angleLow = 45;
            return;
        }

        float root = sqrtf(disc);

        float t1 = atanf((v2 + root) / (g * dx));
        float t2 = atanf((v2 - root) / (g * dx));

        angleHigh = rad2deg(t1);
        angleLow  = rad2deg(t2);
    };

    auto computeVelocity = [&](float angleDeg)
    {
        float dx = target.x - launch.x;
        float dy = target.y - launch.y;

        float t = deg2rad(angleDeg);
// powf(x, y)= x^y
        float denom = 2.0f * (dx * tanf(t) - dy) * powf(cosf(t), 2);

        if (denom <= 0.0f)
        {
            requiredSpeed = 0.0f;
            return;
        }

        requiredSpeed = sqrtf((g * dx * dx) / denom);
    };

    // ---------------- loop ----------------
    while (renderer.isRunning())
    {
        GLFWwindow* win = renderer.getWindow();

        // mode switch
        if (glfwGetKey(win, GLFW_KEY_1)) mode = 1;
        if (glfwGetKey(win, GLFW_KEY_2)) mode = 2;
        if (glfwGetKey(win, GLFW_KEY_3)) mode = 3;

        // move target
        if (glfwGetKey(win, GLFW_KEY_LEFT))  target.x -= 0.5f;
        if (glfwGetKey(win, GLFW_KEY_RIGHT)) target.x += 0.5f;
        if (glfwGetKey(win, GLFW_KEY_UP))    target.y += 0.5f;
        if (glfwGetKey(win, GLFW_KEY_DOWN))  target.y -= 0.5f;

        if (glfwGetKey(win, GLFW_KEY_R))
            reset();

        // ---------------- MODE 1: angle solver ----------------
        if (mode == 1)
        {
            computeAngles(fixedSpeed);

            if (!fired && glfwGetKey(win, GLFW_KEY_SPACE))
            {
                float rad = deg2rad(angleHigh);

                ball.position = launch;
                ball.velocity = {
                    fixedSpeed * cosf(rad),
                    fixedSpeed * sinf(rad),
                    0.0f
                };

                fired = true;
            }
        }

        // ---------------- MODE 2: velocity solver ----------------
        if (mode == 2)
        {
            computeVelocity(fixedAngle);

            if (!fired && glfwGetKey(win, GLFW_KEY_SPACE))
            {
                float rad = deg2rad(fixedAngle);

                ball.position = launch;
                ball.velocity = {
                    requiredSpeed * cosf(rad),
                    requiredSpeed * sinf(rad),
                    0.0f
                };

                fired = true;
            }
        }


        // ---------------- physics ----------------
        if (fired)
        {
            ball.force.y -= g * ball.mass;

            Physics::integrate(ball, 1.0f/60.0f, 0.02f);

            trail.push_back(ball.position);

            if (ball.position.y <= 0)
            {
                ball.position.y = 0;
                fired = false;
            }
        }

        // ---------------- render ----------------
        renderer.beginFrame();

        renderer.drawGround();

        renderer.drawBall(ball, 0.5f);
        renderer.drawTrail(trail);

        renderer.drawCannon(launch.x, launch.y, fixedAngle);

        // FIXED target rendering
        BodyData targetBody{};
        targetBody.position = target;
        targetBody.radius = 0.7f;

        renderer.drawBall(targetBody, 0.7f, 1.0f, 0.0f, 0.0f);

        // ---------------- UI ----------------
        std::string modeText =
            (mode == 1) ? "ANGLE SOLVER" :
            (mode == 2) ? "VELOCITY SOLVER" :
                          "GRID SEARCH";

        renderer.drawText("STAGE 5 - INVERSE PREDICTION", 20, 40, 1,1,1,2);
        renderer.drawText("Mode: " + modeText, 20, 70, 0.8,0.8,1,1.5);

        renderer.drawText("Target X: " + std::to_string((int)target.x), 20, 110, 1,0.8,0.5,1.5);
        renderer.drawText("Target Y: " + std::to_string((int)target.y), 20, 140, 1,0.8,0.5,1.5);

        renderer.drawText("High Angle: " + std::to_string(angleHigh), 20, 180, 0.6,1,0.6,1.5);
        renderer.drawText("Low Angle: " + std::to_string(angleLow), 20, 210, 0.6,1,0.6,1.5);

        renderer.drawText("Required Speed: " + std::to_string(requiredSpeed), 20, 240, 0.9,0.9,0.9,1.5);

        renderer.endFrame();
    }

    return 0;
}