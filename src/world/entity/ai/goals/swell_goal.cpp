#include "world/entity/ai/goals/swell_goal.h"
#include "world/entity/ai/sensing.h"
#include "world/entity/monster/creeper.h"
#include "world/level/level.h"
#include "world/level/pathfinder/path_navigation.h"

SwellGoal::SwellGoal(Creeper* creeper) : creeper(creeper), targetId(0) {
    setRequiredControlFlags(CONTROL_MOVE);
}

bool SwellGoal::canUse() {
    if (creeper->getSwellDir() > 0) return true;
    Entity* t = creeper->getTarget();
    return t && creeper->distanceToSqr(t) < 9.0f;
}

void SwellGoal::start() {
    creeper->getNavigation()->stop();
    Entity* t = creeper->getTarget();
    targetId = t ? t->entityId : 0;
}

void SwellGoal::stop() { targetId = 0; }

void SwellGoal::tick() {
    Entity* t = targetId ? creeper->level->getEntity(targetId) : 0;

    bool hold = t && t->isAlive() && creeper->distanceToSqr(t) <= 49.0f &&
                creeper->getSensing()->canSee(t);
    if (creeper->getSwellDir() != 2) creeper->setSwellDir(hold ? 1 : -1);
}
