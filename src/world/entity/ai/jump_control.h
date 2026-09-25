
#ifndef MCPSP_WORLD_ENTITY_AI_JUMP_CONTROL_H
#define MCPSP_WORLD_ENTITY_AI_JUMP_CONTROL_H

class Mob;

class JumpControl {
public:
    JumpControl(Mob* mob) : mob(mob), jumping(false) {}
    void jump() { jumping = true; }
    void tick();

private:
    Mob* mob;
    bool jumping;
};

#endif
