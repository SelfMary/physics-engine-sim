// drag force stuff to be added

#ifndef PHYSICSWORLD_H
#define PHYSICSWORLD_H
#include "Types.h"


class Physics
// Physics::function
{
    private:
    BodyData body;
    public: 
    static void integrate(BodyData& b, float dt, float drag) 
    {
        vec3d force = b.force; // get the current force on the body
        
        {
        // Apply linear drag
        b.force.x -= drag * b.velocity.x;
        b.force.y -= drag * b.velocity.y;
        b.force.z -= drag * b.velocity.z;
        };
        // F =  m * a
        vec3d accel =
        {
            b.force.x / b.mass,
            b.force.y / b.mass,
            b.force.z / b.mass
        };

        // x'' = a
        // x' = v
        // v = a * sum(dt) (integration)

        b.velocity.x += accel.x * dt;
        b.velocity.y += accel.y * dt;
        b.velocity.z += accel.z * dt;

        // drag force air resistance Fdrag = -k * v (simplified Linear Drag)
        // where k is the drag coefficient, and v is the velocity of the object.
        // adrag = Fdrag / m = -k/m * v
        // v' = a - k/m * v
        // vnew = adt + vold = -k/m * vold * dt + vold
        // vnew = vold * (1 - k/m * dt)

        // x = v * sum(dt) (integration)

        b.position.x += b.velocity.x * dt;
        b.position.y += b.velocity.y * dt;
        b.position.z += b.velocity.z * dt;

        // finding distance from launch d(t) = | current_position − launch_point |
        b.distance = vec3dMath::length(vec3dMath::subtract(b.position, b.initial_pos));

        b.force = {0,0,0}; // reset force-to be changed
    }

    // Linear drag: Fd = -k v
    static void applyLinearDrag(BodyData& b, float k, float dt) 
    {
        float dragCoefficient = k / b.mass;
        b.velocity.x -= dragCoefficient * b.velocity.x * dt;
        b.velocity.y -= dragCoefficient * b.velocity.y * dt;
        b.velocity.z -= dragCoefficient * b.velocity.z * dt;
    }

    // Quadratic drag: Fd = -k |v| v
    static void applyQuadraticDrag(BodyData& b, float k, float dt) 
    {
        float dragCoefficient = k / b.mass;
        b.velocity.x -= dragCoefficient * b.velocity.x * std::abs(b.velocity.x) * dt;
        b.velocity.y -= dragCoefficient * b.velocity.y * std::abs(b.velocity.y) * dt;
        b.velocity.z -= dragCoefficient * b.velocity.z * std::abs(b.velocity.z) * dt;
    }
};


#endif