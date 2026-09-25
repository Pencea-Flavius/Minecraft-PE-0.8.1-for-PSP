#include "world/entity/ai/goals/follow_parent_goal.h"
#include "world/entity/animal/animal.h"
#include "world/level/level.h"
#include "world/level/pathfinder/path_navigation.h"

FollowParentGoal::FollowParentGoal(Animal* animal, float speed)
:   animal(animal), speedMultiplier(speed), parentId(0), ticksFollowing(0) {}

Animal* FollowParentGoal::parent() const {
    if (!parentId) return 0;
    Entity* e = animal->level->getEntity(parentId);
    return (e && e->isAlive()) ? (Animal*)e : 0;
}

bool FollowParentGoal::canUse() {
    if (animal->getAge() >= 0) return false;

    static EntityList nearby;

    AABB box(animal->bb.x0 - 8.0f, animal->bb.y0 - 4.0f, animal->bb.z0 - 8.0f,
             animal->bb.x1 + 8.0f, animal->bb.y1 + 4.0f, animal->bb.z1 + 8.0f);
    animal->level->getEntitiesOfType(animal->getEntityTypeId(), box, nearby);

    Animal* best = 0;
    float bestDist = 1e30f;
    for (unsigned int i = 0; i < nearby.size(); i++) {
        Animal* a = (Animal*)nearby[i];
        if (a->getAge() < 0) continue;
        float d = animal->distanceToSqr(a);
        if (d <= bestDist) { bestDist = d; best = a; }
    }
    if (!best) return false;
    if (bestDist < 9.0f) return false;
    parentId = best->entityId;
    return true;
}

bool FollowParentGoal::canContinueToUse() {
    Animal* p = parent();
    if (!p) return false;
    float d = animal->distanceToSqr(p);
    return d >= 9.0f && d <= 256.0f;
}

void FollowParentGoal::start() { ticksFollowing = 0; }
void FollowParentGoal::stop()  { parentId = 0; }

void FollowParentGoal::tick() {
    if (--ticksFollowing > 0) return;
    ticksFollowing = 10;
    Animal* p = parent();
    if (p) animal->getNavigation()->moveTo(p, speedMultiplier);
}
