
#ifndef MCPSP_WORLD_ENTITY_AI_GOALS_LOOK_AT_PLAYER_GOAL_H
#define MCPSP_WORLD_ENTITY_AI_GOALS_LOOK_AT_PLAYER_GOAL_H

#include "world/entity/ai/goal.h"

class Mob;
class Entity;

class LookAtPlayerGoal : public Goal {
public:
    LookAtPlayerGoal(Mob* mob, float maxDistance, float startChance = 0.02f);
    virtual bool canUse();
    virtual bool canContinueToUse();
    virtual void start();
    virtual void stop();
    virtual void tick();
private:
    Entity* target() const;
    Mob*  mob;
    float maxDistance, startChance;
    int   targetId, ticksLeft;
};

#endif
