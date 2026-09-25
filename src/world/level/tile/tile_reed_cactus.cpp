
#include "world/level/tile/tile_behavior.h"
#include "world/level/tile/material.h"

bool reedCanSurvive(World* w, int x, int y, int z) {
    unsigned char below = worldBlock(w, x, y - 1, z);
    if (below == BLOCK_REEDS) return true;
    if (below != BLOCK_GRASS && below != BLOCK_DIRT && below != BLOCK_SAND) return false;
    return isWaterId(worldBlock(w, x - 1, y - 1, z)) || isWaterId(worldBlock(w, x + 1, y - 1, z)) ||
           isWaterId(worldBlock(w, x, y - 1, z - 1)) || isWaterId(worldBlock(w, x, y - 1, z + 1));
}

bool cactusCanSurvive(World* w, int x, int y, int z) {
    if (materialOf(worldBlock(w, x - 1, y, z)).isSolid() || materialOf(worldBlock(w, x + 1, y, z)).isSolid() ||
        materialOf(worldBlock(w, x, y, z - 1)).isSolid() || materialOf(worldBlock(w, x, y, z + 1)).isSolid()) return false;
    unsigned char below = worldBlock(w, x, y - 1, z);
    return below == BLOCK_CACTUS || below == BLOCK_SAND;
}

void reedCactusGrow(World* w, int x, int y, int z, unsigned char id, int ageThreshold) {
    if (worldBlock(w, x, y + 1, z) != BLOCK_AIR) return;
    int height = 1;
    while (worldBlock(w, x, y - height, z) == id) height++;
    if (height >= 3) return;
    int age = worldData(w, x, y, z);
    if (age >= ageThreshold) {
        worldSetTileUpdate(w, x, y + 1, z, id, 0);
        worldSetDataNoUpdate(w, x, y, z, 0);
    } else {
        worldSetDataNoUpdate(w, x, y, z, (unsigned char)(age + 1));
    }
}

bool bonemealReed(World* w, int x, int y, int z) {
    int below = y - 1;
    while (below > 0 && worldBlock(w, x, below, z) == BLOCK_REEDS) below--;
    int above = y + 1;
    while (above <= WORLD_H - 1 && worldBlock(w, x, above, z) == BLOCK_REEDS) above++;

    const int grow = below - (above - 1) + 3;
    if (grow <= 0) return false;

    const int top = above - 1 + grow;
    for (int yy = above; yy <= top; yy++)
        worldSetTileUpdate(w, x, yy, z, BLOCK_REEDS, 0);
    return true;
}
