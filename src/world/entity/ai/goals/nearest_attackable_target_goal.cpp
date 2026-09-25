#include "world/entity/ai/goals/nearest_attackable_target_goal.h"
#include "world/entity/path_finder_mob.h"
#include "world/level/level.h"

NearestAttackableTargetGoal::NearestAttackableTargetGoal(PathfinderMob* mob, float radius)
:   TargetGoal(mob, radius, true), pfMob(mob), candidateId(0) {
    setRequiredControlFlags(CONTROL_MOVE);
}

bool NearestAttackableTargetGoal::canUse() {
    Entity* candidate = pfMob->findAttackTarget();
    if (!candidate) return false;
    if (pfMob->distanceToSqr(candidate) > radius * radius) return false;

    if (!canAttack(candidate, true)) return false;
    candidateId = candidate->entityId;
    return true;
}

void NearestAttackableTargetGoal::start() {
    mob->setTarget(mob->level->getEntity(candidateId));
}
