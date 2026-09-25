
#ifndef MCPSP_WORLD_ENTITY_AI_GOALS_TARGET_GOAL_H
#define MCPSP_WORLD_ENTITY_AI_GOALS_TARGET_GOAL_H

#include "world/entity/ai/goal.h"

class Mob;
class Entity;

class TargetGoal : public Goal {
public:
    TargetGoal(Mob* mob, float radius, bool needsSight);
    virtual bool canContinueToUse();
    virtual void stop();

    static const int UNSEEN_MEMORY = 60;

    float keepRadius() const { return radius * 2.0f; }
protected:

    bool canAttack(Entity* candidate, bool ignoreSight);

    Mob*  mob;
    float radius;
    bool  needsSight;
    int   unseenTicks;
};

#endif
