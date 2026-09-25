
#ifndef MCPSP_WORLD_ENTITY_AI_GOALS_BREED_GOAL_H
#define MCPSP_WORLD_ENTITY_AI_GOALS_BREED_GOAL_H

#include "world/entity/ai/goal.h"

class Animal;

class BreedGoal : public Goal {
public:
    BreedGoal(Animal* animal, float speed);
    virtual bool canUse();
    virtual bool canContinueToUse();
    virtual void stop();
    virtual void tick();
private:
    Animal* partner() const;
    Animal* findFreePartner();
    void    breed();

    Animal* animal;
    float   speedMultiplier;
    int     partnerId;
    int     inUseTicks;
    int     repathDelay;
};

#endif
