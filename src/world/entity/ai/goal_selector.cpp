
#include "world/entity/ai/goal_selector.h"
#include "world/entity/ai/goal.h"

GoalSelector::~GoalSelector() {
    for (unsigned int i = 0; i < goals.size(); i++)
        if (goals[i].owned) delete goals[i].goal;
}

void GoalSelector::addGoal(int priority, Goal* goal, bool owned) {

    if (goals.empty()) goals.reserve(10);
    goals.push_back(Entry(priority, goal, owned));
}

bool GoalSelector::canCoExist(const Entry& a, const Entry& b) {
    return (a.goal->getRequiredControlFlags() & b.goal->getRequiredControlFlags()) == 0;
}

bool GoalSelector::canUseInSystem(const Entry& e) const {
    for (unsigned int i = 0; i < goals.size(); i++) {
        const Entry& other = goals[i];
        if (other.goal == e.goal) continue;
        if (e.priority >= other.priority) {

            if (other.running && !canCoExist(e, other)) return false;
        } else if (other.running && !other.goal->canInterrupt()) {

            return false;
        }
    }
    return true;
}

void GoalSelector::tick() {

    for (unsigned int i = 0; i < goals.size(); i++) {
        Entry& e = goals[i];
        if (e.running) {
            if (canUseInSystem(e) && e.goal->canContinueToUse()) continue;
            e.goal->stop();
            e.running = false;
        }
        if (canUseInSystem(e) && e.goal->canUse()) {
            e.running = true;
            e.goal->start();
        }
    }
    for (unsigned int i = 0; i < goals.size(); i++)
        if (goals[i].running) goals[i].goal->tick();
}
