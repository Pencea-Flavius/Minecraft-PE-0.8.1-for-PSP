
#ifndef MCPSP_GPU_VRAM_ALLOC_H
#define MCPSP_GPU_VRAM_ALLOC_H

#define VRAM_ALLOC_NONE  0xFFFFFFFFu
#define VRAM_MAX_BLOCKS  16

typedef struct {
    unsigned int offset, size;
    unsigned char used;
} VramBlock;

static VramBlock  s_vramBlocks[VRAM_MAX_BLOCKS];
static int        s_vramBlockCount = 0;
static unsigned int s_vramTop   = 0;
static unsigned int s_vramLimit = 0;

static inline void vramAllocInit(unsigned int base, unsigned int limit) {
    s_vramBlockCount = 0;
    s_vramTop   = base;
    s_vramLimit = limit;
}

static inline unsigned int vramAlloc(unsigned int bytes) {
    if (bytes == 0) return VRAM_ALLOC_NONE;
    bytes = (bytes + 63u) & ~63u;

    for (int i = 0; i < s_vramBlockCount; i++)
        if (!s_vramBlocks[i].used && s_vramBlocks[i].size >= bytes) {
            s_vramBlocks[i].used = 1;
            return s_vramBlocks[i].offset;
        }

    if (bytes > s_vramLimit || s_vramTop > s_vramLimit - bytes) return VRAM_ALLOC_NONE;
    if (s_vramBlockCount >= VRAM_MAX_BLOCKS) return VRAM_ALLOC_NONE;

    unsigned int off = s_vramTop;
    s_vramTop += bytes;
    s_vramBlocks[s_vramBlockCount].offset = off;
    s_vramBlocks[s_vramBlockCount].size   = bytes;
    s_vramBlocks[s_vramBlockCount].used   = 1;
    s_vramBlockCount++;
    return off;
}

static inline void vramFreeAt(unsigned int offset) {
    if (offset == VRAM_ALLOC_NONE) return;
    for (int i = 0; i < s_vramBlockCount; i++) {
        if (s_vramBlocks[i].offset != offset || !s_vramBlocks[i].used) continue;
        s_vramBlocks[i].used = 0;

        while (s_vramBlockCount > 0 && !s_vramBlocks[s_vramBlockCount - 1].used) {
            s_vramTop = s_vramBlocks[s_vramBlockCount - 1].offset;
            s_vramBlockCount--;
        }
        return;
    }
}

static inline unsigned int vramBytesFree(void) {
    unsigned int free = s_vramTop >= s_vramLimit ? 0u : s_vramLimit - s_vramTop;
    for (int i = 0; i < s_vramBlockCount; i++)
        if (!s_vramBlocks[i].used) free += s_vramBlocks[i].size;
    return free;
}

#endif
