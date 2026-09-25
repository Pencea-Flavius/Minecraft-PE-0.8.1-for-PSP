#include "world/level/levelgen/features.h"
#include "world/level/levelgen/Random.h"
#include "world/level/world.h"

void setBlock(World* w, int x, int y, int z, unsigned char id, unsigned char data) {
    if (y < 0 || y >= WORLD_H || !worldReady(w, x, z)) return;
    worldSetBlockAndData(w, x, y, z, id, data);
    if (id == BLOCK_WATER || id == BLOCK_LAVA) {
        worldScheduleTick(w, x, y, z, id, (id == BLOCK_WATER) ? 5 : 30);
    }
}

bool isSolidGen(unsigned char id) {
    return id != BLOCK_AIR && !isWaterId(id) && !isLeaf(id) &&
           id != BLOCK_FLOWER && id != BLOCK_ROSE &&
           id != BLOCK_MUSHROOM_BROWN && id != BLOCK_MUSHROOM_RED;
}

int heightmapAt(World* w, int x, int z) {
    for (int y = WORLD_H - 1; y >= 0; y--) {
        unsigned char b = worldBlock(w, x, y, z);
        if (b == BLOCK_GRASS || b == BLOCK_DIRT || b == BLOCK_SAND ||
            b == BLOCK_STONE || b == BLOCK_GRAVEL || b == BLOCK_SANDSTONE ||
            b == BLOCK_CLAY || b == BLOCK_BEDROCK)
            return y + 1;
    }
    return 0;
}

bool isTreeClear(unsigned char b) {
    return b == BLOCK_AIR || isLeaf(b);
}

bool treeSpaceClear(World* w, int x, int y, int z, int treeHeight,
                    int (*radiusAt)(int, int, int), int arg) {
    if (y < 1 || y + treeHeight + 1 > WORLD_H) return false;
    for (int yy = y; yy <= y + 1 + treeHeight; yy++) {
        int r = radiusAt(yy - y, treeHeight, arg);
        for (int xx = x - r; xx <= x + r; xx++)
        for (int zz = z - r; zz <= z + r; zz++)
            if (!isTreeClear(worldBlock(w, xx, yy, zz))) return false;
    }
    unsigned char below = worldBlock(w, x, y - 1, z);
    if (below != BLOCK_GRASS && below != BLOCK_DIRT) return false;
    return true;
}

static int basicRadiusAt(int layer, int treeHeight, int) {
    if (layer == 0) return 0;
    return (layer >= 1 + treeHeight - 2) ? 2 : 1;
}

void treeBasic(World* w, Random& random, int x, int y, int z,
               int minHeight, unsigned char leafData, unsigned char logData) {
    int treeHeight = random.nextInt(3) + minHeight;
    if (!treeSpaceClear(w, x, y, z, treeHeight, basicRadiusAt, 0)) return;
    setBlock(w, x, y - 1, z, BLOCK_DIRT);
    for (int yy = y - 3 + treeHeight; yy <= y + treeHeight; yy++) {
        int yo = yy - (y + treeHeight);
        int offs = 1 - yo / 2;
        for (int xx = x - offs; xx <= x + offs; xx++) {
            int axo = (xx - x < 0) ? -(xx - x) : (xx - x);
            for (int zz = z - offs; zz <= z + offs; zz++) {
                int azo = (zz - z < 0) ? -(zz - z) : (zz - z);
                if (axo == offs && azo == offs && (random.nextInt(2) == 0 || yo == 0)) continue;
                if (!isSolidGen(worldBlock(w, xx, yy, zz))) setBlock(w, xx, yy, zz, BLOCK_LEAVES, leafData);
            }
        }
    }
    for (int hh = 0; hh < treeHeight; hh++) {
        unsigned char t = worldBlock(w, x, y + hh, z);
        if (!isSolidGen(t)) setBlock(w, x, y + hh, z, BLOCK_LOG, logData);
    }
}
