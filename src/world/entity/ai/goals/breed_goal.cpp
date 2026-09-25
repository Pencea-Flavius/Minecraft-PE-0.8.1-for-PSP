#include "world/entity/ai/goals/breed_goal.h"
#include "world/entity/ai/look_control.h"
#include "world/entity/animal/animal.h"
#include "world/entity/mob_category.h"
#include "world/level/level.h"
#include "world/level/pathfinder/path_navigation.h"
#include "client/renderer/particle.h"
#include "client/gui/hud.h"

BreedGoal::BreedGoal(Animal* animal, float speed)
:   animal(animal), speedMultiplier(speed), partnerId(0), inUseTicks(0), repathDelay(0) {
    setRequiredControlFlags(CONTROL_MOVE | CONTROL_LOOK);
}

Animal* BreedGoal::partner() const {
    if (!partnerId) return 0;
    Entity* e = animal->level->getEntity(partnerId);
    return (e && e->isAlive()) ? (Animal*)e : 0;
}

Animal* BreedGoal::findFreePartner() {
    static EntityList nearby;
    AABB box = animal->bb.grow(8.0f, 8.0f, 8.0f);
    animal->level->getEntitiesOfType(animal->getEntityTypeId(), box, nearby);
    for (unsigned int i = 0; i < nearby.size(); i++) {
        Animal* other = (Animal*)nearby[i];
        if (animal->canMate(other)) return other;
    }
    return 0;
}

bool BreedGoal::canUse() {
    if (!animal->isInLove()) return false;
    Animal* p = findFreePartner();
    partnerId = p ? p->entityId : 0;
    return partnerId != 0;
}

bool BreedGoal::canContinueToUse() {
    Animal* p = partner();
    if (!p || !p->isInLove()) return false;
    return inUseTicks <= 59;
}

void BreedGoal::stop() { partnerId = 0; inUseTicks = 0; repathDelay = 0; }

void BreedGoal::tick() {
    Animal* p = partner();
    if (!p) return;
    animal->getLookControl()->setLookAt(p, 10.0f, animal->getMaxHeadXRot());

    if (--repathDelay <= 0) {
        repathDelay = 10;
        animal->getNavigation()->moveTo(p, speedMultiplier);
    }
    if (++inUseTicks == 60) breed();
}

void BreedGoal::breed() {
    Animal* p = partner();
    if (!p) return;

    int why = MobCap::OK;
    if (!animal->level->canCreateMore(animal->getEntityTypeId(), Level::SPAWN_BREED, &why)) {
        animal->resetLove(); p->resetLove();

        if (why == MobCap::TOO_MANY_CHICKENS)
            hudChatMessage("These Chickens can't enter Love Mode. The maximum number "
                           "of Chickens in a world has been reached.");
        else if (why == MobCap::TOO_MANY_ANIMALS)
            hudChatMessage("These animals can't enter Love Mode. The maximum number "
                           "of Pigs, Sheep and Cows has been reached.");
        return;
    }

    Animal* baby = animal->getBreedOffspring(p);
    if (!baby) { animal->resetLove(); p->resetLove(); return; }

    animal->setAge(6000);
    p->setAge(6000);
    animal->resetLove();
    p->resetLove();
    baby->setAge(-24000);
    baby->moveTo(animal->x, animal->y, animal->z, 0.0f, 0.0f);
    baby->setDespawnProtected();
    animal->level->addEntity(baby);

    particlesHeartBurst(animal->x, animal->y - animal->heightOffset, animal->z,
                        animal->bbWidth, animal->bbHeight);
}
