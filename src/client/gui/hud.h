#pragma once
#include "client/gui/screens/menu.h"
#include "world/item/item_instance.h"

void hotbarDraw(MenuState& s);
void guiFill(float x, float y, float w, float h, unsigned int color);
void guiFillGradient(float x, float y, float w, float h, unsigned int topColor, unsigned int botColor);

#define GUI_SCROLL_MIN_OVERFLOW 3.0f

void guiScrollbar(float x, float y, float w, float h, float contentH, float scroll,
                  unsigned int alpha = 255);
void drawBlockIcon(short id, unsigned char data, float x, float y, float sizePx, unsigned int colorTint = 0xFFFFFFFFu);
void drawStackCount(Font& font, int count, float slotX, float slotY, float size);

void drawDurabilityBar(short id, short data, float x, float y, float sizePx);

void drawGuiItem(Font& font, const ItemInstance& item, float x, float y, float sizePx,
                 unsigned int tint = 0xFFFFFFFFu, bool showCount = true);
int getGuiBlockIcon(short id, unsigned char data);
const char* getBlockName(short id, unsigned char data);
const char* getBlockDescription(short id, unsigned char data);

int  itemFlatIcon(short id, unsigned char data);
void drawFlatIcon(int icon, float x, float y, float sizePx, unsigned int tint);

void hudChatMessage(const char* msg);
void hudChatClear(void);
