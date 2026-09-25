
#ifndef MCPSP_WORLD_ENTITY_AI_LOOK_CONTROL_H
#define MCPSP_WORLD_ENTITY_AI_LOOK_CONTROL_H

class Mob;
class Entity;

class LookControl {
public:
    LookControl(Mob* mob);

    void setLookAt(Entity* target, float yawSpeed, float pitchSpeed);
    void setLookAt(float x, float y, float z, float yawSpeed, float pitchSpeed);
    void tick();

private:
    Mob*  mob;
    float yawSpeed, pitchSpeed;
    bool  isLooking;
    float lookX, lookY, lookZ;
};

#endif
