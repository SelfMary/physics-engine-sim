#ifndef PROJECTILE_H
#define PROJECTILE_H

#include <algorithm>
#include "Types.h"
#include <cmath>

class Projectile
{
    private:
        BodyData body;
    public:
        // Default constructor
        Projectile() {
            body.position     = vec3d{0,0,0};
            body.initial_pos  = vec3d{0,0,0};
            body.velocity     = vec3d{0,0,0};
            body.initial_vel  = vec3d{0,0,0};
            body.mass         = 1.0f;
            body.restitution  = 0.8f;
            body.radius       = 1.0f;
        }
        Projectile(const vec3d& initial_pos, const vec3d& initial_vel, float mass, float restitution, float radius = 1.0f)
        {
            body.position = initial_pos;
            body.initial_pos = initial_pos;
            body.velocity = initial_vel;
            body.initial_vel = initial_vel;
            body.mass = mass;
            body.restitution = restitution;
            body.radius = radius;
        }

        BodyData& data() { return body; } // call like projectile.data() to get the body data for physics calculations and rendering
        const BodyData& data() const { return body; } // const version for read-only access
        const vec3d& ini_pos() const { return body.initial_pos; } // for accessing initial position
        const vec3d& ini_vel() const { return body.initial_vel; } // for accessing initialvelocity
        const vec3d& getPosition() const { return body.position; }
        const vec3d& getVelocity() const { return body.velocity; }

};
#endif