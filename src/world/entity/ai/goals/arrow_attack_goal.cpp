#include "world/entity/ai/goals/arrow_attack_goal.h"
#include "world/entity/ai/look_control.h"
#include "world/entity/ai/sensing.h"
#include "world/entity/mob.h"
#include "world/level/pathfinder/path_navigation.h"
#include "world/entity/throwable.h"
#include "world/entity/entity_types.h"
#include "world/level/level.h"
#include "util/mth.h"

ArrowAttackGoal::ArrowAttackGoal(Mob* mob, float speed, int attackType,
                                 int delayBetweenAttacks)
:   mob(mob), speedMultiplier(speed), attackType(attackType),
    delayBetweenAttacks(delayBetweenAttacks), attackTimeout(0), seenTicks(0) {
    setRequiredControlFlags(CONTROL_MOVE | CONTROL_LOOK);
}

bool ArrowAttackGoal::canUse() { return mob->getTarget() != 0; }

bool ArrowAttackGoal::canContinueToUse() {
    if (canUse()) return true;
    return !mob->getNavigation()->isDone();
}

void ArrowAttackGoal::stop() { seenTicks = 0; }

void ArrowAttackGoal::tick() {
    Entity* t = mob->getTarget();
    if (!t) return;

    float d2 = mob->distanceToSqr(t->x, t->bb.y0, t->z);
    bool  canSee = mob->getSensing()->canSee(t);
    seenTicks = canSee ? seenTicks + 1 : 0;

    if (d2 <= 100.0f && seenTicks > 19) mob->getNavigation()->stop();
    else                                mob->getNavigation()->moveTo((Mob*)t, speedMultiplier);

    mob->getLookControl()->setLookAt(t, 30.0f, 30.0f);

    if (attackTimeout > 0) attackTimeout--;
    if (attackTimeout == 0 && d2 <= 100.0f && canSee) {
        fireAtTarget(t);
        attackTimeout = delayBetweenAttacks;
    }
}

void ArrowAttackGoal::fireAtTarget(Entity* t) {
    if (attackType == ATTACK_ARROW) {

        mob->performRangedAttack(t, 12.0f);
        return;
    }
    if (attackType != ATTACK_SNOWBALL) return;

    Throwable* s = new Throwable(mob->level, mob, EntityTypes::IdSnowball);
    float dx = t->x - mob->x;
    float dz = t->z - mob->z;
    float headY = t->isMob() ? t->y + ((Mob*)t)->getHeadHeight()
                             : (t->bb.y0 + t->bb.y1) * 0.5f;
    float dy = (headY - 1.1f) - s->y;
    float arc = Mth::sqrt(dx * dx + dz * dz) * 0.2f;
    s->shoot(dx, dy + arc, dz, 1.6f, 12.0f);
    mob->level->playSound(mob, "random.bow", 1.0f,
                          1.0f / (Entity::sharedRandom.nextFloat() * 0.4f + 0.8f));
    mob->level->addEntity(s);
}
