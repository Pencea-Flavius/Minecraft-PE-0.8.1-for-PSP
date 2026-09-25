
#include "util/random_pos.h"
#include "util/mth.h"
#include "world/entity/path_finder_mob.h"

namespace RandomPos {

bool generateRandomPos(Vec3& res, PathfinderMob* mob, int xzRange, int yRange,
                       const Vec3* bias) {
    Random& random = Entity::sharedRandom;
    const int xzSpan = 2 * xzRange;
    const int ySpan  = 2 * yRange;

    bool  found = false;
    float best  = -99999.0f;
    int   bx = 0, by = 0, bz = 0;

    for (int i = 0; i < 10; i++) {

        int dx = (int)(random.nextInt(xzSpan)) - xzRange;
        int dy = (int)(random.nextInt(ySpan))  - yRange;
        int dz = (int)(random.nextInt(xzSpan)) - xzRange;
        if (bias && (dz * bias->z + dx * bias->x) < 0.0f) continue;

        int xt = Mth::floor(mob->x) + dx;
        int yt = Mth::floor(mob->y) + dy;
        int zt = Mth::floor(mob->z) + dz;
        float value = mob->getWalkTargetValue(xt, yt, zt);
        if (value > best) { best = value; bx = xt; by = yt; bz = zt; found = true; }
    }

    if (!found) return false;
    res.x = (float)bx; res.y = (float)by; res.z = (float)bz;
    return true;
}

}
