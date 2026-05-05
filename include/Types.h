#ifndef TYPES_H 
//a preprocessor directive in C++ that checks if a specific macro is not defined. If it's not defined, it defines it, else, blank. (prevents multiple declaration)
#define TYPES_H

#include <cmath>
#include <iostream>
// vector to be used
struct vec3d
{
    float x, y, z;
};

struct BodyData
{
    vec3d position;
    vec3d initial_pos; // snapshot of launch position
    vec3d velocity;
    vec3d initial_vel; //snapshot of the launch velocity
    vec3d accel;
    vec3d force;

    float distance; // distance formula to be used
    float radius; // default to 1.0f
    // for collision detection, we are treating all bodies as spheres for simplicity, so we need a radius for the sphere-sphere collision detection.
    // grounds will have small radius for point mass collision, and projectiles will have larger radius for more realistic collision.

    float mass; // amount of matter in a body will be used for density and inertia
    float restitution; // elasticity 0-1
    float theta;          // rotation angle
    float net_tau;
    
    float inertia; // straight line resistance
    float mom_of_inertia; //UwU rotational resistance
    float omega;          // angular velocity
    int   bounces;        // bounce counter

};

class vec3dMath
{
    public:
    static vec3d add(const vec3d& a, const vec3d& b)
    { 
        return {a.x+ b.x, a.y + b.y, a.z + b.z};
    }
    static vec3d subtract(const vec3d& a, const vec3d b)
    { 
        return {a.x - b.x, a.y - b.y, a.z - b.z};
    }
    // multiply by a scalar
    static vec3d scale(const vec3d& v, float s)
    { 
        return {v.x*s,   v.y*s,   v.z*s};
    }

    static vec3d divide(const vec3d& v, float s)
    { 
        return {v.x/s,   v.y/s,   v.z/s};   
    }

    // Now we are doing update
    static void addToSelf(vec3d& a, const vec3d& b)
    { 
        a.x+=b.x; // a.x = a.x + b.x
        a.y+=b.y;
        a.z+=b.z;
    }
    static void subtractToSelf(vec3d& a, const vec3d& b)
    { 
        a.x-=b.x;
        a.y-=b.y;
        a.z-=b.z;
    }

    static void scaleToSelf(vec3d& v, float s)
    { 
        v.x*=s;
        v.y*=s;
        v.z*=s;   
    }

    static void divideToSelf(vec3d& v, float s)
    { 
        v.x/=s;
        v.y/=s;
        v.z/=s;   
    }

    static void zero(vec3d& v)
    {
        v.x = 0.0f;
        v.y = 0.0f;
        v.z = 0.0f;
    }

    static vec3d nagate(vec3d& v)
    {
        return scale(v, -1.0f); // this will generate a new vector that will be negative of previous on.
    }

    static void negateToSelf(vec3d& v)
    {
        v.x = -v.x; // could've used selfscale
        v.y = -v.y;
        v.z = -v.z;
    }

    // distance formula for length // used a lot
    // d = sqrt(x^2 + y^2 + z^2)
    static float length(const vec3d& v) 
    { 
        return sqrtf(v.x*v.x + v.y*v.y + v.z*v.z); 
    }

    // Unit Vector // used alot.
    // Vn = V/|V|
    static vec3d normalized(const vec3d& v) 
    {
        float len = length(v);
        if (len < 1e-6f) return {0,0,0};
        return divide(v, len);
    }

};



#endif