#include "world/entity/ai/goals/door_interact_goal.h"
#include "world/entity/mob.h"
#include "world/level/level.h"
#include "world/level/chunk/chunk.h"
#include "world/level/pathfinder/path.h"
#include "world/level/pathfinder/node.h"
#include "world/level/pathfinder/path_navigation.h"
#include "util/mth.h"

DoorInteractGoal::DoorInteractGoal(Mob* mob)
:   mob(mob), doorX(0), doorY(0), doorZ(0), haveDoor(false), pastDoor(false),
    toDoorX(0), toDoorZ(0) {}

bool DoorInteractGoal::isDoorAt(int x, int y, int z) const {
    return mob->level->getTile(x, y, z) == BLOCK_DOOR_WOOD;
}

bool DoorInteractGoal::isDoorOpen() const {
    unsigned char d = (unsigned char)mob->level->getData(doorX, doorY, doorZ);
    int lowerY = (d & 8) ? doorY - 1 : doorY;
    return (mob->level->getData(doorX, lowerY, doorZ) & 4) != 0;
}

bool DoorInteractGoal::canUse() {
    if (!mob->horizontalCollision) return false;
    if (mob->path.isEmpty() || mob->path.isDone()) return false;
    if (!mob->getNavigation()->canOpenDoors()) return false;

    int last = mob->path.getIndex() + 2;
    if (last > mob->path.getSize()) last = mob->path.getSize();
    for (int i = mob->path.getIndex(); i < last; i++) {
        Node* n = mob->path.get(i);
        if (mob->distanceToSqr((float)n->x, mob->y, (float)n->z) > 2.25f) continue;
        if (isDoorAt(n->x, n->y + 1, n->z)) {
            doorX = n->x; doorY = n->y + 1; doorZ = n->z;
            haveDoor = true;
            return true;
        }
    }

    int bx = Mth::floor(mob->x), by = Mth::floor(mob->y + 1.0f), bz = Mth::floor(mob->z);
    if (isDoorAt(bx, by, bz)) {
        doorX = bx; doorY = by; doorZ = bz;
        haveDoor = true;
        return true;
    }
    haveDoor = false;
    return false;
}

bool DoorInteractGoal::canContinueToUse() { return !pastDoor; }

void DoorInteractGoal::start() {
    pastDoor = false;
    toDoorX = ((float)doorX + 0.5f) - mob->x;
    toDoorZ = ((float)doorZ + 0.5f) - mob->z;
}

void DoorInteractGoal::tick() {

    float nowX = ((float)doorX + 0.5f) - mob->x;
    float nowZ = ((float)doorZ + 0.5f) - mob->z;
    if (nowX * toDoorX + nowZ * toDoorZ < 0.0f) pastDoor = true;
}
