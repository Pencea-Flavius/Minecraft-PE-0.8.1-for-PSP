#ifndef MCPSP_WORLD_ITEM_DIAL_MATH_H
#define MCPSP_WORLD_ITEM_DIAL_MATH_H

#include <math.h>

namespace Dial {

const int CLOCK_FRAMES   = 64;
const int COMPASS_FRAMES = 32;

inline int clockFrame(float timeOfDay) {
    int f = (int)(timeOfDay * (float)CLOCK_FRAMES);
    if (f < 0) return 0;
    if (f >= CLOCK_FRAMES) return CLOCK_FRAMES - 1;
    return f;
}

inline int compassFrame(float spawnX, float spawnZ, float px, float pz, float yawDeg) {
    float dx = spawnX - px;
    float dz = spawnZ - pz;
    float len = sqrtf(dx * dx + dz * dz);
    if (len >= 0.0001f) { dx /= len; dz /= len; }
    else                { dx = 0.0f;  dz = 0.0f; }

    float a = atan2f(dz, dx) * 57.296f - yawDeg + 90.0f;
    while (a < 0.0f)    a += 360.0f;
    while (a >= 360.0f) a -= 360.0f;

    int f = (int)(a / 360.0f * (float)COMPASS_FRAMES);
    if (f < 0) return 0;
    if (f >= COMPASS_FRAMES) return COMPASS_FRAMES - 1;
    return f;
}

inline void clockCell(int frame, int* col, int* row) { *col = frame / 8; *row = frame % 8; }

}

#endif
