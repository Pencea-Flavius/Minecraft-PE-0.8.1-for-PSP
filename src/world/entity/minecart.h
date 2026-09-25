
#ifndef MCPSP_WORLD_ENTITY_MINECART_H
#define MCPSP_WORLD_ENTITY_MINECART_H

#include "world/entity/entity.h"

class Player;

class Minecart : public Entity {
    typedef Entity super;
public:
    Minecart(Level* level);
    Minecart(Level* level, float x, float y, float z);

    virtual void tick();
    virtual int  getEntityTypeId() const;
    virtual EntityRendererId queryEntityRenderer();

    virtual bool isPickable()  { return !removed; }
    virtual bool isPushable()  { return true; }

    virtual float getRideHeight() { return -0.3f; }
    virtual float getShadowHeightOffs() { return 0.0f; }

    virtual bool interact();

    virtual bool hurt(Entity* source, int damage);

    virtual void addAdditonalSaveData(CompoundTag* tag);
    virtual void readAdditionalSaveData(CompoundTag* tag);

    virtual void push(Entity* e);

    float damage;
    int   hurtTime;
    int   hurtDir;
    bool  flipped;

    bool railPos(float px, float py, float pz, float* out);

    bool railPosOffs(float px, float py, float pz, float offs, float* out);

private:
    void init();
};

#endif
