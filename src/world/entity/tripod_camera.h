
#ifndef MCPSP_WORLD_ENTITY_TRIPOD_CAMERA_H
#define MCPSP_WORLD_ENTITY_TRIPOD_CAMERA_H

#include "world/entity/mob.h"
#include "world/entity/entity_types.h"

class TripodCamera : public Mob {
public:

    TripodCamera(Level* level, float x, float y, float z, float ownerYaw, float ownerPitch);

    explicit TripodCamera(Level* level);

    virtual void tick();
    virtual bool isPickable() { return !removed; }
    virtual bool isPushable() { return false; }
    virtual int  getEntityTypeId() const { return EntityTypes::IdTripodCamera; }

    virtual bool hurt(Entity* source, int dmg);

    virtual const char* getHurtSound()  { return "random.pop"; }
    virtual const char* getDeathSound() { return "random.pop"; }

    void breakAndDrop();

private:
    void init();
};

#endif
