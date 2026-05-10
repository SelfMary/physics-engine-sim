#include "Render.cpp"
#include "PhysicsWorld.h"
#include "Collision.h"
#include "Types.h"

#include <vector>
#include <iostream>
#include <cmath>
#include <algorithm>

int main()
{
    Renderer renderer(1000, 650, "Stage 4 - Full Integration + Data");

    if (!renderer.isRunning())
        return -1;

    renderer.setWorldView(120.0f, 60.0f, 0.05f, 0.1f);

    const vec3d LAUNCH = {5.0f, 0.0f, 0.0f};

    // Projectile
    BodyData ball{};
    ball.mass = 1.0f;
    ball.radius = 0.5f;
    ball.restitution = 0.6f;

    // Target
    BodyData target{};
    target.position = {80.0f, 10.0f, 0.0f};
    target.velocity = {-4.0f, 0.0f, 0.0f};
    target.mass = 2.0f;
    target.radius = 2.0f;
    target.restitution = 0.8f;

    std::vector<vec3d> trail;

    float angle = 45.0f;
    float speed = 30.0f;

    bool fired = false;
    bool collision = false;

    const float dt = 1.0f / 60.0f;
    const float DRAG = 0.05f;
    const float g = 9.81f;

    auto reset = [&]()
    {
        ball.position = LAUNCH;
        ball.velocity = {0,0,0};
        ball.force = {0,0,0};

        target.position = {80.0f, 10.0f, 0.0f};
        target.velocity = {-4.0f, 0.0f, 0.0f};

        trail.clear();
        fired = false;
        collision = false;
    };

    reset();

    while (renderer.isRunning())
    {
        GLFWwindow* win = renderer.getWindow();

        bool up    = glfwGetKey(win, GLFW_KEY_UP) == GLFW_PRESS;
        bool down  = glfwGetKey(win, GLFW_KEY_DOWN) == GLFW_PRESS;
        bool left  = glfwGetKey(win, GLFW_KEY_LEFT) == GLFW_PRESS;
        bool right = glfwGetKey(win, GLFW_KEY_RIGHT) == GLFW_PRESS;

        bool space = glfwGetKey(win, GLFW_KEY_SPACE) == GLFW_PRESS;
        bool resetK = glfwGetKey(win, GLFW_KEY_R) == GLFW_PRESS;

        static bool prevSpace = false;
        static bool prevReset = false;

        if (resetK && !prevReset)
            reset();

        prevReset = resetK;

        // INPUT
        if (!fired)
        {
            if (up)   angle += 60.0f * dt;
            if (down) angle -= 60.0f * dt;
            if (left) speed -= 30.0f * dt;
            if (right)speed += 30.0f * dt;

            angle = std::clamp(angle, 5.0f, 85.0f);
            speed = std::clamp(speed, 5.0f, 60.0f);
        }

        // FIRE
        if (!fired && space && !prevSpace)
        {
            float rad = angle * 3.1415926f / 180.0f;

            ball.position = LAUNCH;
            ball.velocity = {
                speed * cosf(rad),
                speed * sinf(rad),
                0
            };

            fired = true;
        }

        prevSpace = space;

        // PHYSICS
        if (fired)
        {
            ball.force.y = -g * ball.mass;
            target.force.y = -g * target.mass;

            Physics::integrate(ball, dt, DRAG);
            Physics::integrate(target, dt, DRAG);

            // ground
            if (ball.position.y < 0)
            {
                ball.position.y = 0;
                ball.velocity.y *= -ball.restitution;
                ball.velocity.x *= 0.98f;
            }

            if (target.position.y < 0)
            {
                target.position.y = 0;
                target.velocity.y *= -target.restitution;
            }

            // collision
            if (CollisionDetection::OverlapCheck(ball, target, ball.radius, target.radius))
            {
                vec3d n = CollisionDetection::contactNormal(ball, target);
                float e = std::min(ball.restitution, target.restitution);

                CollisionResponse::resolve(ball, target, n, e);

                collision = true;
            }

            trail.push_back(ball.position);
        }

        // DATA COMPUTATION
        float vx = ball.velocity.x;
        float vy = ball.velocity.y;

        float speedNow = sqrt(vx*vx + vy*vy);

        float distToTarget =
            sqrt(pow(ball.position.x - target.position.x, 2) +
                 pow(ball.position.y - target.position.y, 2));

        float range = ball.position.x - LAUNCH.x;

        // RENDER
        renderer.beginFrame();

        renderer.drawGround();

        renderer.drawCannon(LAUNCH.x, LAUNCH.y, angle);

        renderer.drawBall(ball, 0.5f, 1, 0.5f, 0.1f);
        renderer.drawBall(target, target.radius, 0.2f, 0.6f, 1);

        renderer.drawTrail(trail);

        // HUD
        renderer.drawText("STAGE 4 - FULL SIMULATION", 20, 40, 1,1,1,2);

        renderer.drawText("Angle: " + std::to_string((int)angle) + " deg", 20, 80, 0.9,0.9,0.9,1.5);
        renderer.drawText("Speed: " + std::to_string((int)speed) + " m/s", 20, 110, 0.9,0.9,0.9,1.5);

        renderer.drawText("Ball Speed: " + std::to_string((int)speedNow), 20, 150, 0.6,1,0.6,1.5);
        renderer.drawText("Range: " + std::to_string((int)range) + " m", 20, 180, 0.6,1,0.6,1.5);

        renderer.drawText("Distance to Target: " + std::to_string((int)distToTarget), 20, 210, 1,0.8,0.4,1.5);

        renderer.drawText(
            collision ? "COLLISION: YES" : "COLLISION: NO",
            20, 250,
            collision ? 1.0f : 0.6f,
            collision ? 0.2f : 0.6f,
            0.2f,
            2.0f
        );

        renderer.endFrame();
    }

    return 0;
}