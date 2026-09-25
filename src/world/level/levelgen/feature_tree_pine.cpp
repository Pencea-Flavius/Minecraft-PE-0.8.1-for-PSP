#include "world/level/levelgen/features.h"
#include "world/level/levelgen/Random.h"
#include "world/level/world.h"

static int coneRadiusAt(int layer, int, int arg) {
    return (layer < (arg >> 8)) ? 0 : (arg & 0xFF);
}

void treePine(World* w, Random& random, int x, int y, int z) {
    int treeHeight = random.nextInt(5) + 7;
    int trunkHeight = treeHeight - random.nextInt(2) - 3;
    int topHeight = treeHeight - trunkHeight;
    int topRadius = 1 + random.nextInt(topHeight + 1);

    if (!treeSpaceClear(w, x, y, z, treeHeight, coneRadiusAt, (trunkHeight << 8) | topRadius)) return;
    setBlock(w, x, y - 1, z, BLOCK_DIRT);
    int currentRadius = 0;
    for (int yy = y + treeHeight; yy >= y + trunkHeight; yy--) {
        for (int xx = x - currentRadius; xx <= x + currentRadius; xx++) {
            int axo = (xx - x < 0) ? -(xx - x) : (xx - x);
            for (int zz = z - currentRadius; zz <= z + currentRadius; zz++) {
                int azo = (zz - z < 0) ? -(zz - z) : (zz - z);
                if (axo == currentRadius && azo == currentRadius && currentRadius > 0) continue;
                if (!isSolidGen(worldBlock(w, xx, yy, zz))) setBlock(w, xx, yy, zz, BLOCK_LEAVES, LEAF_SPRUCE);
            }
        }
        if (currentRadius >= 1 && yy == y + trunkHeight + 1) currentRadius--;
        else if (currentRadius < topRadius) currentRadius++;
    }
    for (int hh = 0; hh < treeHeight - 1; hh++) {
        unsigned char t = worldBlock(w, x, y + hh, z);
        if (!isSolidGen(t)) setBlock(w, x, y + hh, z, BLOCK_LOG, LOG_SPRUCE);
    }
}
