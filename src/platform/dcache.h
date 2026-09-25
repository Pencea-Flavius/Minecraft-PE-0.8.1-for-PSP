
#ifndef MCPSP_PLATFORM_DCACHE_H
#define MCPSP_PLATFORM_DCACHE_H

#include <pspkernel.h>
#include <stddef.h>

static inline void dcacheFlush(const void* p, size_t bytes) {
    unsigned int a = (unsigned int)p & ~63u;
    unsigned int e = ((unsigned int)p + (unsigned int)bytes + 63u) & ~63u;
    sceKernelDcacheWritebackRange((void*)a, e - a);
}

#endif
