#include "world/entity/ai/goals/random_stroll_goal.h"
#include "world/entity/path_finder_mob.h"
#include "world/level/pathfinder/path_navigation.h"
#include "util/random_pos.h"

RandomStrollGoal::RandomStrollGoal(PathfinderMob* mob, float speed)
:   mob(mob), speedMultiplier(speed), target(0, 0, 0) {
    setRequiredControlFlags(CONTROL_MOVE | CONTROL_LOOK);
}

bool RandomStrollGoal::canUse() {
    if (mob->noActionTime > 99) return false;
    if (Entity::sharedRandom.nextInt(120) != 0) return false;
    return RandomPos::generateRandomPos(target, mob, 10, 7, 0);
}

bool RandomStrollGoal::canContinueToUse() { return !mob->getNavigation()->isDone(); }

void RandomStrollGoal::start() {
    mob->getNavigation()->moveTo(target.x, target.y, target.z, speedMultiplier);
}
