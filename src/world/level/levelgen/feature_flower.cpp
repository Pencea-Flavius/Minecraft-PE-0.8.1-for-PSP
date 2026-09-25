#include "world/level/levelgen/features.h"
#include "world/level/levelgen/mcpegen.h"
#include "world/level/levelgen/Random.h"
#include "world/level/world.h"
#include "world/level/chunk/chunk.h"
#include "world/level/tile/tile_behavior.h"
#include <vector>

struct PendingFlower { int x, y, z; unsigned char tile, data; };
static std::vector<PendingFlower> g_pendingFlowers;

#define PENDING_MAX 4096
unsigned int g_pendingFlowerDrops = 0;

void flowerFeature(World* w, Random& random, int x, int y, int z, unsigned char tile) {
    for (int i = 0; i < 64; i++) {
        int x2 = x + random.nextInt(8) - random.nextInt(8);
        int y2 = y + random.nextInt(4) - random.nextInt(4);
        int z2 = z + random.nextInt(8) - random.nextInt(8);
        if (worldBlock(w, x2, y2, z2) == BLOCK_AIR) {
            unsigned char below = worldBlock(w, x2, y2 - 1, z2);
            if (below == BLOCK_GRASS || below == BLOCK_DIRT) {
                PendingFlower pf = { x2, y2, z2, tile, 0 };
                if (g_pendingFlowers.size() >= PENDING_MAX) { g_pendingFlowerDrops++; continue; }
                g_pendingFlowers.push_back(pf);
            }
        }
    }
}

void tallGrassFeature(World* w, Random& random, int x, int y, int z, unsigned char type) {
    unsigned char t;
    while (y > 0 && ((t = worldBlock(w, x, y, z)) == BLOCK_AIR || t == BLOCK_LEAVES))
        y--;

    for (int i = 0; i < 128; i++) {
        int x2 = x + random.nextInt(8) - random.nextInt(8);
        int y2 = y + random.nextInt(4) - random.nextInt(4);
        int z2 = z + random.nextInt(8) - random.nextInt(8);
        if (worldBlock(w, x2, y2, z2) == BLOCK_AIR) {
            unsigned char below = worldBlock(w, x2, y2 - 1, z2);
            if (below == BLOCK_GRASS || below == BLOCK_DIRT) {
                PendingFlower pf = { x2, y2, z2, BLOCK_TALLGRASS, type };
                if (g_pendingFlowers.size() >= PENDING_MAX) { g_pendingFlowerDrops++; continue; }
                g_pendingFlowers.push_back(pf);
            }
        }
    }
}

void deadBushFeature(World* w, Random& random, int x, int y, int z) {
    unsigned char t;
    while (y > 0 && ((t = worldBlock(w, x, y, z)) == BLOCK_AIR || t == BLOCK_LEAVES))
        y--;

    for (int i = 0; i < 4; i++) {
        int x2 = x + random.nextInt(8) - random.nextInt(8);
        int y2 = y + random.nextInt(4) - random.nextInt(4);
        int z2 = z + random.nextInt(8) - random.nextInt(8);
        if (worldBlock(w, x2, y2, z2) != BLOCK_AIR) continue;
        if (worldBlock(w, x2, y2 - 1, z2) != BLOCK_SAND) continue;
        PendingFlower pf = { x2, y2, z2, BLOCK_TALLGRASS, TG_DEAD_SHRUB };
        if (g_pendingFlowers.size() >= PENDING_MAX) { g_pendingFlowerDrops++; continue; }
        g_pendingFlowers.push_back(pf);
    }
}

void pumpkinFeature(World* w, Random& random, int x, int y, int z) {

    unsigned char t;
    while (y > 0 && ((t = worldBlock(w, x, y, z)) == BLOCK_AIR || t == BLOCK_LEAVES))
        y--;

    for (int i = 0; i < 64; i++) {
        int x2 = x + random.nextInt(8) - random.nextInt(8);
        int y2 = y + random.nextInt(4) - random.nextInt(4);
        int z2 = z + random.nextInt(8) - random.nextInt(8);
        int face = random.nextInt(4);
        if (worldBlock(w, x2, y2, z2) != BLOCK_AIR) continue;
        if (worldBlock(w, x2, y2 - 1, z2) != BLOCK_GRASS) continue;

        setBlock(w, x2, y2, z2, BLOCK_PUMPKIN, (unsigned char)face);
    }
}

void worldPlaceFlowers(World* w) {
    for (size_t i = 0; i < g_pendingFlowers.size(); i++) {
        const PendingFlower& f = g_pendingFlowers[i];

        unsigned char here = worldBlock(w, f.x, f.y, f.z);
        if (here != BLOCK_AIR && here != BLOCK_TOPSNOW) continue;

        if (!bushMayPlaceOn(w, f.tile, f.data, f.x, f.y, f.z)) continue;
        if (lightRawAt(w, f.x, f.y, f.z) >= 8 || worldCanSeeSky(w, f.x, f.y, f.z))
            setBlock(w, f.x, f.y, f.z, f.tile, f.data);
    }
    g_pendingFlowers.clear();
}
