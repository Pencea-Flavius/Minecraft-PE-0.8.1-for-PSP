
#include "world/entity/minecart.h"
#include "client/gamemode/gamemode.h"
#include "world/entity/entity_types.h"
#include "world/entity/local_player.h"
#include "world/level/level.h"
#include "world/level/world.h"
#include "world/level/tile/rail_tile.h"
#include "world/item/item.h"
#include "util/mth.h"
#include "nbt/compound_tag.h"
#include <cmath>

Minecart::Minecart(Level* level) : super(level) { init(); }

Minecart::Minecart(Level* level, float px, float py, float pz) : super(level) {
    init();

    setPos(px, py, pz);

    xo = xOld = x; yo = yOld = y; zo = zOld = z;
    xd = yd = zd = 0.0f;
}

void Minecart::init() {
    setSize(0.98f, 0.7f);
    heightOffset = bbHeight / 2.0f;
    entityRendererId = ER_MINECART_RENDERER;
    blocksBuilding = true;

    makeStepSound = false;
    damage = 0.0f; hurtTime = 0; hurtDir = 1; flipped = false;
}

int Minecart::getEntityTypeId() const { return EntityTypes::IdMinecart; }
EntityRendererId Minecart::queryEntityRenderer() { return ER_MINECART_RENDERER; }

bool Minecart::interact() {
    Entity* p = level ? (Entity*)level->player : 0;
    if (!p) return false;
    if (rider == p) { p->ride(0); return true; }
    if (rider) return false;
    p->ride(this);
    return true;
}

bool Minecart::hurt(Entity* source, int amount) {
    if (removed) return false;
    hurtDir = -hurtDir;
    hurtTime = 10;
    markHurt();

    if (g_gameMode && g_gameMode->isCreative()) damage = 100.0f;
    damage += amount * 10.0f;
    if (damage <= 40.0f) return true;
    if (rider) rider->ride(0);
    spawnAtLocation(ITEM_MINECART, 1);
    remove();
    return true;
}

void Minecart::push(Entity* e) {
    if (!e || e == rider || e->riding == this) return;
    if (e->getEntityTypeId() != EntityTypes::IdMinecart) { super::push(e); return; }

    float dx = e->x - x, dz = e->z - z;
    float d = dx * dx + dz * dz;
    if (d < 1.0e-4f) return;
    d = sqrtf(d);
    dx /= d; dz /= d;
    float inv = 1.0f / d;
    if (inv > 1.0f) inv = 1.0f;
    dx *= inv; dz *= inv;
    dx *= 0.1f; dz *= 0.1f;
    dx *= 1.0f - pushthrough;
    dz *= 1.0f - pushthrough;
    dx *= 0.5f; dz *= 0.5f;

    float sx = (e->xd - xd) * 0.5f, sz = (e->zd - zd) * 0.5f;
    xd += sx - dx; zd += sz - dz;
    e->xd -= sx - dx; e->zd -= sz - dz;
}

bool Minecart::railPos(float px, float py, float pz, float* out) {
    World* w = level->w;
    int xt = Mth::floor(px), yt = Mth::floor(py), zt = Mth::floor(pz);
    if (isRail(worldBlock(w, xt, yt - 1, zt))) yt--;

    unsigned char id = worldBlock(w, xt, yt, zt);
    if (!isRail(id)) return false;
    int data = railDir(id, worldData(w, xt, yt, zt));

    float py2 = (float)yt;
    if (data >= 2 && data <= 5) py2 = (float)(yt + 1);

    int ex[2][3];
    RailShape::exits(data, ex);

    float x0 = xt + 0.5f + ex[0][0] * 0.5f;
    float y0 = yt + 0.5f + ex[0][1] * 0.5f;
    float z0 = zt + 0.5f + ex[0][2] * 0.5f;
    float x1 = xt + 0.5f + ex[1][0] * 0.5f;
    float y1 = yt + 0.5f + ex[1][1] * 0.5f;
    float z1 = zt + 0.5f + ex[1][2] * 0.5f;

    float xD = x1 - x0, yD = (y1 - y0) * 2.0f, zD = z1 - z0;
    float progress;
    if (xD == 0.0f)      { px = xt + 0.5f; progress = pz - zt; }
    else if (zD == 0.0f) { pz = zt + 0.5f; progress = px - xt; }
    else                 { progress = ((px - x0) * xD + (pz - z0) * zD) * 2.0f; }

    (void)py2;
    out[0] = x0 + xD * progress;
    out[1] = y0 + yD * progress;
    out[2] = z0 + zD * progress;
    if (yD < 0.0f) out[1] += 1.0f;
    if (yD > 0.0f) out[1] += 0.5f;
    return true;
}

