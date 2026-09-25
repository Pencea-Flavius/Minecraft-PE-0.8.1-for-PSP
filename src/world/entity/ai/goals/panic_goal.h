
#ifndef MCPSP_WORLD_ENTITY_AI_GOALS_PANIC_GOAL_H
#define MCPSP_WORLD_ENTITY_AI_GOALS_PANIC_GOAL_H

#include "world/entity/ai/goal.h"
#include "world/level/pathfinder/vec3.h"

class PathfinderMob;

class PanicGoal : public Goal {
public:
    PanicGoal(PathfinderMob* mob, float speed);
    virtual bool canUse();
    virtual bool canContinueToUse();
    virtual void start();
private:
    PathfinderMob* mob;
    float speedMultiplier;
    Vec3  target;
};

#endif
