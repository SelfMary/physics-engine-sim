#ifndef STAGE_H
#define STAGE_H

#include "Render.h"
#include "PhysicsWorld.h"
#include "Types.h"

// Every stage must implement these three functions
void StageSetup();
void StageUpdate(float dt);
void StageRender(Renderer& renderer);

#endif