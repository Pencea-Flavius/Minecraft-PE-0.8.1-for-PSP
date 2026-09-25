#include "world/entity/throwable.h"
#include "world/entity/entity_types.h"
#include "world/entity/entity_renderer_id.h"
#include "world/entity/animal/chicken.h"
#include "world/entity/mob.h"
#include "world/entity/local_player.h"
#include "client/player/player_state.h"
#include "world/item/item.h"
#include "world/level/level.h"
#include "world/level/world.h"
#include "world/level/chunk/chunk.h"
#include "client/renderer/particle.h"
#include "util/mth.h"
#include <cmath>
#include "client/gui/hud.h"

static const float RAD = 180.0f / Mth::PI;
static const float DEG = Mth::PI / 180.0f;

void Throwable::configure(int t) {
    type = t;
    setSize(0.25f, 0.25f);
    if (t == EntityTypes::IdThrownEgg) {
        itemId = ITEM_EGG;      entityRendererId = ER_THROWNEGG_RENDERER;
    } else {
        itemId = ITEM_SNOWBALL; entityRendererId = ER_SNOWBALL_RENDERER;
    }
}

Throwable::Throwable(Level* level, int t) : super(level), ownerId(0), life(0) {
    configure(t);
}

Throwable::Throwable(Level* level, Mob* owner, int t) : super(level), ownerId(0), life(0) {
    configure(t);
    ownerId = owner->entityId;

    float yaw = owner->yRot;
    float px = owner->x - cosf(yaw * DEG) * 0.16f;
    float py = owner->y + owner->getHeadHeight() - 0.1f;
    float pz = owner->z - sinf(yaw * DEG) * 0.16f;
    setPos(px, py, pz);
    xOld = x; yOld = y; zOld = z;
    setRot(yaw, owner->xRot);
}

Throwable::Throwable(Level* level, float px, float py, float pz,
                     float yaw, float pitch, int t) : super(level), ownerId(0), life(0) {
    configure(t);

    float cy = cosf(yaw * DEG),   sy = sinf(yaw * DEG);
    float cp = cosf(pitch * DEG), sp = sinf(pitch * DEG);

    px -= cy * 0.16f;
    py -= 0.1f;
    pz -= sy * 0.16f;
    setPos(px, py, pz);
    xOld = x; yOld = y; zOld = z;

    shoot(-cp * sy, sp, cp * cy, 1.5f);
}

void Throwable::shoot(float dx, float dy, float dz, float power, float inaccuracy) {
    float dist = Mth::sqrt(dx * dx + dy * dy + dz * dz);
    if (dist >= 0.001f) { dx /= dist; dy /= dist; dz /= dist; }
    else { dx = dy = dz = 0.0f; }

    dx += sharedRandom.nextGaussian() * 0.0075f * inaccuracy;
    dy += sharedRandom.nextGaussian() * 0.0075f * inaccuracy;
    dz += sharedRandom.nextGaussian() * 0.0075f * inaccuracy;

    xd = dx * power; yd = dy * power; zd = dz * power;
    float sd = Mth::sqrt(xd * xd + zd * zd);
    yRot = yRotO = atan2f(xd, zd) * RAD;
    xRot = xRotO = atan2f(yd, sd) * RAD;
}

void Throwable::onHit() {
    if (type == EntityTypes::IdThrownEgg && sharedRandom.nextInt(8) == 0) {
        int count = sharedRandom.nextInt(32) == 0 ? 4 : 1;
        for (int i = 0; i < count; i++) {
            Chicken* c = new Chicken(level);
            c->setAge(-24000);
            c->moveTo(x, y, z, yRot, 0.0f);
            level->addEntity(c);
        }
    }
    particlesThrowPoof(x, y, z, itemFlatIcon(itemId, 0));
    remove();
}

void Throwable::tick() {
    xOld = x; yOld = y; zOld = z;
    baseTick();

    float nx = x + xd, ny = y + yd, nz = z + zd;
    BlockHit tileHit = worldClip(level->w, x, y, z, nx, ny, nz, false, true);
    if (tileHit.hit) {
        nx = tileHit.x + tileHit.clickX;
        ny = tileHit.y + tileHit.clickY;
        nz = tileHit.z + tileHit.clickZ;
    }
    float sxd = nx - x, syd = ny - y, szd = nz - z;
    float dist = Mth::sqrt(sxd * sxd + syd * syd + szd * szd);

    static EntityList candidates;
    level->getEntities(this, bb.expand(xd, yd, zd).grow(0.3f, 0.3f, 0.3f), candidates);
    if (dist > 1e-6f) {
        float ux = sxd / dist, uy = syd / dist, uz = szd / dist;
        Entity* hitEntity = 0; float nearest = 0.0f, t;
        for (size_t ei = 0; ei < candidates.size(); ei++) {
            Entity* e = candidates[ei];
            if (e->removed || e == (Entity*)level->player || !e->isPickable()) continue;

            if (e->entityId == ownerId && life < 5) continue;
            if (e->bb.grow(0.3f, 0.3f, 0.3f).clip(x, y, z, ux, uy, uz, dist, t) &&
                (!hitEntity || t < nearest)) { hitEntity = e; nearest = t; }
        }

        bool hitPlayer = false;
        if (ownerId != 0 && level->player && level->player->isAlive()) {
            LocalPlayer* p = level->player;
            float pf = p->y - PLAYER_EYE;
            AABB pbb(p->x - PLAYER_W * 0.5f, pf, p->z - PLAYER_W * 0.5f,
                     p->x + PLAYER_W * 0.5f, pf + PLAYER_H, p->z + PLAYER_W * 0.5f);
            if (pbb.grow(0.3f, 0.3f, 0.3f).clip(x, y, z, ux, uy, uz, dist, t) &&
                (!hitEntity || t < nearest)) { hitEntity = (Entity*)p; hitPlayer = true; }
        }
        if (hitEntity) {

            if (hitPlayer) level->player->hurt(this, 0);
            else           hitEntity->hurt(this, 0);
            onHit();
            return;
        }
    }
    if (tileHit.hit) { onHit(); return; }

    x += xd; y += yd; z += zd;
    float sd = Mth::sqrt(xd * xd + zd * zd);
    yRot = atan2f(xd, zd) * RAD;
    xRot = atan2f(yd, sd) * RAD;

    float inertia = 0.99f;
    if (isInWater()) {
        inertia = 0.80f;
        for (int i = 0; i < 4; i++) {
            float s = 0.25f;
            particlesBubble(x - xd * s, y - yd * s, z - zd * s, xd, yd, zd);
        }
    }
    xd *= inertia; yd *= inertia; zd *= inertia;
    yd -= 0.03f;
    setPos(x, y, z);

    if (++life >= 60 * TicksPerSecond) remove();
}
