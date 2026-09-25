
#ifndef MCPSP_WORLD_ENTITY_AI_GOALS_TEMPT_GOAL_H
#define MCPSP_WORLD_ENTITY_AI_GOALS_TEMPT_GOAL_H

#include "world/entity/ai/goal.h"

class PathfinderMob;
class Entity;

class TemptGoal : public Goal {
public:

    TemptGoal(PathfinderMob* mob, float speed, const int* items, int count);

    virtual bool canUse();
    virtual bool canContinueToUse();
    virtual void start();
    virtual void stop();
    virtual void tick();
    bool isRunning() const { return running; }

    static const int OFF_GRACE = 10;

private:
    Entity* player() const;

    PathfinderMob* mob;
    float speedMultiplier;
    const int* items;
    int   itemCount;
    int   playerId;
    int   timeout;
    int   offTicks;
    int   repathDelay;
    bool  running, prevAvoidWater;
};

#endif
