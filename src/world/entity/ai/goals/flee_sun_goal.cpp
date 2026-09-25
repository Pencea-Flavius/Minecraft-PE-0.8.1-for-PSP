#include "world/entity/ai/goals/flee_sun_goal.h"
#include "world/entity/path_finder_mob.h"
#include "world/level/level.h"
#include "world/level/world.h"
#include "world/level/chunk/chunk.h"
#include "world/level/pathfinder/path_navigation.h"
#include "util/mth.h"
#include <cmath>

FleeSunGoal::FleeSunGoal(PathfinderMob* mob, float speed)
:   mob(mob), speedMultiplier(speed), target(0, 0, 0) {
    setRequiredControlFlags(CONTROL_MOVE);
}

bool FleeSunGoal::getHidePos(Vec3& out) {
    World* w = mob->level->w;
    for (int i = 0; i < 10; i++) {
        int xt = Mth::floor(mob->x + Entity::sharedRandom.nextInt(20) - 10);
        int yt = Mth::floor(mob->bb.y0 + Entity::sharedRandom.nextInt(6) - 3);
        int zt = Mth::floor(mob->z + Entity::sharedRandom.nextInt(20) - 10);
        if (worldCanSeeSky(w, xt, yt, zt)) continue;
        if (yt <= 0) continue;
        if (!isSolidPhys(worldBlock(w, xt, yt - 1, zt))) continue;

        bool fits = true;
        for (int h = 0; h < (int)ceilf(mob->bbHeight); h++)
            if (isSolidPhys(worldBlock(w, xt, yt + h, zt))) { fits = false; break; }
        if (!fits) continue;
        out = Vec3((float)xt, (float)yt, (float)zt);
        return true;
    }
    return false;
}

bool FleeSunGoal::canUse() {
    if (!mob->level->isDay()) return false;
    if (mob->onFire <= 0) return false;
    if (!worldCanSeeSky(mob->level->w, Mth::floor(mob->x), (int)mob->bb.y0,
                        Mth::floor(mob->z))) return false;
    return getHidePos(target);
}

bool FleeSunGoal::canContinueToUse() { return !mob->getNavigation()->isDone(); }

void FleeSunGoal::start() {
    mob->getNavigation()->moveTo(target.x, target.y, target.z, speedMultiplier);
}
