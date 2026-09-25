
#include "world/entity/ai/look_control.h"
#include "world/entity/mob.h"
#include "world/level/pathfinder/path_navigation.h"
#include "util/mth.h"
#include <cmath>

LookControl::LookControl(Mob* mob)
:   mob(mob), yawSpeed(0), pitchSpeed(0), isLooking(false),
    lookX(0), lookY(0), lookZ(0) {}

void LookControl::setLookAt(Entity* target, float yawSpeed, float pitchSpeed) {
    lookX = target->x;

    lookY = target->isMob() ? (target->y + ((Mob*)target)->getHeadHeight())
                            : (target->bb.y0 + target->bb.y1) * 0.5f;
    lookZ = target->z;
    this->yawSpeed = yawSpeed;
    this->pitchSpeed = pitchSpeed;
    isLooking = true;
}

void LookControl::setLookAt(float x, float y, float z, float yawSpeed, float pitchSpeed) {
    lookX = x; lookY = y; lookZ = z;
    this->yawSpeed = yawSpeed;
    this->pitchSpeed = pitchSpeed;
    isLooking = true;
}

void LookControl::tick() {
    mob->xRot = 0.0f;

    float wantHead, speed;
    if (isLooking) {
        isLooking = false;
        float dx = lookX - mob->x;
        float dz = lookZ - mob->z;
        float dy = lookY - (mob->y + mob->getHeadHeight());
        float horiz = sqrtf(dx * dx + dz * dz);
        float wantPitch = atan2f(dy, horiz) * 180.0f / Mth::PI;

        mob->xRot = Mth::clampRotate(mob->xRot, wantPitch, pitchSpeed);
        wantHead  = atan2f(dz, dx) * 180.0f / Mth::PI - 90.0f;
        speed     = yawSpeed;
    } else {

        wantHead = mob->yBodyRot;
        speed    = 10.0f;
    }
    mob->yHeadRot = Mth::clampRotate(mob->yHeadRot, wantHead, speed);

    if (!mob->getNavigation()->isDone()) {
        float d = Mth::wrapDegrees(mob->yHeadRot - mob->yBodyRot);
        if (d < -75.0f) mob->yHeadRot = mob->yBodyRot - 75.0f;
        if (d >  75.0f) mob->yHeadRot = mob->yBodyRot + 75.0f;
    }
}
