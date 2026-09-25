
#include "world/level/world.h"

#include <stdlib.h>
#include <string.h>

static const int LIGHT_PLANES = 2 * (WORLD_CHUNKS_X * WORLD_CHUNKS_Z) * WORLD_H;

bool lightAlloc(World* w) {
    memset(w->lightPool, 0, sizeof(w->lightPool));
    w->lightBlocksUsed   = 0;
    w->lightPagesUsed    = 0;
    w->lightPagesAlloced = 0;
    w->lightFreeHead     = LP_ALL0;
    w->lightOomDrops     = 0;

    w->lightIdx = (unsigned short*)malloc((size_t)LIGHT_PLANES * sizeof(unsigned short));
    if (!w->lightIdx) return false;

    for (int i = 0; i < LIGHT_PLANES; i++) w->lightIdx[i] = LP_ALL0;
    return true;
}

void lightFree(World* w) {
    for (int i = 0; i < w->lightBlocksUsed; i++)
        if (w->lightPool[i]) { free(w->lightPool[i]); w->lightPool[i] = 0; }
    w->lightBlocksUsed   = 0;
    w->lightPagesUsed    = 0;
    w->lightPagesAlloced = 0;
    w->lightFreeHead     = LP_ALL0;
    if (w->lightIdx) { free(w->lightIdx); w->lightIdx = 0; }
}

void lightClearAll(World* w) {
    for (int i = 0; i < LIGHT_PLANES; i++) w->lightIdx[i] = LP_ALL0;
    w->lightPagesUsed    = 0;
    w->lightPagesAlloced = 0;
    w->lightFreeHead     = LP_ALL0;
}

unsigned int lightPagePromote(World* w, int idxSlot, unsigned char prefill) {
    unsigned int id;

    if (w->lightFreeHead != LP_ALL0) {

        id = w->lightFreeHead;
        unsigned char* p = lightPage(w, id);
        w->lightFreeHead = (unsigned short)(p[0] | (p[1] << 8));
    } else {
        int blk = w->lightPagesAlloced >> 8;
        if (blk >= LP_MAX_BLKS) { w->lightOomDrops++; return LP_ALL0; }
        if (blk >= w->lightBlocksUsed) {
            unsigned char* nb = (unsigned char*)malloc(LP_BLK_PAGES * LP_PAGE);
            if (!nb) { w->lightOomDrops++; return LP_ALL0; }
            w->lightPool[blk] = nb;
            w->lightBlocksUsed = blk + 1;
        }
        id = (unsigned int)w->lightPagesAlloced++;
    }

    memset(lightPage(w, id), prefill, LP_PAGE);
    w->lightIdx[idxSlot] = (unsigned short)id;
    w->lightPagesUsed++;
    return id;
}

static void lightPageFree(World* w, unsigned int id) {
    unsigned char* p = lightPage(w, id);
    p[0] = (unsigned char)(w->lightFreeHead & 0xFF);
    p[1] = (unsigned char)(w->lightFreeHead >> 8);
    w->lightFreeHead = (unsigned short)id;
    w->lightPagesUsed--;
}

void lightSlotRelease(World* w, int slotIdx) {
    int slot0 = (slotIdx << 7) << 1;
    for (int i = 0; i < 2 * WORLD_H; i++) {
        unsigned int id = w->lightIdx[slot0 + i];
        if (id < LP_SENT) lightPageFree(w, id);
        w->lightIdx[slot0 + i] = LP_ALL0;
    }
}

void lightChunkClearBlock(World* w, int cx, int cz) {
    int slot0 = lightPlaneSlot0(w, 1, cx, cz);
    for (int y = 0; y < WORLD_H; y++) {
        int slot = slot0 + y * 2;
        unsigned int id = w->lightIdx[slot];
        if (id < LP_SENT) lightPageFree(w, id);
        w->lightIdx[slot] = LP_ALL0;
    }
}

void lightInitSkyChunk(World* w, int cx, int cz) {
            int hmin = WORLD_H, hmax = 0;
            for (int lx = 0; lx < 16; lx++)
                for (int lz = 0; lz < 16; lz++) {
                    int hy = w->heightmap[worldColumn(w, cx * 16 + lx, cz * 16 + lz)];
                    if (hy < hmin) hmin = hy;
                    if (hy > hmax) hmax = hy;
                }
            int slot0 = lightPlaneSlot0(w, 0, cx, cz);
            for (int y = 0; y < WORLD_H; y++) {
                int slot = slot0 + y * 2;

                unsigned int old = w->lightIdx[slot];
                if (old < LP_SENT) { w->lightIdx[slot] = LP_ALL0; lightPageFree(w, old); }
                if (y < hmin)  { w->lightIdx[slot] = LP_ALL0;  continue; }
                if (y >= hmax) { w->lightIdx[slot] = LP_ALL15; continue; }
                unsigned int id = lightPagePromote(w, slot, 0x00);
                if (id >= LP_SENT) continue;
                unsigned char* p = lightPage(w, id);
                for (int lx = 0; lx < 16; lx++)
                    for (int lz = 0; lz < 16; lz++) {
                        if (y < w->heightmap[worldColumn(w, cx * 16 + lx, cz * 16 + lz)]) continue;
                        int pi = (lx << 4) | lz;
                        p[pi >> 1] |= (unsigned char)(0x0F << ((pi & 1) * 4));
                    }
            }
}

