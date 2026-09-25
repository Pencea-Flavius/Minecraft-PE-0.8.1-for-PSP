#ifndef MCPSP_WORLD_ITEM_CLOCK_ITEM_H
#define MCPSP_WORLD_ITEM_CLOCK_ITEM_H

#include "world/item/item.h"

class ClockItem : public Item {
public:
    static const int FRAMES = 64;

    explicit ClockItem(short id) : Item(id) {}

    virtual int iconFrameCount() const { return FRAMES; }
    virtual int getAnimationFrameFor(const Player* p) const;
};

#endif