bool Minecart::railPosOffs(float px, float py, float pz, float offs, float* out) {
    World* w = level->w;
    int xt = Mth::floor(px), yt = Mth::floor(py), zt = Mth::floor(pz);
    if (isRail(worldBlock(w, xt, yt - 1, zt))) yt--;

    unsigned char id = worldBlock(w, xt, yt, zt);
    if (!isRail(id)) return false;
    int data = railDir(id, worldData(w, xt, yt, zt));

    float ny = (float)yt;
    if (data >= 2 && data <= 5) ny = (float)(yt + 1);

    int ex[2][3];
    RailShape::exits(data, ex);
    float xD = (float)(ex[1][0] - ex[0][0]);
    float zD = (float)(ex[1][2] - ex[0][2]);
    float dd = sqrtf(xD * xD + zD * zD);
    xD /= dd; zD /= dd;
    px += xD * offs;
    pz += zD * offs;

    if (ex[0][1] != 0 && Mth::floor(px) - xt == ex[0][0] && Mth::floor(pz) - zt == ex[0][2])
        ny += ex[0][1];
    else if (ex[1][1] != 0 && Mth::floor(px) - xt == ex[1][0] && Mth::floor(pz) - zt == ex[1][2])
        ny += ex[1][1];

    return railPos(px, ny, pz, out);
}

