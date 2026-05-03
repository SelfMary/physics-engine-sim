// Using the differe

#ifndef TRAJECTORY_H
#define TRAJECTORY_H

#include "Projectile.h"
#include "PhysicsWorld.h"
#include "Collision.h"
#include <vector>

class Trajectory
{
    public:
    // compute is function that takes in a projectile, time step, number of steps, and ground level (optional) and returns a vector of vec3d representing the trajectory path of the projectile.
    // std::vector<vec3d> means we are returning a vector of vec3d, which is the position of the projectile at each time step.
    // vector is a dynamic array that can grow in size, and we are reserving space for the number of steps to avoid unnecessary reallocations.
    // returns list of positions (positions over time)
    
    static std::vector<vec3d> compute(Projectile proj /*copy of projectile*/, float dt, int steps, float groundY=0.0f) 
    {
        std::vector<vec3d> path; // create a vector to hold the trajectory path
        path.reserve(steps); // reserve space for the number of steps to avoid unnecessary reallocations, reserve is from vector library
        // We are simulating the projectile's motion over time by integrating its position and velocity at each time step, and checking for collisions with the ground.
        for(int i=0;i<steps;i++)
        {
            // Integrate the projectile's motion using the physics engine, which will update its position and velocity based on the forces acting on it (like gravity).
            Physics::integrate(proj.data(), dt, 0.0f); // no drag for trajectory calculation, we want the ideal path without air resistance
            
            
                BodyData ground;
                ground.position = vec3d{proj.getPosition().x, groundY, proj.getPosition().z};                
                ground.velocity = vec3d{0,0,0};
                ground.mass = INFINITY;
                ground.restitution = proj.data().restitution;
                ground.radius = 0.0f; // radius of the ground is zero for point mass collision

                if(CollisionDetection::OverlapCheck(proj.data(), ground, ground.radius,proj.data().radius)) // radius of projectile and ground are both 0 for point mass
                {
                vec3d normal = {0,1,0}; // normal vector pointing up from the ground
                float penetration = CollisionDetection::penetrationDepth(proj.data(), ground, ground.radius , proj.data().radius);
                // penetration depth is how much the projectile has penetrated the ground, which is the distance from the projectile's position to the ground level (groundY) if it is below the ground.
                
                CollisionResponse::resolve(proj.data(), ground, normal, proj.data().restitution);
                CollisionResponse::positionalCorrection(proj.data(), ground, normal, penetration);
                }
            // Add the current position of the projectile to the trajectory path vector.
            path.push_back(proj.getPosition());
            
            
            // Check for collision with the ground (if the projectile's y position is less than or equal to the ground level and its velocity in the y direction is zero, we consider it a collision with the ground).
            if(proj.getPosition().y<=groundY && proj.getVelocity().y==0) break;
            // vertical position is at or below ground level, and vertical velocity is zero (indicating it has come to rest on the ground)
                
    }
    return path;
    }
    

};
#endif