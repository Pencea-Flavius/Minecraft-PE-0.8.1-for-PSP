#include "world/entity/ai/goals/hurt_by_target_goal.h"
#include "world/entity/mob.h"

HurtByTargetGoal::HurtByTargetGoal(Mob* mob, float radius)
:   TargetGoal(mob, radius, false) {
    setRequiredControlFlags(CONTROL_MOVE);
}

bool HurtByTargetGoal::canUse() { return canAttack(mob->getLastHurtByMob(), true); }
void HurtByTargetGoal::start()  { mob->setTarget(mob->getLastHurtByMob()); }
