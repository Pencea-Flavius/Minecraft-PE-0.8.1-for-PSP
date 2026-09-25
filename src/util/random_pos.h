
#ifndef MCPSP_UTIL_RANDOM_POS_H
#define MCPSP_UTIL_RANDOM_POS_H

#include "world/level/pathfinder/vec3.h"

class PathfinderMob;

namespace RandomPos {

    bool generateRandomPos(Vec3& res, PathfinderMob* mob, int xzRange, int yRange,
                           const Vec3* bias);
}

#endif
