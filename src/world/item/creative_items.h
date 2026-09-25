
#ifndef MCPSP_WORLD_ITEM_CREATIVE_ITEMS_H
#define MCPSP_WORLD_ITEM_CREATIVE_ITEMS_H

struct CreativeEntry {
    short         id;
    unsigned char count;
    short         aux;
};

enum { CREATIVE_TABS = 4 };

namespace CreativeItems {

void populate();

int                 tabCount(int tab);
const CreativeEntry* at(int tab, int i);

int                 count();
const CreativeEntry* entry(int i);

}

#endif
