#include "world/entity/ai/goals/random_look_around_goal.h"
#include "world/entity/ai/look_control.h"
#include "world/entity/mob.h"
#include "util/mth.h"
#include <cmath>

RandomLookAroundGoal::RandomLookAroundGoal(Mob* mob)
:   mob(mob), xOff(0), zOff(0), ticksLeft(0) {
    setRequiredControlFlags(CONTROL_MOVE | CONTROL_LOOK);
}

bool RandomLookAroundGoal::canUse() { return Entity::sharedRandom.nextFloat() < 0.02f; }
bool RandomLookAroundGoal::canContinueToUse() { return ticksLeft >= 0; }

void RandomLookAroundGoal::start() {
    float t = Entity::sharedRandom.nextFloat() * 2.0f * Mth::PI;
    xOff = cosf(t);
    zOff = sinf(t);
    ticksLeft = 20 + Entity::sharedRandom.nextInt(20);
}

void RandomLookAroundGoal::tick() {
    ticksLeft--;
    mob->getLookControl()->setLookAt(mob->x + xOff, mob->y + mob->getHeadHeight(),
                                     mob->z + zOff, 10.0f, mob->getMaxHeadXRot());
}
