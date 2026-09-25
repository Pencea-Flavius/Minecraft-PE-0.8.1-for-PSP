#include "world/item/clock_item.h"
#include "world/item/dial_math.h"
#include "world/level/world.h"
#include "world/level/level.h"

int ClockItem::getAnimationFrameFor(const Player* ) const {
    return Dial::clockFrame(worldTimeOfDay(g_level.w ? g_level.w->dayTime : 0, 1.0f));
}
