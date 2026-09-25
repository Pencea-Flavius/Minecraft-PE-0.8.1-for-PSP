#include "world/entity/ai/goals/float_goal.h"
#include "world/entity/ai/jump_control.h"
#include "world/entity/mob.h"
#include "world/level/pathfinder/path_navigation.h"

FloatGoal::FloatGoal(Mob* mob) : mob(mob) {
    setRequiredControlFlags(CONTROL_JUMP);
    mob->getNavigation()->setCanFloat(true);
}

bool FloatGoal::canUse() { return mob->isInWater() || mob->isInLava(); }

void FloatGoal::tick() {
    if (Entity::sharedRandom.nextFloat() < 0.8f) mob->getJumpControl()->jump();
}
