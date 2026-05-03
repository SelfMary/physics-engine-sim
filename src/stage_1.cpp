#include "Render.h"
#include "PhysicsWorld.h"
#include "Projectile.h"
#include "Types.h"
#include <vector>
#include <cmath>
#include <string>

enum class Mode { IDEAL, DRAG };

int main()
{
    Renderer renderer(1000, 650, "Stage 1 - Fixed Target");

    renderer.setWorldView(120, 60, 0.05f, 0.1f);

    const vec3d LAUNCH = {5,0,0};

    BodyData ball{};
    ball.mass = 1;
    ball.radius = 0.5;

    float angle = 45;
    float speed = 30;

    Mode mode = Mode::IDEAL;

    bool fired = false;
    std::vector<vec3d> trail;

    const float dt = 1.0f / 60.0f;
    const float DRAG = 0.1f;
    const float g = 9.81f;

    auto reset = [&]()
    {
        ball.position = LAUNCH;
        ball.velocity = {0,0,0};
        ball.force = {0,0,0};
        trail.clear();
        fired = false;
    };

    reset();

    while(renderer.isRunning())
    {
        GLFWwindow* win = renderer.getWindow();

        if (glfwGetKey(win, GLFW_KEY_1)) mode = Mode::IDEAL;
        if (glfwGetKey(win, GLFW_KEY_2)) mode = Mode::DRAG;

        if (!fired && glfwGetKey(win, GLFW_KEY_SPACE))
        {
            float r = angle * 3.14159f / 180.0f;

            ball.position = LAUNCH;
            ball.velocity = {
                speed * cosf(r),
                speed * sinf(r),
                0
            };

            fired = true;
        }

        if (fired)
        {
            ball.force.y -= g * ball.mass;

            float drag = (mode == Mode::DRAG) ? DRAG : 0.0f;
            Physics::integrate(ball, dt, drag);

            trail.push_back(ball.position);

            if (ball.position.y <= 0)
                reset();
        }

        // RENDER
        renderer.beginFrame();

        renderer.drawGround();
        renderer.drawBall(ball, 0.5f);
        renderer.drawTrail(trail);
        renderer.drawCannon(LAUNCH.x, LAUNCH.y, angle);

        // STATS BOX
        float r = angle * 3.14159f / 180.0f;
        float vx = speed * cosf(r);
        float vy = speed * sinf(r);

        std::string modeText = (mode == Mode::IDEAL) ? "IDEAL" : "DRAG";

        renderer.drawText("STAGE 1 - PHYSICS DATA", 20, 40, 1,1,1,2);
        renderer.drawText("Mode: " + modeText, 20, 70, 0.8,0.8,1,1.5);

        renderer.drawText("Speed: " + std::to_string((int)speed) + " m/s", 20, 100, 0.9,0.9,0.9,1.5);
        renderer.drawText("Angle: " + std::to_string((int)angle) + " deg", 20, 130, 0.9,0.9,0.9,1.5);

        renderer.drawText("Vx: " + std::to_string((int)vx), 20, 160, 0.6,1,0.6,1.5);
        renderer.drawText("Vy: " + std::to_string((int)vy), 20, 190, 0.6,1,0.6,1.5);

        renderer.drawText("g = 9.81 m/s^2", 20, 220, 1,0.8,0.5,1.5);
        renderer.drawText("dt = 1/60 s", 20, 250, 1,0.8,0.5,1.5);

        renderer.endFrame();
    }
}