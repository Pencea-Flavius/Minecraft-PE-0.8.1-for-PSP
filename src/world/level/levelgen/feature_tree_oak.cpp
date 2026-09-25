#include "world/level/levelgen/features.h"
#include "world/level/levelgen/Random.h"
#include "world/level/world.h"

void treeOak(World* w, Random& random, int x, int y, int z) {
    treeBasic(w, random, x, y, z, 4, LEAF_OAK, LOG_OAK);
}
