// g++ src/main_basic.cpp src/Render.cpp  src/glad.c -Iinclude -lglfw -lGL -ldl -lpthread -o basic_sim

#include "Render.h"
#include "PhysicsWorld.h"
#include "Collision.h"
#include "Types.h"
#include <vector>
#include <iostream>
#include <algorithm>

int main()
{
    Renderer renderer(800, 600, "Stable Collision Demo");
    if (!renderer.isRunning()) {
        std::cerr << "Renderer failed\n";
        return -1;
    }

    float worldW = 100.0f;
    float worldH = 60.0f;
    renderer.setWorldView(worldW, worldH, 0.05f, 0.1f);

    // Small balls
    BodyData ballA{};
    ballA.position    = {30.0f, 5.0f, 0.0f};
    ballA.initial_pos = ballA.position;
    ballA.velocity    = {10.0f, 0.0f, 0.0f};
    ballA.mass        = 2.0f;
    ballA.restitution = 0.8f;
    ballA.radius      = 1.0f;

    BodyData ballB{};
    ballB.position    = {70.0f, 5.0f, 0.0f};
    ballB.initial_pos = ballB.position;
    ballB.velocity    = {-5.0f, 0.0f, 0.0f};
    ballB.mass        = 1.0f;
    ballB.restitution = 0.8f;
    ballB.radius      = 1.0f;

    const float dt = 1.0f / 60.0f;
    const float DRAG = 0.01f;
    const float VEL_EPSILON = 0.05f; // threshold for “resting” contact

    while (renderer.isRunning())
    {
        // gravity
        ballA.force.y = -9.81f * ballA.mass;
        ballB.force.y = -9.81f * ballB.mass;

        // integrate physics
        Physics::integrate(ballA, dt, DRAG);
        Physics::integrate(ballB, dt, DRAG);

        // ground collisions with damping
        auto handleGround = [&](BodyData& b) {
            if (b.position.y < 0.0f) {
                b.position.y = 0.0f;
                if (std::abs(b.velocity.y) < VEL_EPSILON) b.velocity.y = 0.0f;
                else b.velocity.y *= -0.6f;
            }
        };
        handleGround(ballA);
        handleGround(ballB);

        // sphere-sphere collision
        if (CollisionDetection::OverlapCheck(ballA, ballB, ballA.radius, ballB.radius))
        {
            vec3d normal = CollisionDetection::contactNormal(ballA, ballB);
            float penetration = CollisionDetection::penetrationDepth(ballA, ballB, ballA.radius, ballB.radius);
            float e = std::min(ballA.restitution, ballB.restitution);

            // Positional correction first
            CollisionResponse::positionalCorrection(ballA, ballB, normal, penetration);

            //Relative velocity along normal
            vec3d rv = {
                ballB.velocity.x - ballA.velocity.x,
                ballB.velocity.y - ballA.velocity.y,
                ballB.velocity.z - ballA.velocity.z
            };
            float velAlongNormal = rv.x*normal.x + rv.y*normal.y + rv.z*normal.z;

            //Only resolve if moving toward each other
            if (velAlongNormal < -VEL_EPSILON) {
                CollisionResponse::resolve(ballA, ballB, normal, e);
            }

            //Tiny damping to prevent jitter
            ballA.velocity.x *= 0.995f; ballA.velocity.y *= 0.995f; ballA.velocity.z *= 0.995f;
            ballB.velocity.x *= 0.995f; ballB.velocity.y *= 0.995f; ballB.velocity.z *= 0.995f;
        }

        // draw frame
        renderer.beginFrame();
        renderer.drawGround();
        renderer.drawBall(ballA, ballA.radius, 1.0f, 0.4f, 0.1f);
        renderer.drawBall(ballB, ballB.radius, 0.2f, 0.5f, 1.0f);
        renderer.endFrame();
    }

    return 0;
}