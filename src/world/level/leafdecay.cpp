
#include "world/level/world.h"
#include "world/level/chunk/chunk.h"
#include <string.h>

static const int REQUIRED_WOOD_RANGE = 4;

#define W  11
#define WO 5
#define WW (W * W)
static int g_leafBuf[W * W * W];

void leafFlagNeighbors(World* w, int x, int y, int z) {
    for (int xo = -1; xo <= 1; xo++)
    for (int yo = -1; yo <= 1; yo++)
    for (int zo = -1; zo <= 1; zo++) {
        int xx = x + xo, yy = y + yo, zz = z + zo;
        if (yy < 0 || yy >= WORLD_H || !worldReady(w, xx, zz)) continue;
        unsigned char id = worldBlock(w, xx, yy, zz);
        if (!isLeaf(id)) continue;
        unsigned char d = worldData(w, xx, yy, zz);
        if (d & LEAF_PERSISTENT_BIT) continue;
        worldSetDataNoUpdate(w, xx, yy, zz, (unsigned char)(d | LEAF_UPDATE_BIT));
    }
}

void leafDecayTick(World* w, int x, int y, int z) {
    int data = worldData(w, x, y, z);
    if ((data & LEAF_UPDATE_BIT) == 0 || (data & LEAF_PERSISTENT_BIT) != 0) return;

    const int r = REQUIRED_WOOD_RANGE;

    for (int i = 0; i < W * W * W; i++) g_leafBuf[i] = -1;

    for (int xo = -r; xo <= r; xo++)
    for (int yo = -r; yo <= r; yo++)
    for (int zo = -r; zo <= r; zo++) {
        int xx = x + xo, yy = y + yo, zz = z + zo;
        int t;
        if (yy < 0 || yy >= WORLD_H || !worldReady(w, xx, zz)) t = 0;
        else t = worldBlock(w, xx, yy, zz);
        int* cell = &g_leafBuf[(xo + WO) * WW + (yo + WO) * W + (zo + WO)];
        if (isLog(t))       *cell = 0;
        else if (isLeaf(t)) *cell = -2;
        else                *cell = -1;
    }

    for (int i = 1; i <= r; i++)
        for (int xo = -r; xo <= r; xo++)
        for (int yo = -r; yo <= r; yo++)
        for (int zo = -r; zo <= r; zo++) {
            int* c = &g_leafBuf[(xo + WO) * WW + (yo + WO) * W + (zo + WO)];
            if (*c != i - 1) continue;
            int* n;
            n = c - WW; if (*n == -2) *n = i;
            n = c + WW; if (*n == -2) *n = i;
            n = c - W;  if (*n == -2) *n = i;
            n = c + W;  if (*n == -2) *n = i;
            n = c - 1;  if (*n == -2) *n = i;
            n = c + 1;  if (*n == -2) *n = i;
        }

    if (g_leafBuf[WO * WW + WO * W + WO] >= 0) {
        worldSetDataNoUpdate(w, x, y, z, (unsigned char)(data & ~LEAF_UPDATE_BIT));
    } else {

        worldSpawnResources(w, x, y, z, worldBlock(w, x, y, z), data);
        worldSetTileUpdate(w, x, y, z, BLOCK_AIR, 0);
        leafFlagNeighbors(w, x, y, z);
    }
}
