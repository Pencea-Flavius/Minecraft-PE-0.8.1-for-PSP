
#ifndef MCPSP_WORLD_ENTITY_AI_GOALS_HURT_BY_TARGET_GOAL_H
#define MCPSP_WORLD_ENTITY_AI_GOALS_HURT_BY_TARGET_GOAL_H

#include "world/entity/ai/goals/target_goal.h"

class HurtByTargetGoal : public TargetGoal {
public:
    HurtByTargetGoal(Mob* mob, float radius);
    virtual bool canUse();
    virtual void start();
};

#endif
