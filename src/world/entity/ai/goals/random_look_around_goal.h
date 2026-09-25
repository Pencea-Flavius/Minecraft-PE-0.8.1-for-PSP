
#ifndef MCPSP_WORLD_ENTITY_AI_GOALS_RANDOM_LOOK_AROUND_GOAL_H
#define MCPSP_WORLD_ENTITY_AI_GOALS_RANDOM_LOOK_AROUND_GOAL_H

#include "world/entity/ai/goal.h"

class Mob;

class RandomLookAroundGoal : public Goal {
public:
    RandomLookAroundGoal(Mob* mob);
    virtual bool canUse();
    virtual bool canContinueToUse();
    virtual void start();
    virtual void tick();
private:
    Mob*  mob;
    float xOff, zOff;
    int   ticksLeft;
};

#endif
