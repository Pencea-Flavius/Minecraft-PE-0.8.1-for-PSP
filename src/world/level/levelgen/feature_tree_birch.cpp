#include "world/level/levelgen/features.h"
#include "world/level/levelgen/Random.h"
#include "world/level/world.h"

void treeBirch(World* w, Random& random, int x, int y, int z) {
    treeBasic(w, random, x, y, z, 5, LEAF_BIRCH, LOG_BIRCH);
}
