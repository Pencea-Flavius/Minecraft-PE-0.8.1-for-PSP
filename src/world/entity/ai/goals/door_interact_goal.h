
#ifndef MCPSP_WORLD_ENTITY_AI_GOALS_DOOR_INTERACT_GOAL_H
#define MCPSP_WORLD_ENTITY_AI_GOALS_DOOR_INTERACT_GOAL_H

#include "world/entity/ai/goal.h"

class Mob;

class DoorInteractGoal : public Goal {
public:
    DoorInteractGoal(Mob* mob);
    virtual bool canUse();
    virtual bool canContinueToUse();
    virtual void start();
    virtual void tick();

protected:

    bool isDoorOpen() const;
    bool isDoorAt(int x, int y, int z) const;

    Mob* mob;
    int  doorX, doorY, doorZ;
    bool haveDoor;
    bool pastDoor;
    float toDoorX, toDoorZ;
};

#endif
