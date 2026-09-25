
#include "world/entity/arrow.h"
#include "world/level/level.h"
#include "world/entity/local_player.h"
#include "world/entity/entity_types.h"
#include "world/level/level.h"
#include "world/level/world.h"
#include "world/level/chunk/chunk.h"
#include "nbt/compound_tag.h"
#include "world/inventory/inventory.h"
#include "world/item/item_instance.h"
#include "world/item/item.h"
#include "util/mth.h"
#include "client/player/player_state.h"
#include "client/renderer/particle.h"
#include <cmath>
#include <cstdlib>

static const float RAD = 180.0f / Mth::PI;
static const float DEG = Mth::PI / 180.0f;

static float arrowPitch() {
    return 1.2f / ((rand() / (float)RAND_MAX) * 0.2f + 0.9f);
}

static unsigned char lodgedData(World* w, int x, int y, int z) {
    unsigned char d = worldData(w, x, y, z);
    if (isDoor(worldBlock(w, x, y, z)) && (d & 8)) return worldData(w, x, y - 1, z);
    return d;
}

static const float ARROW_BASE_DAMAGE = 2.0f;
int Arrow::damageForSpeed() {
    float pow = Mth::sqrt(xd * xd + yd * yd + zd * zd);
    int dmg = (int)ceilf(pow * ARROW_BASE_DAMAGE);

    if (critArrow) dmg += sharedRandom.nextInt(dmg / 2 + 2);
    return dmg;
}

Arrow::Arrow(Level* level)
    : super(level), ownerId(0), inGround(false), life(0), shakeTime(0), critArrow(false),
      playerArrow(false), xTile(-1), yTile(-1), zTile(-1), lastTile(0), lastData(0), flightTime(0) {
    setSize(0.25f, 0.25f);
    entityRendererId = ER_ARROW_RENDERER;
}

Arrow::Arrow(Level* level, float px, float py, float pz,
             float yaw, float pitch, float speed, bool crit, bool fromPlayer,
             float uncertainty)
    : super(level), ownerId(0), inGround(false), life(0), shakeTime(0), critArrow(crit),
      playerArrow(fromPlayer), xTile(-1), yTile(-1), zTile(-1), lastTile(0), lastData(0), flightTime(0) {
    setSize(0.25f, 0.25f);
    entityRendererId = ER_ARROW_RENDERER;

    float cy = cosf(yaw * DEG),   sy = sinf(yaw * DEG);
    float cp = cosf(pitch * DEG), sp = sinf(pitch * DEG);
    float dx = -cp * sy, dy = sp, dz = cp * cy;

    setPos(px - cy * 0.16f, py - 0.1f, pz - sy * 0.16f);
    xOld = x; yOld = y; zOld = z;

    speed *= 1.5f;

    const float radius = 0.0075f * uncertainty;
    dx += sharedRandom.nextGaussian() * radius;
    dy += sharedRandom.nextGaussian() * radius;
    dz += sharedRandom.nextGaussian() * radius;

    xd = dx * speed; yd = dy * speed; zd = dz * speed;
    yRot = yRotO = atan2f(xd, zd) * RAD;
    xRot = xRotO = atan2f(yd, Mth::sqrt(xd * xd + zd * zd)) * RAD;
}

