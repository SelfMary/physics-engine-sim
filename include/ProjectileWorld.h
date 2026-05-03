#ifndef PROJECTILEWORLD_H
#define PROJECTILEWORLD_H

#include "World.h"
#include "Render.h"
#include "ProjectileStage.h"

class ProjectileWorld : public World {
private:
    Renderer& renderer;
    ProjectileStage* currentStage;

public:
    ProjectileWorld(Renderer& r) // constructor to initialize the renderer reference and set currentStage to nullptr
        : World(WorldType::ProjectileView), renderer(r), currentStage(nullptr) {}

    ~ProjectileWorld() { delete currentStage; } // destructor to clean up the current stage

    void setup() 
    {
        setStage(STAGE1); // default stage
    }

    void update(float dt)  // update function to run the current stage's physics update
    {
        if (currentStage) currentStage->run(dt);
    }

    void render() {
        renderer.beginFrame();
        renderer.drawGround();
        if (currentStage) currentStage->render(renderer);
        renderer.endFrame();
    }

    void setStage(StageType type) {
        delete currentStage;
        switch (type) {
            case STAGE1: currentStage = new Stage1(); break;
            case STAGE2: currentStage = new Stage2(); break;
            case STAGE3: currentStage = new Stage3(); break;
            case STAGE4: currentStage = new Stage4(); break;
            case STAGE5: currentStage = new Stage5(); break;
        }
        currentStage->setup();
    }
};

#endif // PROJECTILEWORLD_H
