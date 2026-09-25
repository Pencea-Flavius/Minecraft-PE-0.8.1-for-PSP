
#ifndef MCPSP_WORLD_ENTITY_AI_GOALS_ARROW_ATTACK_GOAL_H
#define MCPSP_WORLD_ENTITY_AI_GOALS_ARROW_ATTACK_GOAL_H

#include "world/entity/ai/goal.h"

class Mob;
class Entity;

class ArrowAttackGoal : public Goal {
public:

    enum { ATTACK_ARROW = 1, ATTACK_SNOWBALL = 2 };

    ArrowAttackGoal(Mob* mob, float speed, int attackType, int delayBetweenAttacks);
    virtual bool canUse();
    virtual bool canContinueToUse();
    virtual void stop();
    virtual void tick();
private:
    void fireAtTarget(Entity* target);

    Mob*  mob;
    float speedMultiplier;
    int   attackType;
    int   delayBetweenAttacks, attackTimeout, seenTicks;
};

#endif
