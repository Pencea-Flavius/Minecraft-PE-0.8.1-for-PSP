
#ifndef MCPSP_WORLD_ENTITY_AI_GOALS_NEAREST_ATTACKABLE_TARGET_GOAL_H
#define MCPSP_WORLD_ENTITY_AI_GOALS_NEAREST_ATTACKABLE_TARGET_GOAL_H

#include "world/entity/ai/goals/target_goal.h"

class PathfinderMob;

class NearestAttackableTargetGoal : public TargetGoal {
public:
    NearestAttackableTargetGoal(PathfinderMob* mob, float radius);
    virtual bool canUse();
    virtual void start();
private:
    PathfinderMob* pfMob;
    int candidateId;
};

#endif
