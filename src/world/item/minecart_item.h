#ifndef MCPSP_WORLD_ITEM_MINECART_ITEM_H
#define MCPSP_WORLD_ITEM_MINECART_ITEM_H

#include "world/item/item.h"

class MinecartItem : public Item {
public:
    explicit MinecartItem(short id) : Item(id) { maxStackSize = 1; }
    virtual bool useOn(ItemInstance* item, Player* player, World* world,
                       int x, int y, int z, int face,
                       float clickX, float clickY, float clickZ);
};

#endif