void lightInitSkyFromHeightmap(World* w) {
    for (int cx = 0; cx < WORLD_CHUNKS_X; cx++) {
        for (int cz = 0; cz < WORLD_CHUNKS_Z; cz++) {
            int hmin = WORLD_H, hmax = 0;
            for (int lx = 0; lx < 16; lx++)
                for (int lz = 0; lz < 16; lz++) {
                    int hy = w->heightmap[worldColumn(w, cx * 16 + lx, cz * 16 + lz)];
                    if (hy < hmin) hmin = hy;
                    if (hy > hmax) hmax = hy;
                }
            int slot0 = lightPlaneSlot0(w, 0, cx, cz);
            for (int y = 0; y < WORLD_H; y++) {
                int slot = slot0 + y * 2;
                if (y < hmin)       { w->lightIdx[slot] = LP_ALL0;  continue; }
                if (y >= hmax)      { w->lightIdx[slot] = LP_ALL15; continue; }
                unsigned int id = lightPagePromote(w, slot, 0x00);
                if (id >= LP_SENT) continue;
                unsigned char* p = lightPage(w, id);
                for (int lx = 0; lx < 16; lx++)
                    for (int lz = 0; lz < 16; lz++) {
                        if (y < w->heightmap[worldColumn(w, cx * 16 + lx, cz * 16 + lz)]) continue;
                        int pi = (lx << 4) | lz;
                        p[pi >> 1] |= (unsigned char)(0x0F << ((pi & 1) * 4));
                    }
            }
        }
    }
}

static void loadLayerPlanes(World* w, int layer, int cx, int cz, const unsigned char* nib) {
    int slot0 = lightPlaneSlot0(w, layer, cx, cz);
    for (int y = 0; y < WORLD_H; y++) {

        int idx0 = (0 << 11) | (0 << 7) | y;
        unsigned char first = (unsigned char)((nib[idx0 >> 1] >> ((idx0 & 1) * 4)) & 15);
        bool uniform = (first == 0 || first == 15);
        if (uniform) {
            for (int lx = 0; lx < 16 && uniform; lx++)
                for (int lz = 0; lz < 16; lz++) {
                    int i = (lx << 11) | (lz << 7) | y;
                    if ((unsigned char)((nib[i >> 1] >> ((i & 1) * 4)) & 15) != first) { uniform = false; break; }
                }
        }
        int slot = slot0 + y * 2;
        if (uniform) { w->lightIdx[slot] = first ? LP_ALL15 : LP_ALL0; continue; }

        unsigned int id = lightPagePromote(w, slot, 0x00);
        if (id >= LP_SENT) continue;
        unsigned char* p = lightPage(w, id);
        for (int lx = 0; lx < 16; lx++)
            for (int lz = 0; lz < 16; lz++) {
                int i = (lx << 11) | (lz << 7) | y;
                unsigned char v = (unsigned char)((nib[i >> 1] >> ((i & 1) * 4)) & 15);
                if (!v) continue;
                int pi = (lx << 4) | lz;
                p[pi >> 1] |= (unsigned char)(v << ((pi & 1) * 4));
            }
    }
}

void lightLoadChunk(World* w, int cx, int cz,
                    const unsigned char* skyNib, const unsigned char* blockNib) {
    loadLayerPlanes(w, 0, cx, cz, skyNib);
    loadLayerPlanes(w, 1, cx, cz, blockNib);
}

static bool compactGroup(World* w, int group) {

    int slot0 = (group >> 1) * (WORLD_H * 2) + (group & 1);
    bool any = false;
    for (int y = 0; y < WORLD_H; y++) {
        int slot = slot0 + y * 2;
        unsigned int id = w->lightIdx[slot];
        if (id >= LP_SENT) continue;
        const unsigned char* p = lightPage(w, id);
        unsigned char first = p[0];
        if (first != 0x00 && first != 0xFF) continue;
        bool uniform = true;
        for (int i = 1; i < LP_PAGE; i++)
            if (p[i] != first) { uniform = false; break; }
        if (!uniform) continue;
        w->lightIdx[slot] = (first == 0xFF) ? LP_ALL15 : LP_ALL0;
        lightPageFree(w, id);
        any = true;
    }
    return any;
}

void lightCompactStep(World* w) {
    static int cursor = 0;
    compactGroup(w, cursor);
    cursor = (cursor + 1) % (WORLD_CHUNKS_X * WORLD_CHUNKS_Z * 2);
}

void lightCompactAll(World* w) {
    for (int g = 0; g < WORLD_CHUNKS_X * WORLD_CHUNKS_Z * 2; g++) compactGroup(w, g);
}

unsigned int lightBytes(const World* w) {
    return (unsigned int)LIGHT_PLANES * sizeof(unsigned short)
         + (unsigned int)w->lightBlocksUsed * (LP_BLK_PAGES * LP_PAGE);
}
