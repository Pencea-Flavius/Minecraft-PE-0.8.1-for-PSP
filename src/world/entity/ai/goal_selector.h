
#ifndef MCPSP_WORLD_ENTITY_AI_GOAL_SELECTOR_H
#define MCPSP_WORLD_ENTITY_AI_GOAL_SELECTOR_H

#include <vector>

class Goal;

class GoalSelector {
public:
    GoalSelector() {}
    ~GoalSelector();

    void addGoal(int priority, Goal* goal, bool owned = true);
    void tick();

private:
    struct Entry {
        Goal* goal;
        int   priority;
        bool  owned;
        bool  running;
        Entry(int p, Goal* g, bool o) : goal(g), priority(p), owned(o), running(false) {}
    };

    bool canUseInSystem(const Entry& e) const;

    static bool canCoExist(const Entry& a, const Entry& b);

    std::vector<Entry> goals;
};

#endif
