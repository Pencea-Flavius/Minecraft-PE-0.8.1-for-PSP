
#ifndef MCPSP_WORLD_ENTITY_AI_GOALS_RANDOM_STROLL_GOAL_H
#define MCPSP_WORLD_ENTITY_AI_GOALS_RANDOM_STROLL_GOAL_H

#include "world/entity/ai/goal.h"
#include "world/level/pathfinder/vec3.h"

class PathfinderMob;

class RandomStrollGoal : public Goal {
public:
    RandomStrollGoal(PathfinderMob* mob, float speed);
    virtual bool canUse();
    virtual bool canContinueToUse();
    virtual void start();
private:
    PathfinderMob* mob;
    float speedMultiplier;
    Vec3  target;
};

#endif
