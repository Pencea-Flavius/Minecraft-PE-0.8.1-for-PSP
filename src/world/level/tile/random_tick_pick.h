
#ifndef RANDOM_TICK_PICK_H
#define RANDOM_TICK_PICK_H

#include "world/level/world.h"

#define SAMPLES_PER_CHUNK 20

typedef char assert_pow2_dims[((CHUNK_SX & (CHUNK_SX - 1)) == 0 &&
                               (CHUNK_SZ & (CHUNK_SZ - 1)) == 0 &&
                               (WORLD_H & (WORLD_H - 1)) == 0) ? 1 : -1];

static inline unsigned int rtMix32(unsigned int h) {
    h ^= h >> 16; h *= 0x85ebca6bu;
    h ^= h >> 13; h *= 0xc2b2ae35u;
    h ^= h >> 16;
    return h;
}

static inline void randomTickPick(unsigned int tick, unsigned int chunkIndex,
                                  unsigned int sample, int* lx, int* lz, int* y) {
    unsigned int v = rtMix32(tick        * 0x9e3779b9u +
                             chunkIndex  * 0x7feb352du +
                             sample      * 0x846ca68bu);
    *lx = (int)( v        & (CHUNK_SX - 1));
    *lz = (int)((v >>  8) & (CHUNK_SZ - 1));
    *y  = (int)((v >> 16) & (WORLD_H  - 1));
}

#endif
