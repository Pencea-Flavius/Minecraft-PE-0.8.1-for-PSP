
#ifndef MCPSP_WORLD_LEVEL_TILE_ENTITY_CHEST_TILE_ENTITY_H
#define MCPSP_WORLD_LEVEL_TILE_ENTITY_CHEST_TILE_ENTITY_H

#include "world/level/tile/entity/tile_entity.h"
#include "world/inventory/filling_container.h"

class CompoundTag;
class ItemInstance;

class ChestTileEntity : public TileEntity {
    typedef TileEntity super;
public:
    static const int CONTAINER_SIZE = 27;

    ChestTileEntity();
    virtual ~ChestTileEntity();

    virtual bool save(CompoundTag* tag);
    virtual void load(CompoundTag* tag);
    virtual void tick();

    FillingContainer container;

    ChestTileEntity* pair;

    bool pairPending;
    int  pairX, pairZ;

    void pairWith(ChestTileEntity* other);
    void unpair();
    bool canPairWith(TileEntity* te) const;
    float getModelOffsetX() const;

    bool isMaster() const;
    int  getContainerSize() const { return pair ? 2 * CONTAINER_SIZE : CONTAINER_SIZE; }

    ItemInstance* getItem(int i) const;
    void          clearSlot(int i);
    bool          add(ItemInstance& it);

    float openness, oOpenness;
    int   openCount;

    void startOpen();
    void stopOpen();

    ChestTileEntity* openStateOwner();

    static const int OPEN_DELAY = 6;
    void openBy();
    bool canOpen() const;
    bool _canOpenThis() const;
    bool openedBy;
    int  openCountdown;

    enum { LID_SOUND_NONE = 0, LID_SOUND_OPEN = 1, LID_SOUND_CLOSE = 2 };
    int stepOpenness();

private:
    void resolvePendingPair();
};

#endif
