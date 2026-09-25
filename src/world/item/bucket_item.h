
#ifndef MCPSP_WORLD_ITEM_BUCKET_ITEM_H
#define MCPSP_WORLD_ITEM_BUCKET_ITEM_H

#include "world/item/item.h"

enum { BUCKET_EMPTY = 0, BUCKET_MILK = 1 };

class BucketItem : public Item {
public:
    explicit BucketItem(short id) : Item(id) {
        maxStackSize = 16;
        maxDamage    = 0;
    }

    virtual bool useOn(ItemInstance* item, Player* player, World* world, int x, int y, int z, int face,
                       float clickX, float clickY, float clickZ);

    virtual int  getMaxStackSize(const ItemInstance* item) const;
    virtual int  getIcon(short data) const;

    virtual bool isLiquidClipItem(short data) const { return data == BUCKET_EMPTY; }

    static bool emptyBucket(World* world, int type, int x, int y, int z);
};

#endif
