
#ifndef MCPSP_WORLD_ENTITY_AI_GOALS_EAT_TILE_GOAL_H
#define MCPSP_WORLD_ENTITY_AI_GOALS_EAT_TILE_GOAL_H

#include "world/entity/ai/goal.h"

class Mob;

class EatTileGoal : public Goal {
public:
    EatTileGoal(Mob* mob);
    virtual bool canUse();
    virtual bool canContinueToUse();
    virtual void start();
    virtual void stop();
    virtual void tick();

    int getEatAnimationTick() const { return ticksLeft; }

    static const int EAT_TICKS = 40;
private:
    Mob* mob;
    int  ticksLeft;
};

#endif
