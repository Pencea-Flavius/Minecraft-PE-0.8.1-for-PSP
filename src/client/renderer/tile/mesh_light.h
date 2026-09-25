
#ifndef MCPSP_CLIENT_RENDERER_TILE_MESH_LIGHT_H
#define MCPSP_CLIENT_RENDERER_TILE_MESH_LIGHT_H

#include "world/level/chunk/chunk.h"
#include "world/level/world.h"

static inline void faceCornerColors(const World* w, const unsigned char* lc,
                                    unsigned char* llc, int nbi, int nx, int ny, int nz,
                                    int f, unsigned char id, unsigned int tint,
                                    unsigned int shade, unsigned int cc[2][2]) {
    float sm[2][2];
    smoothFaceLight(w, lc, llc, nbi, nx, ny, nz, f, sm);
    unsigned int shadeTint = mulColor(shade, tint);
    float emit = g_brightRamp[lightEmit(id)];
    for (int i = 0; i < 2; i++)
    for (int j = 0; j < 2; j++)
        cc[i][j] = mulColor(shadeTint, brightColorF(sm[i][j] < emit ? emit : sm[i][j]));
}

static inline unsigned int tileHash81(int x, int y, int z) {
    unsigned int v = (unsigned int)y
                   ^ (3129871u   * (unsigned int)x)
                   ^ (116129781u * (unsigned int)z);
    return 11u * v + 42317861u * v * v;
}

static const unsigned char kMcpeFaceBit[6] = { 4, 5, 0, 1, 2, 3 };

static inline int faceRotation(unsigned char mask, int f, int x, int y, int z) {
    if (!((mask >> kMcpeFaceBit[f]) & 1)) return 0;
    return (int)((tileHash81(x, y, z) >> 24) & 3u);
}

static inline int pillarFaceRotation(unsigned char id, unsigned char data, int f) {
    if (id != BLOCK_LOG && id != BLOCK_HAY_BLOCK) return 0;
    switch (data & LOG_AXIS_MASK) {
        case LOG_AXIS_X:
            return (f == F_DOWN || f == F_TOP || f == F_BACK || f == F_FORWARD) ? 1 : 0;
        case LOG_AXIS_Z:
            return (f == F_LEFT || f == F_RIGHT) ? 1 : 0;
        default:
            return 0;
    }
}

static inline void applyFaceRot(int r, int f, float* su, float* sv) {
    static const unsigned char kBranchAIsRot1[6] = { 1, 0, 0, 1, 0, 1 };
    static const unsigned char kMirrored[6]      = { 0, 1, 0, 0, 1, 0 };
    const float u = *su, v = *sv;
    if (r == 3) { *su = 1.0f - u; *sv = 1.0f - v; return; }
    const bool branchA = ((r == 1) == (kBranchAIsRot1[f] != 0));
    if (kMirrored[f]) {
        if (branchA) { *su = v;        *sv = u;        }
        else         { *su = 1.0f - v; *sv = 1.0f - u; }
    } else {
        if (branchA) { *su = v;        *sv = 1.0f - u; }
        else         { *su = 1.0f - v; *sv = u;        }
    }
}

static inline void crossJitter81(unsigned char id, int x, int y, int z, float* dx, float* dy, float* dz) {
    *dx = *dy = *dz = 0.0f;
    if (id != BLOCK_TALLGRASS && id != BLOCK_FLOWER && id != BLOCK_ROSE) return;
    const unsigned int h = tileHash81(x, y, z);
    *dx = ((float)((h >> 16) & 0xFu) / 15.0f - 0.5f) * 0.35f;
    *dy = ((float)((h >> 20) & 0xFu) / 15.0f - 1.0f) * 0.1f;
    *dz = ((float)((h >> 24) & 0xFu) / 15.0f - 0.5f) * 0.35f;
}

#endif
