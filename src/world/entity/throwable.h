
#ifndef MCPSP_WORLD_ENTITY_THROWABLE_H
#define MCPSP_WORLD_ENTITY_THROWABLE_H

#include "world/entity/entity.h"

class Mob;

class Throwable : public Entity {
    typedef Entity super;
public:
    Throwable(Level* level, int type);
    Throwable(Level* level, float x, float y, float z,
              float yaw, float pitch, int type);

    Throwable(Level* level, Mob* owner, int type);

    virtual void tick();
    virtual int  getEntityTypeId() const { return type; }
    virtual void addAdditonalSaveData(CompoundTag* ) {}
    virtual void readAdditionalSaveData(CompoundTag* ) {}

    int   ownerId;
    int   type;
    short itemId;
    int   life;

    void shoot(float dx, float dy, float dz, float power, float inaccuracy = 1.0f);

private:
    void configure(int type);
    void onHit();
};

#endif
