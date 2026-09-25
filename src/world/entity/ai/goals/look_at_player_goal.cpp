#include "world/entity/ai/goals/look_at_player_goal.h"
#include "world/entity/ai/look_control.h"
#include "world/entity/mob.h"
#include "world/entity/local_player.h"
#include "world/level/level.h"

LookAtPlayerGoal::LookAtPlayerGoal(Mob* mob, float maxDistance, float startChance)
:   mob(mob), maxDistance(maxDistance), startChance(startChance),
    targetId(0), ticksLeft(0) {
    setRequiredControlFlags(CONTROL_LOOK);
}

Entity* LookAtPlayerGoal::target() const {
    return targetId ? mob->level->getEntity(targetId) : 0;
}

bool LookAtPlayerGoal::canUse() {
    if (Entity::sharedRandom.nextFloat() >= startChance) return false;
    LocalPlayer* p = mob->level->getNearestPlayer(mob, maxDistance);
    targetId = p ? p->entityId : 0;
    return targetId != 0;
}

bool LookAtPlayerGoal::canContinueToUse() {
    Entity* t = target();
    if (!t || !t->isAlive()) return false;
    if (mob->distanceToSqr(t) > maxDistance * maxDistance) return false;
    return ticksLeft > 0;
}

void LookAtPlayerGoal::start() { ticksLeft = 40 + Entity::sharedRandom.nextInt(40); }
void LookAtPlayerGoal::stop()  { targetId = 0; }

void LookAtPlayerGoal::tick() {
    Entity* t = target();
    if (!t) return;
    float headY = t->y + (t->isMob() ? ((Mob*)t)->getHeadHeight() : 0.0f);
    mob->getLookControl()->setLookAt(t->x, headY, t->z, 10.0f, mob->getMaxHeadXRot());
    ticksLeft--;
}
