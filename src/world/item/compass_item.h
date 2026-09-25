#ifndef MCPSP_WORLD_ITEM_COMPASS_ITEM_H
#define MCPSP_WORLD_ITEM_COMPASS_ITEM_H

#include "world/item/item.h"

class CompassItem : public Item {
public:
    static const int FRAMES = 32;

    explicit CompassItem(short id) : Item(id) {}

    virtual int iconFrameCount() const { return FRAMES; }
    virtual int getAnimationFrameFor(const Player* p) const;
};

#endif
