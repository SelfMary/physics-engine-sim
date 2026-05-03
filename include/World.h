#ifndef WORLD_H
#define WORLD_H

#include "Render.h"

// Enum for different world types
enum class WorldType {
    Basic,
    ProjectileView
};

// Abstract base class
class World {
protected:
    WorldType type;

public:
    World(WorldType t) : type(t) {}
    virtual ~World() = default;

    WorldType getType() const { return type; }

    // Must be implemented by derived worlds
    virtual void update(float dt) = 0;
    virtual void render() = 0;

    virtual void setup() = 0;

    // already present
    virtual void update(float dt) = 0;
    virtual void render() = 0;
};

#endif // WORLD_H