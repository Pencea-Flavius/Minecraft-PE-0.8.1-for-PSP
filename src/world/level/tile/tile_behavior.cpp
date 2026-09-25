
#include "world/level/tile/tile_behavior.h"
#include "world/level/tile/tile.h"
#include "world/level/tile/random_tick_pick.h"
#include "world/level/level.h"
#include "world/entity/falling_tile.h"
#include "world/entity/local_player.h"
#include <stdlib.h>
#include <math.h>

static inline bool heavyIsFree(World* w, int x, int y, int z) {
    unsigned char id = worldBlock(w, x, y, z);
    return id == BLOCK_AIR || isLiquidId(id);
}

void heavyTileTick(World* w, int x, int y, int z, unsigned char id) {
    if (y >= 0 && heavyIsFree(w, x, y - 1, z)) {
        FallingTile* e = new FallingTile(&g_level, x + 0.5f, y + 0.5f, z + 0.5f,
                                         id, worldData(w, x, y, z));
        g_level.addEntity(e);
    }
}

bool tileUnobstructedAt(World* w, unsigned char id, int x, int y, int z) {
    Tile* t = Tile::tiles[id];
    BlockAABB boxes[3];
    int n = t->getAABB(w, x, y, z, boxes);
    for (int i = 0; i < n; i++) {
        AABB box(boxes[i].x0, boxes[i].y0, boxes[i].z0,
                 boxes[i].x1, boxes[i].y1, boxes[i].z1);
        if (!g_level.isUnobstructed(box)) return false;
    }
    return true;
}

bool tileMayPlace(World* w, unsigned char id, int x, int y, int z, int face, int data) {
    if (!tileUnobstructedAt(w, id, x, y, z)) return false;

    if (id == BLOCK_TALLGRASS && !bushMayPlaceOn(w, id, data, x, y, z)) return false;
    return Tile::tiles[id]->mayPlace(w, x, y, z, face);
}

void tileNeighborChanged(World* w, int x, int y, int z) {

    Tile::tiles[worldBlock(w, x, y, z)]->neighborChanged(w, x, y, z);
}

static unsigned int s_tick = 0;

static inline int simDistChunks() {
    extern float g_viewDist;
    int d = (int)(g_viewDist / (float)CHUNK_SX) + 1;
    return d < 2 ? 2 : d;
}

void tileRandomTick(World* w) {
    s_tick++;

    const int simD = simDistChunks();
    bool haveP = g_level.player != 0;
    int pcx = 0, pcz = 0;
    if (haveP) {

        pcx = (int)floorf(g_level.player->x) >> 4;
        pcz = (int)floorf(g_level.player->z) >> 4;
    }

    for (int slot = 0; slot < w->slotN * w->slotN; slot++) {
        LevelChunk* lc = &w->slots[slot];
        if (!lc->isAt(lc->x, lc->z) || lc->generating) continue;
        int cx = lc->x, cz = lc->z;

        if (haveP) {
            int dx = cx - pcx, dz = cz - pcz;
            if (dx < 0) dx = -dx;
            if (dz < 0) dz = -dz;
            if (dx > simD || dz > simD) continue;
        }
        int xo = cx * CHUNK_SX, zo = cz * CHUNK_SZ;

        unsigned int chunkIndex = (unsigned int)slot;
        for (unsigned int i = 0; i < SAMPLES_PER_CHUNK; i++) {
            int lx, lz, y;
            randomTickPick(s_tick, chunkIndex, i, &lx, &lz, &y);
            int x = xo + lx, z = zo + lz;
            Tile* t = Tile::tiles[worldBlock(w, x, y, z)];
            if (t->randomTicks) t->randomTick(w, x, y, z);
        }
    }
}
