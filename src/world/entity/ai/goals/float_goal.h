
#ifndef MCPSP_WORLD_ENTITY_AI_GOALS_FLOAT_GOAL_H
#define MCPSP_WORLD_ENTITY_AI_GOALS_FLOAT_GOAL_H

#include "world/entity/ai/goal.h"

class Mob;

class FloatGoal : public Goal {
public:
    FloatGoal(Mob* mob);
    virtual bool canUse();
    virtual void tick();
private:
    Mob* mob;
};

#endif
