
#ifndef MCPSP_WORLD_ENTITY_AI_GOALS_RESTRICT_SUN_GOAL_H
#define MCPSP_WORLD_ENTITY_AI_GOALS_RESTRICT_SUN_GOAL_H

#include "world/entity/ai/goal.h"

class PathfinderMob;

class RestrictSunGoal : public Goal {
public:
    RestrictSunGoal(PathfinderMob* mob) : mob(mob) {}
    virtual bool canUse();
    virtual void start();
    virtual void stop();
private:
    PathfinderMob* mob;
};

#endif
