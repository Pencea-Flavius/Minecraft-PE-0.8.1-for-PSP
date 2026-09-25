
#ifndef MCPSP_WORLD_ENTITY_AI_GOALS_FLEE_SUN_GOAL_H
#define MCPSP_WORLD_ENTITY_AI_GOALS_FLEE_SUN_GOAL_H

#include "world/entity/ai/goal.h"
#include "world/level/pathfinder/vec3.h"

class PathfinderMob;

class FleeSunGoal : public Goal {
public:
    FleeSunGoal(PathfinderMob* mob, float speed);
    virtual bool canUse();
    virtual bool canContinueToUse();
    virtual void start();
private:
    bool getHidePos(Vec3& out);
    PathfinderMob* mob;
    float speedMultiplier;
    Vec3  target;
};

#endif
