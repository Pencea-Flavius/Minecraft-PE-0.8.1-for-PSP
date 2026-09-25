#pragma once
#include "client/gui/screens/menu.h"

void inventoryDraw(MenuState& s);

enum { INV_BTN_BACK = 0, INV_BTN_CRAFT, INV_BTN_ARMOR, INV_BTN_COUNT };
bool invHeaderButton(MenuState& s, int i, float* x = 0, float* w = 0);

#include "world/item/creative_items.h"
enum { CREATIVE_COLS = 7 };
void creativeRebuild();
int  creativeCount();
const CreativeEntry* creativeEntry(int i);

bool creativeLocate(short id, short aux, int* tab, int* idx);
extern int  g_creativeTab;
