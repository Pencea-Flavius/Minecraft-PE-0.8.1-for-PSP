#include "world/entity/ai/goals/eat_tile_goal.h"
#include "world/entity/mob.h"
#include "world/level/level.h"
#include "world/level/world.h"
#include "world/level/chunk/chunk.h"
#include "world/level/pathfinder/path_navigation.h"
#include "util/mth.h"

extern World g_world;

EatTileGoal::EatTileGoal(Mob* mob) : mob(mob), ticksLeft(0) {

    setRequiredControlFlags(CONTROL_MOVE | CONTROL_LOOK | CONTROL_JUMP);
}

bool EatTileGoal::canUse() {

    int chance = mob->isBaby() ? 50 : 1000;
    if (Entity::sharedRandom.nextInt(chance) != 0) return false;

    int bx = Mth::floor(mob->x), by = Mth::floor(mob->y), bz = Mth::floor(mob->z);

    if (mob->level->getTile(bx, by, bz) == BLOCK_TALLGRASS &&
        mob->level->getData(bx, by, bz) == TG_TALL_GRASS) return true;
    return mob->level->getTile(bx, by - 1, bz) == BLOCK_GRASS;
}

bool EatTileGoal::canContinueToUse() { return ticksLeft > 0; }

void EatTileGoal::start() {
    ticksLeft = EAT_TICKS;
    mob->getNavigation()->stop();
}

void EatTileGoal::stop() { ticksLeft = 0; }

void EatTileGoal::tick() {
    if (--ticksLeft < 0) ticksLeft = 0;
    if (ticksLeft != 4) return;

    int bx = Mth::floor(mob->x), by = Mth::floor(mob->y), bz = Mth::floor(mob->z);
    if (mob->level->getTile(bx, by, bz) == BLOCK_TALLGRASS &&
        mob->level->getData(bx, by, bz) == TG_TALL_GRASS) {
        worldSetBlockAndData(&g_world, bx, by, bz, 0, 0);
        worldNotifyNeighborsChanged(&g_world, bx, by, bz);
        worldRebuildAroundNow(&g_world, bx, by, bz);
        mob->ate();
        return;
    }
    if (mob->level->getTile(bx, by - 1, bz) == BLOCK_GRASS) {
        worldSetBlockAndData(&g_world, bx, by - 1, bz, BLOCK_DIRT, 0);
        worldNotifyNeighborsChanged(&g_world, bx, by - 1, bz);
        worldRebuildAroundNow(&g_world, bx, by - 1, bz);
        mob->ate();
    }
}
