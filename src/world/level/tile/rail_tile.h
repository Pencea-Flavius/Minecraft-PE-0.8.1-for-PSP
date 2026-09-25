
#ifndef MCPSP_WORLD_LEVEL_TILE_RAIL_TILE_H
#define MCPSP_WORLD_LEVEL_TILE_RAIL_TILE_H

struct World;

namespace RailShape {

const int DIR_FLAT_Z = 0;
const int DIR_FLAT_X = 1;
const int DATA_BIT   = 8;

static inline void exits(int dir, int out[2][3]) {
    static const signed char kExits[10][2][3] = {
        { {  0,  0, -1 }, {  0,  0,  1 } },
        { { -1,  0,  0 }, {  1,  0,  0 } },
        { { -1, -1,  0 }, {  1,  0,  0 } },
        { { -1,  0,  0 }, {  1, -1,  0 } },
        { {  0,  0, -1 }, {  0, -1,  1 } },
        { {  0, -1, -1 }, {  0,  0,  1 } },
        { {  0,  0,  1 }, {  1,  0,  0 } },
        { {  0,  0,  1 }, { -1,  0,  0 } },
        { {  0,  0, -1 }, { -1,  0,  0 } },
        { {  0,  0, -1 }, {  1,  0,  0 } },
    };
    if (dir < 0 || dir > 9) dir = 0;
    for (int i = 0; i < 2; i++)
        for (int k = 0; k < 3; k++) out[i][k] = kExits[dir][i][k];
}
}

void railUpdateDir(World* w, int x, int y, int z, bool first);

bool railSupportOk(World* w, int x, int y, int z);

#endif
