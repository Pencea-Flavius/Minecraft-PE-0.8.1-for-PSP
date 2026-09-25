#include "world/level/world.h"
#include "world/level/tile/tile_shapes.h"
#include "world/level/chunk/chunk.h"
#include "world/level/tile/fire.h"
#include "client/player/physics.h"
#include <math.h>

static inline int wpFloor(float v) { int i = (int)v; return (v < 0 && v != i) ? i - 1 : i; }

struct SelectionAABB { float x0, y0, z0, x1, y1, z1; };

static int getSelectionAABBs(const World* w, int x, int y, int z, SelectionAABB out[3], bool clipLiquids) {
    unsigned char id = worldBlock(w, x, y, z);
    if (id == BLOCK_AIR) return 0;

    if (id == BLOCK_FIRE) return 0;
    if (isLiquidId(id)) {

        if (!clipLiquids || worldData(w, x, y, z) != 0) return 0;
        out[0] = { x + 0.0f, y + 0.0f, z + 0.0f, x + 1.0f, y + 1.0f, z + 1.0f };
        return 1;
    }

    float b[3][6];
    int n = tileShapeBoxes(w, x, y, z, id, worldData(w, x, y, z), b);
    for (int i = 0; i < n; i++)
        out[i] = { b[i][0], b[i][1], b[i][2], b[i][3], b[i][4], b[i][5] };
    return n;
}

int worldSelectionBoxes(const World* w, int x, int y, int z, float out[3][6]) {
    SelectionAABB b[3];
    int n = getSelectionAABBs(w, x, y, z, b, false);
    for (int i = 0; i < n; i++) {
        out[i][0] = b[i].x0; out[i][1] = b[i].y0; out[i][2] = b[i].z0;
        out[i][3] = b[i].x1; out[i][4] = b[i].y1; out[i][5] = b[i].z1;
    }
    return n;
}

static const float CLIP_EPS = 1e-4f;

static bool clipAxis(float p, float d, float minV, float maxV, int fMin, int fMax, float& tmin, float& tmax, int& faceMin) {
    if (fabsf(d) < 1e-6f) {
        if (p < minV || p > maxV) return false;
    } else {
        float t1 = (minV - p) / d;
        float t2 = (maxV - p) / d;
        int f1 = fMin, f2 = fMax;
        if (t1 > t2) { float tmp = t1; t1 = t2; t2 = tmp; int fTmp = f1; f1 = f2; f2 = fTmp; }
        if (t1 > tmin) { tmin = t1; faceMin = f1; }
        if (t2 < tmax) tmax = t2;
        if (tmin > tmax + CLIP_EPS) return false;
        if (tmax < 0.0f) return false;
    }
    return true;
}

static bool clipAABB(float px, float py, float pz, float dx, float dy, float dz, float range, const SelectionAABB& box, float& outT, int& outFace) {
    float tmin = -1e9f, tmax = 1e9f;
    int faceMin = -1;

    if (!clipAxis(px, dx, box.x0, box.x1, F_LEFT, F_RIGHT, tmin, tmax, faceMin)) return false;
    if (!clipAxis(py, dy, box.y0, box.y1, F_DOWN, F_TOP, tmin, tmax, faceMin)) return false;
    if (!clipAxis(pz, dz, box.z0, box.z1, F_BACK, F_FORWARD, tmin, tmax, faceMin)) return false;

    if (tmin >= 0.0f && tmin <= range) {
        outT = tmin;
        outFace = faceMin;
        return true;
    }
    if (tmin < 0.0f && tmax >= 0.0f && tmax <= range) {
        outT = 0.0f;
        outFace = faceMin;
        return true;
    }
    return false;
}

static bool pickCell(const World* w, int cx, int cy, int cz,
                     float px, float py, float pz, float dx, float dy, float dz,
                     float range, BlockHit& out, bool clipLiquids, bool solidOnly) {
    unsigned char blk = worldBlock(w, cx, cy, cz);
    if (blk == BLOCK_AIR) return false;
    if (isLiquidId(blk) && !clipLiquids) return false;

    if (solidOnly) {
        BlockAABB cb[3];
        if (getBlockAABBs(w, cx, cy, cz, cb) == 0) return false;
    }
    SelectionAABB boxes[3];
    int numBoxes = getSelectionAABBs(w, cx, cy, cz, boxes, clipLiquids);
    float bestT = 1e9f;
    int bestFace = -1;
    for (int i = 0; i < numBoxes; i++) {
        float t; int f;
        if (clipAABB(px, py, pz, dx, dy, dz, range, boxes[i], t, f)) {
            if (t < bestT) { bestT = t; bestFace = f; }
        }
    }
    if (bestFace == -1) return false;
    float hx = px + dx * bestT, hy = py + dy * bestT, hz = pz + dz * bestT;
    BlockHit hit = { true, cx, cy, cz, bestFace, hx - cx, hy - cy, hz - cz };
    out = hit;
    return true;
}

