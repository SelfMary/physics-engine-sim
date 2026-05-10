#include "Render.cpp"
#include "PhysicsWorld.h"
#include "Types.h"
#include <vector>
#include <cmath>
#include <string>
#include <algorithm>

int main()
{
    Renderer renderer(1000, 650, "Stage 3 - Height Differential FIXED");

    renderer.setWorldView(120, 60, 0.05f, 0.1f);

    const vec3d LAUNCH = {5.0f, 0.0f, 0.0f};

    vec3d TARGET = {90.0f, 0.0f, 0.0f};

    BodyData ball{};
    ball.mass = 1.0f;
    ball.restitution = 0.6f;
    ball.radius = 0.5f;

    float angle = 45.0f;
    float speed = 30.0f;

    std::vector<vec3d> trail;

    bool fired = false;
    bool landed = false;
    float landingX = 0.0f;

    const float dt = 1.0f / 60.0f;
    const float DRAG = 0.05f;
    const float g = 9.81f;

    auto reset = [&]()
    {
        ball.position = LAUNCH;
        ball.velocity = {0,0,0};
        ball.force = {0,0,0};
        trail.clear();
        fired = false;
        landed = false;
    };

    reset();

    while (renderer.isRunning())
    {
        GLFWwindow* win = renderer.getWindow();

        // HEIGHT CONTROL
        if (glfwGetKey(win, GLFW_KEY_1)) TARGET.y = 0.0f;
        if (glfwGetKey(win, GLFW_KEY_2)) TARGET.y = 15.0f;
        if (glfwGetKey(win, GLFW_KEY_3)) TARGET.y = -15.0f;

        if (glfwGetKey(win, GLFW_KEY_R))
            reset();

        // FIRE
        if (!fired && glfwGetKey(win, GLFW_KEY_SPACE))
        {
            float r = angle * 3.14159265f / 180.0f;

            ball.position = LAUNCH;
            ball.velocity = {
                speed * cosf(r),
                speed * sinf(r),
                0
            };

            fired = true;
        }

        // PHYSICS
        if (fired)
        {
            ball.force.y -= g * ball.mass;
            Physics::integrate(ball, dt, DRAG);

            trail.push_back(ball.position);

            if (ball.position.y <= 0.0f)
            {
                ball.position.y = 0.0f;

                if (!landed)
                {
                    landed = true;
                    landingX = ball.position.x;
                }

                ball.velocity.y *= -ball.restitution;
                ball.velocity.x *= 0.98f;
            }
        }

        // HEIGHT DIFFERENCE FIX (KEY PART)
        float dy = TARGET.y - LAUNCH.y;
        float rad = angle * 3.14159265f / 180.0f;

        float v0x = speed * cosf(rad);
        float v0y = speed * sinf(rad);

        float disc = v0y*v0y - 2*g*dy;   // corrected height-aware term

        float tof = (disc > 0)
            ? (v0y + sqrt(disc)) / g
            : 0.0f;

        float predictedRange = v0x * tof;

        // RENDER
        renderer.beginFrame();

        renderer.drawGround();
        renderer.drawTrail(trail);
        renderer.drawBall(ball, ball.radius);

        renderer.drawCannon(LAUNCH.x, LAUNCH.y, angle);

        // VISUAL FIX: show launch + target height difference clearly
        renderer.drawBall({TARGET.x, TARGET.y, 0}, 0.7f, 1,0.2,0.2);
        renderer.drawBall({LAUNCH.x, LAUNCH.y, 0}, 0.7f, 0.2,1,0.2);

        // TEXT BOX
        renderer.drawText("STAGE 3 - HEIGHT DIFFERENTIAL", 20, 40, 1,1,1,2);

        renderer.drawText("Target Y: " + std::to_string((int)TARGET.y), 20, 80, 1,0.6,0.6,1.5);
        renderer.drawText("Launch Y: 0", 20, 110, 0.6,1,0.6,1.5);

        renderer.drawText("Speed: " + std::to_string((int)speed), 20, 140, 1,1,1,1.5);
        renderer.drawText("Angle: " + std::to_string((int)angle), 20, 170, 1,1,1,1.5);

        renderer.drawText("Predicted Range: " + std::to_string((int)predictedRange),
                          20, 210, 1,0.8,0.4,1.5);

        if (landed)
        {
            float realRange = landingX - LAUNCH.x;

            renderer.drawText("ACTUAL RANGE: " + std::to_string((int)realRange),
                              20, 250, 0.2,1,0.4,2);

            renderer.drawText("|",
                              200 + realRange * 3,
                              500,
                              1,0.2,0.2,3);
        }

        renderer.endFrame();
    }

    return 0;
}