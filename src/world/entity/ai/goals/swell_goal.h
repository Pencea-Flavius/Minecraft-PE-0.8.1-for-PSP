
#ifndef MCPSP_WORLD_ENTITY_AI_GOALS_SWELL_GOAL_H
#define MCPSP_WORLD_ENTITY_AI_GOALS_SWELL_GOAL_H

#include "world/entity/ai/goal.h"

class Creeper;

class SwellGoal : public Goal {
public:
    SwellGoal(Creeper* creeper);
    virtual bool canUse();
    virtual void start();
    virtual void stop();
    virtual void tick();
private:
    Creeper* creeper;
    int targetId;
};

#endif
