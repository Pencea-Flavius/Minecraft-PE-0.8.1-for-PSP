#include "world/entity/ai/goals/panic_goal.h"
#include "world/entity/path_finder_mob.h"
#include "world/level/pathfinder/path_navigation.h"
#include "util/random_pos.h"
#include <cmath>

PanicGoal::PanicGoal(PathfinderMob* mob, float speed)
:   mob(mob), speedMultiplier(speed), target(0, 0, 0) {
    setRequiredControlFlags(CONTROL_MOVE);
}

bool PanicGoal::canUse() {
    Mob* threat = mob->getLastHurtByMob();
    if (!threat) return false;

    Vec3 away(mob->x - threat->x, 0.0f, mob->z - threat->z);
    float len = sqrtf(away.x * away.x + away.z * away.z);
    if (len < 0.0001f) return RandomPos::generateRandomPos(target, mob, 5, 4, 0);
    away.x /= len; away.z /= len;
    return RandomPos::generateRandomPos(target, mob, 5, 4, &away);
}

bool PanicGoal::canContinueToUse() { return !mob->getNavigation()->isDone(); }

void PanicGoal::start() {
    mob->getNavigation()->moveTo(target.x, target.y, target.z, speedMultiplier);
}
