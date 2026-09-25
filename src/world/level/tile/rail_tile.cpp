#include "world/level/tile/rail_tile.h"
#include "world/level/world.h"
#include "world/level/tile/tile.h"
#include "world/level/tile/material.h"

namespace {

struct Rail {
    World* w;
    int x, y, z;
    bool valid;
    bool usesDataBit;
    int  nConn;
    int  cx[2], cy[2], cz[2];

    Rail() : w(0), x(0), y(0), z(0), valid(false), usesDataBit(false), nConn(0) {}
};

void updateConnections(Rail& r, int dir) {
    r.nConn = 0;
    if (!r.valid) return;
    struct P { int x, y, z; };
    P a, b;
    switch (dir) {
        case RailShape::DIR_FLAT_Z: a = { r.x, r.y, r.z - 1 };     b = { r.x, r.y, r.z + 1 };     break;
        case RailShape::DIR_FLAT_X: a = { r.x - 1, r.y, r.z };     b = { r.x + 1, r.y, r.z };     break;
        case 2: a = { r.x - 1, r.y, r.z };     b = { r.x + 1, r.y + 1, r.z }; break;
        case 3: a = { r.x - 1, r.y + 1, r.z }; b = { r.x + 1, r.y, r.z };     break;
        case 4: a = { r.x, r.y + 1, r.z - 1 }; b = { r.x, r.y, r.z + 1 };     break;
        case 5: a = { r.x, r.y, r.z - 1 };     b = { r.x, r.y + 1, r.z + 1 }; break;
        case 6: a = { r.x + 1, r.y, r.z };     b = { r.x, r.y, r.z + 1 };     break;
        case 7: a = { r.x - 1, r.y, r.z };     b = { r.x, r.y, r.z + 1 };     break;
        case 8: a = { r.x - 1, r.y, r.z };     b = { r.x, r.y, r.z - 1 };     break;
        case 9: a = { r.x + 1, r.y, r.z };     b = { r.x, r.y, r.z - 1 };     break;
        default: return;
    }
    r.cx[0] = a.x; r.cy[0] = a.y; r.cz[0] = a.z;
    r.cx[1] = b.x; r.cy[1] = b.y; r.cz[1] = b.z;
    r.nConn = 2;
}

Rail makeRail(World* w, int x, int y, int z) {
    Rail r;
    r.w = w; r.x = x; r.y = y; r.z = z;
    unsigned char id = worldBlock(w, x, y, z);
    r.valid = isRail(id);
    if (!r.valid) return r;
    int dir = worldData(w, x, y, z);
    r.usesDataBit = railUsesDataBit(id);
    if (r.usesDataBit) dir &= ~RailShape::DATA_BIT;
    updateConnections(r, dir);
    return r;
}

Rail getRail(World* w, int x, int y, int z) {
    if (isRail(worldBlock(w, x, y, z)))     return makeRail(w, x, y, z);
    if (isRail(worldBlock(w, x, y + 1, z))) return makeRail(w, x, y + 1, z);
    if (isRail(worldBlock(w, x, y - 1, z))) return makeRail(w, x, y - 1, z);
    Rail none; return none;
}

bool hasConnection(const Rail& r, int x, int z) {
    if (!r.valid) return false;
    for (int i = 0; i < r.nConn; i++)
        if (r.cx[i] == x && r.cz[i] == z) return true;
    return false;
}
bool connectsTo(const Rail& r, const Rail& other) {
    return hasConnection(r, other.x, other.z);
}

void removeSoftConnections(Rail& r) {
    if (!r.valid) return;
    int keep = 0;
    for (int i = 0; i < r.nConn; i++) {
        Rail nb = getRail(r.w, r.cx[i], r.cy[i], r.cz[i]);
        if (!nb.valid || !connectsTo(nb, r)) continue;
        r.cx[keep] = nb.x; r.cy[keep] = nb.y; r.cz[keep] = nb.z;
        keep++;
    }
    r.nConn = keep;
}

bool canConnectTo(const Rail& r, const Rail& other) {
    if (!r.valid) return false;
    if (connectsTo(r, other)) return true;
    if (r.nConn == 2) return false;
    return true;
}

int dirFromNeighbours(bool n, bool s, bool w, bool e, bool curves) {
    int dir = -1;
    if (n || s) dir = RailShape::DIR_FLAT_Z;
    if (w || e) dir = RailShape::DIR_FLAT_X;
    if (curves) {
        if (s && e && !n && !w) dir = 6;
        if (s && w && !n && !e) dir = 7;
        if (n && w && !s && !e) dir = 8;
        if (n && e && !s && !w) dir = 9;
    }
    return dir;
}

int applySlope(World* w, int x, int y, int z, int dir) {
    if (dir == RailShape::DIR_FLAT_Z) {
        if (isRail(worldBlock(w, x, y + 1, z - 1))) dir = 4;
        if (isRail(worldBlock(w, x, y + 1, z + 1))) dir = 5;
    } else if (dir == RailShape::DIR_FLAT_X) {
        if (isRail(worldBlock(w, x + 1, y + 1, z))) dir = 2;
        if (isRail(worldBlock(w, x - 1, y + 1, z))) dir = 3;
    }
    return dir;
}

void writeDir(World* w, int x, int y, int z, int dir) {
    unsigned char id = worldBlock(w, x, y, z);
    int data = dir;
    if (railUsesDataBit(id)) data = (worldData(w, x, y, z) & RailShape::DATA_BIT) | dir;

    worldSetData(w, x, y, z, (unsigned char)data);
}

void connectTo(Rail& r, const Rail& other) {
    if (!r.valid) return;

    if (r.nConn < 2) {
        r.cx[r.nConn] = other.x; r.cy[r.nConn] = other.y; r.cz[r.nConn] = other.z;
        r.nConn++;
    }

    bool n = hasConnection(r, r.x, r.z - 1);
    bool s = hasConnection(r, r.x, r.z + 1);
    bool wst = hasConnection(r, r.x - 1, r.z);
    bool e = hasConnection(r, r.x + 1, r.z);

    int dir = dirFromNeighbours(n, s, wst, e, !r.usesDataBit);
    dir = applySlope(r.w, r.x, r.y, r.z, dir);
    if (dir < 0) dir = RailShape::DIR_FLAT_Z;
    writeDir(r.w, r.x, r.y, r.z, dir);
}

bool hasNeighborRail(const Rail& r, int x, int y, int z) {
    if (!r.valid) return false;
    Rail nb = getRail(r.w, x, y, z);
    if (!nb.valid) return false;
    removeSoftConnections(nb);
    return canConnectTo(nb, r);
}

void place(Rail& r, bool hasSignal, bool first) {
    if (!r.valid) return;

    bool n = hasNeighborRail(r, r.x, r.y, r.z - 1);
    bool s = hasNeighborRail(r, r.x, r.y, r.z + 1);
    bool wst = hasNeighborRail(r, r.x - 1, r.y, r.z);
    bool e = hasNeighborRail(r, r.x + 1, r.y, r.z);

    const bool curves = !r.usesDataBit;
    int dir = -1;

    if ((n || s) && !wst && !e) dir = RailShape::DIR_FLAT_Z;
    if ((wst || e) && !n && !s) dir = RailShape::DIR_FLAT_X;
    if (curves) {
        if (s && e && !n && !wst) dir = 6;
        if (s && wst && !n && !e) dir = 7;
        if (n && wst && !s && !e) dir = 8;
        if (n && e && !s && !wst) dir = 9;
    }
    if (dir == -1) {

        dir = dirFromNeighbours(n, s, wst, e, false);
        if (curves) {
            if (hasSignal) {
                if (s && e) dir = 6;
                if (wst && s) dir = 7;
                if (e && n) dir = 9;
                if (n && wst) dir = 8;
            } else {
                if (n && wst) dir = 8;
                if (e && n) dir = 9;
                if (wst && s) dir = 7;
                if (s && e) dir = 6;
            }
        }
    }
    dir = applySlope(r.w, r.x, r.y, r.z, dir);
    if (dir < 0) dir = RailShape::DIR_FLAT_Z;

    updateConnections(r, dir);

    unsigned char id = worldBlock(r.w, r.x, r.y, r.z);
    int data = dir;
    if (railUsesDataBit(id)) data = (worldData(r.w, r.x, r.y, r.z) & RailShape::DATA_BIT) | dir;

    if (!first && worldData(r.w, r.x, r.y, r.z) == (unsigned char)data) return;

    writeDir(r.w, r.x, r.y, r.z, dir);

    for (int i = 0; i < r.nConn; i++) {
        Rail nb = getRail(r.w, r.cx[i], r.cy[i], r.cz[i]);
        if (!nb.valid) continue;
        removeSoftConnections(nb);
        if (canConnectTo(nb, r)) connectTo(nb, r);
    }
}

}

void railUpdateDir(World* w, int x, int y, int z, bool first) {
    Rail r = makeRail(w, x, y, z);

    place(r, false, first);
}

bool railSupportOk(World* w, int x, int y, int z) {
    unsigned char id = worldBlock(w, x, y, z);
    const Tile* t = Tile::tiles[id];
    if (!t) return false;
    if (t->material->isSolid() && t->cube) return true;
    if (isStairs(id)) return (worldData(w, x, y, z) & STAIR_UPSIDEDOWN_BIT) != 0;
    if (isSlab(id))   return (worldData(w, x, y, z) & SLAB_TOP_SLOT_BIT) != 0;
    return false;
}
