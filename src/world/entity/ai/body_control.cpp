
#include "world/entity/ai/body_control.h"
#include "world/entity/mob.h"
#include "util/mth.h"

void BodyControl::clientTick() {
    float mdx = mob->x - mob->xo;
    float mdz = mob->z - mob->zo;

    if (mdx * mdx + mdz * mdz > 0.00000025f) {

        mob->yBodyRot = mob->yBodyRot +
            Mth::clamp(Mth::wrapDegrees(mob->yRot - mob->yBodyRot),
                       -(float)Mob::BODY_TURN_RATE, (float)Mob::BODY_TURN_RATE);
        float d = Mth::wrapDegrees(mob->yBodyRot - mob->yHeadRot);
        if (d < -75.0f) d = -75.0f;
        else if (d >= 75.0f) d = 75.0f;
        mob->yHeadRot = mob->yBodyRot - d;
        lastHeadRot = mob->yHeadRot;
        stillTicks = 0;
        return;
    }

    float allowance = 75.0f;
    if (Mth::abs(mob->yHeadRot - lastHeadRot) <= 15.0f) {
        if (++stillTicks > 10) {
            float t = 1.0f - (float)(stillTicks - 10) / 10.0f;
            if (t < 0.0f) t = 0.0f;
            allowance = t * 75.0f;
        }
    } else {
        stillTicks = 0;
        lastHeadRot = mob->yHeadRot;
    }

    float d = Mth::clamp(Mth::wrapDegrees(mob->yHeadRot - mob->yBodyRot),
                         -allowance, allowance);
    mob->yBodyRot = mob->yHeadRot - d;
    mob->yRot = mob->yBodyRot;
}