void Minecart::tick() {
    World* w = level->w;

    if (hurtTime > 0) hurtTime--;
    if (damage > 0.0f) damage -= 1.0f;
    if (y < -64.0f) outOfWorld();

    xOld = x; yOld = y; zOld = z;
    xo = x; yo = y; zo = z;
    yd -= 0.04f;

    int xt = Mth::floor(x), yt = Mth::floor(y), zt = Mth::floor(z);
    if (isRail(worldBlock(w, xt, yt - 1, zt))) yt--;

    const float MAXSPEED = 0.4f;
    const float SLIDE    = 1.0f / 128.0f;

    unsigned char tileId = worldBlock(w, xt, yt, zt);
    if (isRail(tileId)) {
        fallDistance = 0.0f;
        float oldPos[3];
        bool haveOld = railPos(x, y, z, oldPos);

        int data = railDir(tileId, worldData(w, xt, yt, zt));
        y = (float)yt;

        bool powerTrack = (tileId == BLOCK_GOLDEN_RAIL);

        if (data >= 2 && data <= 5) y = (float)(yt + 1);

        if (data == 2) xd -= SLIDE;
        if (data == 3) xd += SLIDE;
        if (data == 4) zd += SLIDE;
        if (data == 5) zd -= SLIDE;

        int ex[2][3];
        RailShape::exits(data, ex);

        float xD = (float)(ex[1][0] - ex[0][0]);
        float zD = (float)(ex[1][2] - ex[0][2]);
        float dd = sqrtf(xD * xD + zD * zD);

        if (xd * xD + zd * zD < 0.0f) { xD = -xD; zD = -zD; }
        float pow = sqrtf(xd * xd + zd * zd);
        if (pow > 2.0f) pow = 2.0f;
        xd = pow * xD / dd;
        zd = pow * zD / dd;

        if (rider && rider->isMob() && ((Mob*)rider)->yya > 0.0f) {
            const float a = rider->yRot * (3.14159265f / 180.0f);
            if (xd * xd + zd * zd < 0.01f) {
                xd -= sinf(a) * 0.1f;
                zd += cosf(a) * 0.1f;
            }
        }

        float progress = 0.0f;
        float x0 = xt + 0.5f + ex[0][0] * 0.5f;
        float z0 = zt + 0.5f + ex[0][2] * 0.5f;
        float x1 = xt + 0.5f + ex[1][0] * 0.5f;
        float z1 = zt + 0.5f + ex[1][2] * 0.5f;
        xD = x1 - x0; zD = z1 - z0;
        if (xD == 0.0f)      { x = xt + 0.5f; progress = z - zt; }
        else if (zD == 0.0f) { z = zt + 0.5f; progress = x - xt; }
        else                 { progress = ((x - x0) * xD + (z - z0) * zD) * 2.0f; }
        x = x0 + xD * progress;
        z = z0 + zD * progress;

        setPos(x, y + heightOffset + 0.00001f, z);

        float xdd = xd, zdd = zd;
        if (rider) { xdd *= 0.75f; zdd *= 0.75f; }
        xdd = Mth::clamp(xdd, -MAXSPEED, MAXSPEED);
        zdd = Mth::clamp(zdd, -MAXSPEED, MAXSPEED);
        move(xdd, 0.0f, zdd);

        if (ex[0][1] != 0 && Mth::floor(x) - xt == ex[0][0] && Mth::floor(z) - zt == ex[0][2])
            setPos(x, y + ex[0][1], z);
        else if (ex[1][1] != 0 && Mth::floor(x) - xt == ex[1][0] && Mth::floor(z) - zt == ex[1][2])
            setPos(x, y + ex[1][1], z);

        if (rider) { xd *= 0.997f; yd = 0.0f; zd *= 0.997f; }
        else       { xd *= 0.96f;  yd = 0.0f; zd *= 0.96f;  }

        float newPos[3];
        if (haveOld && railPos(x, y, z, newPos)) {
            float speed = (oldPos[1] - newPos[1]) * 0.05f;
            pow = sqrtf(xd * xd + zd * zd);
            if (pow > 0.0f) {
                xd = xd / pow * (pow + speed);
                zd = zd / pow * (pow + speed);
            }
            setPos(x, newPos[1], z);
        }

        int xn = Mth::floor(x), zn = Mth::floor(z);
        if (xn != xt || zn != zt) {
            pow = sqrtf(xd * xd + zd * zd);
            xd = pow * (float)(xn - xt);
            zd = pow * (float)(zn - zt);
        }

        if (powerTrack) {
            float sp = sqrtf(xd * xd + zd * zd);
            if (sp > 0.01f) {
                const float boost = 0.06f;
                xd += xd / sp * boost;
                zd += zd / sp * boost;
            } else {

                if (data == RailShape::DIR_FLAT_X) {
                    if (isSolidPhys(worldBlock(w, xt - 1, yt, zt)))      xd =  0.02f;
                    else if (isSolidPhys(worldBlock(w, xt + 1, yt, zt))) xd = -0.02f;
                } else if (data == RailShape::DIR_FLAT_Z) {
                    if (isSolidPhys(worldBlock(w, xt, yt, zt - 1)))      zd =  0.02f;
                    else if (isSolidPhys(worldBlock(w, xt, yt, zt + 1))) zd = -0.02f;
                }
            }
        }
    } else {

        xd = Mth::clamp(xd, -MAXSPEED, MAXSPEED);
        zd = Mth::clamp(zd, -MAXSPEED, MAXSPEED);
        if (onGround) { xd *= 0.5f; yd *= 0.5f; zd *= 0.5f; }
        move(xd, yd, zd);
        if (!onGround) { xd *= 0.95f; yd *= 0.95f; zd *= 0.95f; }
    }

    xRot = 0.0f;
    float dxo = xo - x, dzo = zo - z;
    if (dxo * dxo + dzo * dzo > 0.001f) {
        yRot = atan2f(dzo, dxo) * (180.0f / 3.14159265f);
        if (flipped) yRot += 180.0f;
    }
    float rotDiff = yRot - yRotO;
    while (rotDiff < -180.0f) rotDiff += 360.0f;
    while (rotDiff >= 180.0f) rotDiff -= 360.0f;
    if (rotDiff < -170.0f || rotDiff >= 170.0f) { yRot += 180.0f; flipped = !flipped; }
    setRot(yRot, xRot);

    static EntityList nearby;
    level->getEntities(this, bb.grow(0.2f, 0.0f, 0.2f), nearby);
    for (size_t i = 0; i < nearby.size(); i++) {
        Entity* e = nearby[i];
        if (e == rider || !e->isPushable()) continue;
        if (e->getEntityTypeId() != EntityTypes::IdMinecart) continue;
        ((Minecart*)e)->push(this);
        break;
    }

    if (rider && rider->removed) { rider->riding = 0; rider = 0; }

    positionRider(false);
}

void Minecart::addAdditonalSaveData(CompoundTag* tag) {
    tag->putInt("Type", 0);
}
void Minecart::readAdditionalSaveData(CompoundTag*) {}