static BlockHit worldClipDir(const World* w, float px, float py, float pz,
                             float dx, float dy, float dz, float range, bool clipLiquids, bool solidOnly);

BlockHit worldClip(const World* w, float ax, float ay, float az,
                   float bx, float by, float bz, bool clipLiquids, bool solidOnly) {
    float sdx = bx - ax, sdy = by - ay, sdz = bz - az;
    float range = sqrtf(sdx * sdx + sdy * sdy + sdz * sdz);
    BlockHit noMove = {false, 0, 0, 0, 0};
    if (range < 1e-6f) return noMove;
    float dx = sdx / range, dy = sdy / range, dz = sdz / range;
    return worldClipDir(w, ax, ay, az, dx, dy, dz, range, clipLiquids, solidOnly);
}

BlockHit worldPick(const World* w, float px, float py, float pz, float yaw, float pitch, float range,
                   bool clipLiquids) {
    const float DEG2RAD = 3.14159265f / 180.0f;
    float cy = cosf(yaw * DEG2RAD), sy = sinf(yaw * DEG2RAD);
    float cp = cosf(pitch * DEG2RAD), sp = sinf(pitch * DEG2RAD);
    float dx = -cp * sy, dy = sp, dz = cp * cy;
    return worldClipDir(w, px, py, pz, dx, dy, dz, range, clipLiquids, false);
}

static BlockHit worldClipDir(const World* w, float px, float py, float pz,
                             float dx, float dy, float dz, float range, bool clipLiquids, bool solidOnly) {
    float ax = px, ay = py, az = pz;
    float bx = px + dx * range, by = py + dy * range, bz = pz + dz * range;

    int xTile0 = wpFloor(ax), yTile0 = wpFloor(ay), zTile0 = wpFloor(az);
    int xTile1 = wpFloor(bx), yTile1 = wpFloor(by), zTile1 = wpFloor(bz);

    BlockHit miss = {false, 0, 0, 0, 0};

    BlockHit first;
    if (pickCell(w, xTile0, yTile0, zTile0, px, py, pz, dx, dy, dz, range, first, clipLiquids, solidOnly))
        return first;

    int maxIterations = 200;
    while (maxIterations-- >= 0) {
        if (xTile0 == xTile1 && yTile0 == yTile1 && zTile0 == zTile1) return miss;

        float xClip = 1e9f, yClip = 1e9f, zClip = 1e9f;
        if (xTile1 > xTile0) xClip = xTile0 + 1.0f;
        if (xTile1 < xTile0) xClip = xTile0 + 0.0f;
        if (yTile1 > yTile0) yClip = yTile0 + 1.0f;
        if (yTile1 < yTile0) yClip = yTile0 + 0.0f;
        if (zTile1 > zTile0) zClip = zTile0 + 1.0f;
        if (zTile1 < zTile0) zClip = zTile0 + 0.0f;

        float xd = bx - ax, yd = by - ay, zd = bz - az;
        float xDist = 1e9f, yDist = 1e9f, zDist = 1e9f;
        if (xClip < 1e9f) xDist = (xClip - ax) / xd;
        if (yClip < 1e9f) yDist = (yClip - ay) / yd;
        if (zClip < 1e9f) zDist = (zClip - az) / zd;

        int sx = 0, sy = 0, sz = 0;
        if (xDist < yDist && xDist < zDist) {
            ax = xClip; ay += yd * xDist; az += zd * xDist;
            sx = xTile1 > xTile0 ? 1 : -1;
        } else if (yDist < zDist) {
            ax += xd * yDist; ay = yClip; az += zd * yDist;
            sy = yTile1 > yTile0 ? 1 : -1;
        } else {
            ax += xd * zDist; ay += yd * zDist; az = zClip;
            sz = zTile1 > zTile0 ? 1 : -1;
        }

        xTile0 += sx;
        yTile0 += sy;
        zTile0 += sz;

        BlockHit hit;
        if (pickCell(w, xTile0, yTile0, zTile0, px, py, pz, dx, dy, dz, range, hit, clipLiquids, solidOnly))
            return hit;
    }
    return miss;
}
