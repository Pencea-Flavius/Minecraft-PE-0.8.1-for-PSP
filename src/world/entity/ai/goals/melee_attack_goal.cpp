#include "world/entity/ai/goals/melee_attack_goal.h"
#include "world/entity/ai/look_control.h"
#include "world/entity/ai/sensing.h"
#include "world/entity/mob.h"
#include "world/level/pathfinder/path_navigation.h"

MeleeAttackGoal::MeleeAttackGoal(Mob* attacker, float speed, bool persistent)
:   attacker(attacker), speedMultiplier(speed), persistent(persistent),
    attackCooldown(0), repathDelay(0) {
    setRequiredControlFlags(CONTROL_MOVE | CONTROL_LOOK);
}

bool MeleeAttackGoal::canUse() { return attacker->getTarget() != 0; }

bool MeleeAttackGoal::canContinueToUse() {
    if (!attacker->getTarget()) return false;
    if (persistent) return true;
    return !attacker->getNavigation()->isDone();
}

void MeleeAttackGoal::start() {
    repathDelay = 0;
    Entity* t = attacker->getTarget();
    if (t) attacker->getNavigation()->moveTo((Mob*)t, speedMultiplier);
}

void MeleeAttackGoal::stop() { attacker->getNavigation()->stop(); }

void MeleeAttackGoal::tick() {
    Entity* t = attacker->getTarget();
    if (!t) return;
    attacker->getLookControl()->setLookAt(t, 30.0f, 30.0f);

    if (persistent || attacker->getSensing()->canSee(t)) {
        if (--repathDelay <= 0) {
            repathDelay = 4 + Entity::sharedRandom.nextInt(7);
            attacker->getNavigation()->moveTo((Mob*)t, speedMultiplier);
        }
    }

    if (attackCooldown > 0) attackCooldown--;
    float reach = attacker->bbWidth * 2.0f;
    if (attackCooldown <= 0 &&
        attacker->distanceToSqr(t->x, t->bb.y0, t->z) <= reach * reach) {
        attackCooldown = 20;
        attacker->doHurtTarget(t);
    }
}
