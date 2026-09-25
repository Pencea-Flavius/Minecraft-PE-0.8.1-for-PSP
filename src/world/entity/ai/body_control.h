
#ifndef MCPSP_WORLD_ENTITY_AI_BODY_CONTROL_H
#define MCPSP_WORLD_ENTITY_AI_BODY_CONTROL_H

class Mob;

class BodyControl {
public:
    BodyControl(Mob* mob) : mob(mob), stillTicks(0), lastHeadRot(0) {}
    void clientTick();

private:
    Mob* mob;
    int   stillTicks;
    float lastHeadRot;
};

#endif
