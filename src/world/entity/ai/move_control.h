
#ifndef MCPSP_WORLD_ENTITY_AI_MOVE_CONTROL_H
#define MCPSP_WORLD_ENTITY_AI_MOVE_CONTROL_H

class Mob;

class MoveControl {
public:
    MoveControl(Mob* mob);

    float getSpeed() const { return speedMultiplier; }
    bool  hasWanted() const { return _hasWanted; }
    void  setWantedPosition(float x, float y, float z, float speed);
    void  tick();

private:
    Mob*  mob;
    float wantedX, wantedY, wantedZ;
    float speedMultiplier;
    bool  _hasWanted;
};

#endif
