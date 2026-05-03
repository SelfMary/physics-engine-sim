#ifndef PROJECTILESTAGE_H
#define PROJECTILESTAGE_H

#include "Render.h"
#include "PhysicsWorld.h"
#include "Projectile.h"

enum StageType { STAGE1, STAGE2, STAGE3, STAGE4, STAGE5 };

class ProjectileStage {
public:
    virtual ~ProjectileStage() {}
    virtual void setup() = 0;
    virtual void run(float dt) = 0;
    virtual void render(Renderer& renderer) = 0;
};

// Stage1: live integration
class Stage1 : public ProjectileStage {
private:
    Projectile projectile;
    std::vector<vec3d> trail;
public:
    Stage1();
    void setup() override;
    void run(float dt) override;
    void render(Renderer& renderer) override;
};

// Stage2: live integration with different initial conditions
class Stage2 : public ProjectileStage {
private:
    Projectile projectile;
    std::vector<vec3d> trail;
public:
    Stage2();
    void setup() override;
    void run(float dt) override;
    void render(Renderer& renderer) override;
};

// Stage3: precomputed trajectory
class Stage3 : public ProjectileStage {
private:
    Projectile projectile;
    std::vector<vec3d> trail;
public:
    Stage3();
    void setup() override;
    void run(float dt) override;
    void render(Renderer& renderer) override;
};

// Stage4: multiple projectiles
class Stage4 : public ProjectileStage {
private:
    std::vector<Projectile> projectiles;
    std::vector<std::vector<vec3d>> trails;
public:
    Stage4();
    void setup() override;
    void run(float dt) override;
    void render(Renderer& renderer) override;
};

// Stage5: projectile vs target
class Stage5 : public ProjectileStage {
private:
    Projectile projectile;
    BodyData target;
    std::vector<vec3d> trail;
    bool hit;
public:
    Stage5();
    void setup() override;
    void run(float dt) override;
    void render(Renderer& renderer) override;
};

#endif // PROJECTILESTAGE_H
