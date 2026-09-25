#include "world/item/compass_item.h"
#include "world/item/dial_math.h"
#include "world/entity/player.h"
#include "world/level/level.h"

int CompassItem::getAnimationFrameFor(const Player* p) const {
    if (!p) return 0;
    return Dial::compassFrame((float)g_level.spawnX, (float)g_level.spawnZ,
                              p->x, p->z, p->yRot);
}
