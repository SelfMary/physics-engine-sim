# ifndef BASICWORLD_H
# define BASICWORLD_H

#include "World.h"
#include "Render.h"
#include "PhysicsWorld.h"
#include "Collision.h"

class BasicWorld : public World // inherits from World
{
    private:
    Renderer& renderer; // reference to the renderer
    std::vector<BodyData> bodies; // vector to store the bodies in the world
    public: 
    BasicWorld(Renderer& r) : World(WorldType::Basic),  renderer(r) {}// constructor to initialize the renderer reference
    void setup() // setup function to initialize the world
    {
        // Initialize the world with some bodies (for example, a ball and the ground)
        BodyData ballA;
        ballA.position = {0.0f, 10.0f, 0.0f}; // starting position of the ball
        ballA.velocity = {5.0f, 0.0f, 0.0f}; // initial velocity of the ball
        ballA.mass = 1.0f; // mass of the ball
        ballA.force = {0.0f, -9.81f, 0.0f}; // gravity force acting on the ball
        ballA.initial_pos = ballA.position; // store initial position for distance calculation
        bodies.push_back(ballA); // add the ball to the world
        BodyData ballB;
        ballB.position = {20.0f, 15.0f, 0.0f}; // starting position of the second ball
        ballB.velocity = {-5.0f, 0.0f, 0.0f}; // initial velocity of the second ball
        ballB.mass = 1.0f; // mass of the second ball
        ballB.force = {0.0f, -9.81f, 0.0f}; // gravity force acting on the second ball
        ballB.initial_pos = ballB.position; // store initial position for distance calculation  
        bodies.push_back(ballB); // add the second ball to the world

        BodyData ground;
        ground.position = {0.0f, 0.0f, 0.0f}; // position of the ground
        ground.velocity = {0.0f, 0.0f, 0.0f}; // ground is static
        ground.mass = std::numeric_limits<float>::infinity(); // infinite mass for immovable object
        ground.force = {0.0f, 0.0f, 0.0f}; // no force acting on the ground
        bodies.push_back(ground); // add the ground to the world
    };

    void update(float dt)   
    {
        // Update physics for each body
        for (auto& body : bodies)
        {
        if (body.mass < std::numeric_limits<float>::infinity())  // only apply gravity to non-infinite mass bodies (e.g., the balls, not the ground)
        {
        body.force.y -= 9.81f * body.mass; // gravity
        }
            Physics::integrate(body, dt, 0.99f); // integrate the physics for each body
        }

        if (bodies.size() > 1) // check if there are at least two bodies to check for collisions
        {
            auto& a = bodies[0]; // first body (e.g., ballA)
            auto& b = bodies[1]; // second body (e.g., ballB)
            if (CollisionDetection::OverlapCheck(a, b, a.radius, b.radius)) // check for overlap between the two bodies
            {
                vec3d normal = CollisionDetection::contactNormal(a, b); // get the collision normal
                float restitution = 0.8f; // elasticity of the collision
                float penetration = CollisionDetection::penetrationDepth(a, b, a.radius, b.radius); // calculate penetration depth
                CollisionResponse::resolve(a, b, normal, restitution); // resolve the collision between the two bodies
                CollisionResponse::positionalCorrection(a,b, normal, penetration); // apply positional correction to prevent sinking
                
            }
        }
    };

    void render() 
    {
        renderer.beginFrame();
        renderer.drawGround();
        for (auto& b : bodies) renderer.drawBall(b, b.radius);
        renderer.endFrame();
    }
    
};
#endif // BASICWORLD_H