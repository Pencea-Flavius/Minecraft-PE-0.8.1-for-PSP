#include "world/entity/ai/goals/target_goal.h"
#include "world/entity/ai/sensing.h"
#include "world/entity/mob.h"
#include "world/entity/player.h"
#include "world/inventory/inventory.h"

TargetGoal::TargetGoal(Mob* mob, float radius, bool needsSight)
:   mob(mob), radius(radius), needsSight(needsSight), unseenTicks(0) {}

bool TargetGoal::canAttack(Entity* candidate, bool ignoreSight) {
    if (!candidate || candidate == mob || !candidate->isAlive()) return false;

    if (candidate->bb.y1 <= mob->bb.y0 || candidate->bb.y0 >= mob->bb.y1) return false;

    if (candidate->isPlayer() && !ignoreSight &&
        ((Player*)candidate)->inventory->isCreative()) return false;
    if (!needsSight) return true;
    return mob->getSensing()->canSee(candidate);
}

bool TargetGoal::canContinueToUse() {
    Entity* t = mob->getTarget();
    if (!t || !t->isAlive()) return false;
    if (mob->distanceToSqr(t) > keepRadius() * keepRadius()) return false;
    if (!needsSight) return true;

    if (mob->getSensing()->canSee(t)) { unseenTicks = 0; return true; }
    return ++unseenTicks <= UNSEEN_MEMORY;
}

void TargetGoal::stop() { mob->setTarget(0); unseenTicks = 0; }
