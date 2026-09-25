
#ifndef MCPSP_WORLD_ENTITY_AI_GOALS_MELEE_ATTACK_GOAL_H
#define MCPSP_WORLD_ENTITY_AI_GOALS_MELEE_ATTACK_GOAL_H

#include "world/entity/ai/goal.h"

class Mob;
class Entity;

class MeleeAttackGoal : public Goal {
public:

    MeleeAttackGoal(Mob* attacker, float speed, bool persistent);
    virtual bool canUse();
    virtual bool canContinueToUse();
    virtual void start();
    virtual void stop();
    virtual void tick();
private:
    Mob*  attacker;
    float speedMultiplier;
    bool  persistent;
    int   attackCooldown;
    int   repathDelay;
};

#endif
