#include "world/entity/ai/goals/tempt_goal.h"
#include "world/entity/ai/look_control.h"
#include "world/entity/path_finder_mob.h"
#include "world/entity/local_player.h"
#include "world/inventory/inventory.h"
#include "world/item/item_instance.h"
#include "world/level/level.h"
#include "world/level/pathfinder/path_navigation.h"

TemptGoal::TemptGoal(PathfinderMob* mob, float speed, const int* items, int count)
:   mob(mob), speedMultiplier(speed), items(items), itemCount(count),
    playerId(0), timeout(0), offTicks(0), repathDelay(0), running(false),
    prevAvoidWater(false) {
    setRequiredControlFlags(CONTROL_MOVE | CONTROL_LOOK);
}

Entity* TemptGoal::player() const {
    return playerId ? mob->level->getEntity(playerId) : 0;
}

bool TemptGoal::canUse() {

    if (itemCount == 0) return false;
    if (timeout > 0) { timeout--; return false; }

    LocalPlayer* p = mob->level->getNearestPlayer(mob, 10.0f);
    playerId = p ? p->entityId : 0;
    if (!p) return false;

    ItemInstance* held = p->inventory->getSelected();
    if (!held) return false;
    for (int i = 0; i < itemCount; i++)
        if (held->id == items[i]) return true;
    return false;
}

bool TemptGoal::canContinueToUse() {
    if (canUse()) { offTicks = 0; return true; }
    return ++offTicks <= OFF_GRACE;
}

void TemptGoal::start() {
    running = true;
    offTicks = 0;
    prevAvoidWater = mob->getNavigation()->getAvoidWater();
    mob->getNavigation()->setAvoidWater(false);
}

void TemptGoal::stop() {
    playerId = 0;
    offTicks = 0;
    mob->getNavigation()->stop();
    mob->getNavigation()->setAvoidWater(prevAvoidWater);
    timeout = 100;
    running = false;
}

void TemptGoal::tick() {
    Entity* p = player();
    if (!p) return;
    mob->getLookControl()->setLookAt(p, 30.0f, mob->getMaxHeadXRot());

    if (mob->distanceToSqr(p) < 6.25f) {
        mob->getNavigation()->stop();
        return;
    }
    if (--repathDelay <= 0) {
        mob->getNavigation()->moveTo((Mob*)p, speedMultiplier);

        repathDelay = 10;
    }
}
