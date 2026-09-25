
#ifndef MCPSP_GPU_GU_LIST_SIZE_H
#define MCPSP_GPU_GU_LIST_SIZE_H

static inline int guListSizeIsSane(int ret, unsigned int limit) {
    return ret >= 0 && (unsigned int)ret <= limit * 4u;
}

#endif
