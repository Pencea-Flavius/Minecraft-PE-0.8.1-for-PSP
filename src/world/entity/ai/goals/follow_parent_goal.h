
#ifndef MCPSP_WORLD_ENTITY_AI_GOALS_FOLLOW_PARENT_GOAL_H
#define MCPSP_WORLD_ENTITY_AI_GOALS_FOLLOW_PARENT_GOAL_H

#include "world/entity/ai/goal.h"

class Animal;

class FollowParentGoal : public Goal {
public:
    FollowParentGoal(Animal* animal, float speed);
    virtual bool canUse();
    virtual bool canContinueToUse();
    virtual void start();
    virtual void stop();
    virtual void tick();
private:
    Animal* parent() const;
    Animal* animal;
    float   speedMultiplier;
    int     parentId;
    int     ticksFollowing;
};

#endif
