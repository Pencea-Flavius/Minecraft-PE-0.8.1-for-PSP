
#include "world/entity/tripod_camera.h"
#include "world/entity/player.h"
#include "world/level/level.h"
#include "world/inventory/inventory.h"
#include "world/item/item.h"
#include "world/item/item_instance.h"
#include "nbt/compound_tag.h"
#include "util/mth.h"
#include <cmath>

void TripodCamera::init() {
    entityRendererId = ER_TRIPODCAMERA_RENDERER;
    blocksBuilding = true;
    setSize(1.0f, 1.5f);
    heightOffset = bbHeight / 2.0f - 0.25f;
}

TripodCamera::TripodCamera(Level* level, float x, float y, float z,
                           float ownerYaw, float ownerPitch)
    : Mob(level) {
    init();

    xRot = xRotO = ownerPitch;
    yRot = yRotO = ownerYaw;
    setPos(x, y, z);
    xo = xOld = x; yo = yOld = y; zo = zOld = z;
}

TripodCamera::TripodCamera(Level* level)
    : Mob(level) {
    init();
}

void TripodCamera::breakAndDrop() {
    if (removed) return;

    Player* p = (Player*)level->player;
    bool creative = p && p->inventory && p->inventory->isCreative();
    if (!creative) spawnAtLocation(ITEM_CAMERA, 1);
    remove();
}

bool TripodCamera::hurt(Entity* source, int ) {
    if (removed || !source) return false;
    level->playSound(this, getHurtSound(), 1.0f, 1.0f);
    breakAndDrop();
    return true;
}

void TripodCamera::tick() {

    xo = xOld = x; yo = yOld = y; zo = zOld = z;

    yd -= 0.04f;
    move(xd, yd, zd);
    xd *= 0.98f;
    yd *= 0.98f;
    zd *= 0.98f;
    if (onGround) {
        xd *= 0.7f;
        zd *= 0.7f;
        yd *= -0.5f;
    }
}
