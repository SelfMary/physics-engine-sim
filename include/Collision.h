#ifndef COLLISION_H
# define COLLISION_H

# include "Types.h"
# include "PhysicsWorld.h"

// Checks if the two bodies are overlapping -> colliding AABB
class CollisionDetection
{
    public:
    // Sphere-sphere overlap
    // Sum of radius of two spheres must be greater than the distance between their centers if they are not colliding
    static bool OverlapCheck(const BodyData& a, const BodyData& b, float radiusA, float radiusB)
    {
        vec3d diff = vec3dMath::subtract(b.position, a.position);
        float distance_between = vec3dMath::length(diff);
        return distance_between < (radiusA + radiusB);
    };

    // Getting the collision normal (the direction of the collision)
    static vec3d contactNormal(const BodyData& a, const BodyData& b)
    {
        vec3d diff = vec3dMath::subtract(b.position, a.position);
        return vec3dMath::normalized(diff);
    };

    // penetration checked when overlap exists, how much the two spheres are penetrating each other.
    static float penetrationDepth(const BodyData& a, const BodyData& b, float radiusA, float radiusB)
    {
        vec3d diff = vec3dMath::subtract(b.position, a.position);
        float distance_between = vec3dMath::length(diff);
        return (radiusA + radiusB) - distance_between;
    };

};

class CollisionResponse
{
    public:
    // Impulse-based resolution
    // normal should be given the unit vector of the collision normal, and restitution is the elasticity of the collision (0 for inelastic, 1 for perfectly elastic)
    static void resolve(BodyData& a, BodyData& b, const vec3d& normal, float restitution)
    {
        vec3d rv = vec3dMath::subtract(b.velocity, a.velocity); // relative stuff
        float velAlongNormal = rv.x*normal.x + rv.y*normal.y + rv.z*normal.z; // negative if they are moving towards each other.

        // If they are separating, do nothing
        if (velAlongNormal > 0) return;

        // larger mass, smaller inverse mass, less velocity change, barely moves, vice versa for smaller mass
        float invMassA = 1.0f / a.mass;
        float invMassB = 1.0f / b.mass;
/*
        // j = integral( F * dt ) -> time less
        // j -> momentum , chang in P = change in vel * mass 
        // When two bodies collide, we care about their relative velocity along the contact normal: Vrel times normal. 
        // If this value is positive, it means the bodies are moving apart and we don't need to resolve the collision.
        // If it's negative, we need to apply an impulse to separate them.
        // relative velocity after collision = - restitution * relative velocity before collision [Vrel' = -e * Vrel]
        // total relative velocity change = relative velocity after collision - relative velocity before collision [t_Vrel = Vrel' - Vrel = - e * Vrel - Vrel]
        // t_Vrel = -(1 + restitution) * Vrel
        // As we have two bodies, we have to divide impulse.
        // VrelA = - j * invMassA, VrelB = j * invMassB, so Vrel = VrelB - VrelA = j * (invMassA + invMassB) * normal
        // Vrel = impulse / mass, so Change in Vrel = impulse * (InvmassA + InvmassB)
        // -(1 + restitution) * Vrel = j * (InvmassA + InvmassB)
        // j = -(1 + restitution) * velAlongNormal / (invMassA + invMassB)
*/
        float j = -(1 + restitution) * velAlongNormal / (invMassA + invMassB); // scaler
        vec3d impulse = vec3dMath::scale(normal, j); // getting impulse vector

        a.velocity = vec3dMath::subtract(a.velocity, vec3dMath::scale(impulse, invMassA)); // applying impulse to body A, opposite direction of normal
        b.velocity = vec3dMath::add(b.velocity, vec3dMath::scale(impulse, invMassB)); // applying impulse to body B, same direction as normal
    
    }
/*
    // Positional correction, we are putting threashold where correction will happen.
    // Jittering: Smaill oscillation when the object is still, like a ball on ground can start "vibrating" due to the collision response trying to resolve the collision every frame, even when the objects are resting on each other. 
    // Reasons: Numerical errors in floating‑point math, Repeated impulses being applied every frame even when bodies should be at rest, Penetration depth being slightly positive due to integration.
    // This can be fixed by applying a positional correction to separate the objects slightly after a collision is resolved, preventing them from being detected as colliding in the next frame. Giving them an escape from collision detector.
    // We need Positional Correction.
    // Penetration depth is calculated only when overlap happens, penetarion = (radiusA + radiusB) - distance_between.
    // Correction vector is applied along the contact normal.
    Its functions:
    - Pushes bodies apart just enough to remove overlap.
    - The slop tolerance prevents tiny penetrations from being corrected (reduces jitter).
    - The percent factor avoids over‑correction, keeping simulation stable.
*/
    static void positionalCorrection(BodyData& a, BodyData& b, const vec3d& normal, float penetration) 
    {
        const float percent = 0.1f; // usually 20% to 80% of the penetration is corrected to avoid jittering
        const float slop = 0.01f; // penetration allowance, usually a small value to prevent jittering when objects are very close (so they don't go through each other when collision happens)
        float invMassA = 1.0f / a.mass; // larger mass, smaller inverse mass, less position change, barely moves, vice versa for smaller mass
        float invMassB = 1.0f / b.mass;

        if (penetration <= slop) return;

        float correctionMag = (penetration - slop) / (invMassA + invMassB) * percent; // only correct if penetration is greater than slop, and distribute correction based on inverse mass.
        vec3d correction = vec3dMath::scale(normal, correctionMag); // correction vector.

        a.position = vec3dMath::subtract(a.position, vec3dMath::scale(correction, invMassA)); // applying correction to body A, opposite direction of normal
        b.position = vec3dMath::add(b.position, vec3dMath::scale(correction, invMassB)); // applying correction to body B, same direction as normal
    }

};


// Application: Collision Detection -> check overlap -> get penetration if these is overlap -> get contact normal -> give the normal to collision response -> resolve collision with impulse -> apply correction.

#endif