void Arrow::tick() {

    xOld = x; yOld = y; zOld = z;

    baseTick();

    if (shakeTime > 0) shakeTime--;

    if (inGround) {

        if (shakeTime <= 0 && playerArrow) {
            float feet = g_level.player->y - PLAYER_EYE;
            float r    = PLAYER_W * 0.5f + 1.0f;
            if (x >= g_level.player->x - r && x <= g_level.player->x + r &&
                z >= g_level.player->z - r && z <= g_level.player->z + r &&
                y >= feet     && y <= feet + PLAYER_H) {

                if (!g_level.player->inventory->isCreative()) {
                    ItemInstance stack(ITEM_ARROW, 1, 0);
                    if (!g_level.player->inventory->add(stack)) return;
                    g_level.player->inventory->ensureHotbar(ITEM_ARROW, 0);
                }
                level->playSound(this, "random.pop", 0.2f,
                                 (((rand() / (float)RAND_MAX) - (rand() / (float)RAND_MAX)) * 0.7f + 1.0f) * 2.0f);
                remove();
                return;
            }
        }

        if (worldBlock(level->w, xTile, yTile, zTile) != lastTile ||
            lodgedData(level->w, xTile, yTile, zTile) != lastData) {
            inGround = false;

            xd *= sharedRandom.nextFloat() * 0.2f;
            yd *= sharedRandom.nextFloat() * 0.2f;
            zd *= sharedRandom.nextFloat() * 0.2f;
            life = 0;
            flightTime = 0;
            return;
        }
        if (++life >= 60 * TicksPerSecond) remove();
        return;
    }
    flightTime++;

    float nx = x + xd, ny = y + yd, nz = z + zd;
    BlockHit tileHit = worldClip(level->w, x, y, z, nx, ny, nz, false, true);
    bool hit = tileHit.hit;
    if (hit) {
        nx = tileHit.x + tileHit.clickX;
        ny = tileHit.y + tileHit.clickY;
        nz = tileHit.z + tileHit.clickZ;
    }
    float sxd = nx - x, syd = ny - y, szd = nz - z;
    float dist = Mth::sqrt(sxd * sxd + syd * syd + szd * szd);

    static EntityList candidates;
    level->getEntities(this, bb.expand(xd, yd, zd).grow(0.3f, 0.3f, 0.3f), candidates);
    Entity* hitEntity = 0;
    bool    hitPlayer = false;
    float   nearest = 0.0f;
    if (dist > 1e-6f) {
        float ux = sxd / dist, uy = syd / dist, uz = szd / dist;
        float t;
        for (size_t ei = 0; ei < candidates.size(); ei++) {
            Entity* e = candidates[ei];
            if (e->removed || !e->isPickable()) continue;

            if (e->entityId == ownerId && flightTime < 5) continue;
            if (e->bb.grow(0.3f, 0.3f, 0.3f).clip(x, y, z, ux, uy, uz, dist, t) &&
                (!hitEntity || t < nearest)) { hitEntity = e; nearest = t; }
        }

        if (ownerId != 0 && level->player && level->player->isAlive()) {
            LocalPlayer* p = level->player;
            float pf = p->y - PLAYER_EYE;
            AABB pbb(p->x - PLAYER_W * 0.5f, pf, p->z - PLAYER_W * 0.5f,
                     p->x + PLAYER_W * 0.5f, pf + PLAYER_H, p->z + PLAYER_W * 0.5f);
            if (pbb.grow(0.3f, 0.3f, 0.3f).clip(x, y, z, ux, uy, uz, dist, t) &&
                (!hitEntity || t < nearest)) { hitEntity = (Entity*)p; hitPlayer = true; }
        }
    }
    if (hitEntity) {

        bool took = hitPlayer ? level->player->hurt(this, damageForSpeed())
                              : hitEntity->hurt(this, damageForSpeed());
        if (took) {
            level->playSound(this, "random.bowhit", 1.0f, arrowPitch());
            remove();
        } else {

            xd *= -0.1f; yd *= -0.1f; zd *= -0.1f;
            yRot += 180.0f; yRotO += 180.0f;
            flightTime = 0;
        }
        return;
    }

    if (hit) {

        xTile = tileHit.x; yTile = tileHit.y; zTile = tileHit.z;
        float px = nx, py = ny, pz = nz;
        if (dist > 1e-6f) {
            px -= sxd / dist * 0.05f;
            py -= syd / dist * 0.05f;
            pz -= szd / dist * 0.05f;
        }
        lastTile = worldBlock(level->w, xTile, yTile, zTile);
        lastData = lodgedData(level->w, xTile, yTile, zTile);
        x = px; y = py; z = pz;
        xd = yd = zd = 0.0f;
        inGround = true;
        shakeTime = 7;
        critArrow = false;
        level->playSound(this, "random.bowhit", 1.0f, arrowPitch());
        setPos(x, y, z);
        return;
    }

    if (critArrow) {
        for (int i = 0; i < 4; i++)
            particlesCrit(x + xd * i / 4.0f, y + yd * i / 4.0f, z + zd * i / 4.0f,
                          -xd, -yd + 0.2f, -zd);
    }

    x = nx; y = ny; z = nz;
    float sd = Mth::sqrt(xd * xd + zd * zd);
    yRot = atan2f(xd, zd) * RAD;
    xRot = atan2f(yd, sd) * RAD;
    while (xRot - xRotO < -180.0f) xRotO -= 360.0f;
    while (xRot - xRotO >= 180.0f) xRotO += 360.0f;
    while (yRot - yRotO < -180.0f) yRotO -= 360.0f;
    while (yRot - yRotO >= 180.0f) yRotO += 360.0f;
    xRot = xRotO + (xRot - xRotO) * 0.2f;
    yRot = yRotO + (yRot - yRotO) * 0.2f;

    float inertia = 0.99f;
    if (isInWater()) {
        inertia = 0.80f;
        for (int i = 0; i < 4; i++) {
            float s = 0.25f;
            particlesBubble(x - xd * s, y - yd * s, z - zd * s, xd, yd, zd);
        }
    }
    xd *= inertia; yd *= inertia; zd *= inertia;
    yd -= 0.05f;

    setPos(x, y, z);
}

int Arrow::getEntityTypeId() const { return EntityTypes::IdArrow; }

void Arrow::addAdditonalSaveData(CompoundTag* tag) {
    tag->putShort("xTile", (short)xTile);
    tag->putShort("yTile", (short)yTile);
    tag->putShort("zTile", (short)zTile);

    tag->putByte("inTile", (char)lastTile);
    tag->putByte("inData", (char)lastData);
    tag->putByte("shake", (char)shakeTime);
    tag->putByte("inGround", (char)(inGround ? 1 : 0));
    tag->putBoolean("player", playerArrow);
}

void Arrow::readAdditionalSaveData(CompoundTag* tag) {
    xTile = tag->getShort("xTile");
    yTile = tag->getShort("yTile");
    zTile = tag->getShort("zTile");
    lastTile = tag->getByte("inTile") & 0xff;
    lastData = tag->getByte("inData") & 0xff;
    shakeTime = tag->getByte("shake") & 0xff;
    inGround = tag->getByte("inGround") == 1;
    playerArrow = tag->getBoolean("player");
    life = 0;
}
