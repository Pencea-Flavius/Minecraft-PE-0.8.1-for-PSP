
#include "world/entity/local_player.h"
#include <pspctrl.h>
#include <pspgu.h>
#include <pspkernel.h>
#include <cstdio>

#include "client/player/player.h"
#include "client/gui/screens/screen.h"
#include "client/gui/hud.h"
#include "gpu/sprite.h"
#include "platform/audio/sound.h"
#include "world/item/item.h"
#include "world/level/level.h"
#include "world/level/tile/entity/chest_tile_entity.h"
#include "world/inventory/inventory.h"
#include "gpu/gui_atlas.h"

extern Level g_level;

bool g_chestOpen = false;
static ChestTileEntity* s_chest = nullptr;

static int s_pane = 0;
static int s_cursor[2] = { 0, 0 };
static float s_scroll[2] = { 0.0f, 0.0f };

static HoldCharge  s_charge;

static int paneSize(int pane) {
    return pane == 0 ? g_level.player->inventory->gridSize() : s_chest->getContainerSize();
}
static ItemInstance* paneItem(int pane, int i) {
    return pane == 0 ? g_level.player->inventory->gridItem(i) : s_chest->getItem(i);
}
static void paneClear(int pane, int i) {
    if (pane == 0) g_level.player->inventory->clearSlot(i + g_level.player->inventory->firstGridSlot());
    else           s_chest->clearSlot(i);
}
static bool paneAdd(int pane, ItemInstance& it) {
    return pane == 0 ? g_level.player->inventory->add(it) : s_chest->add(it);
}

bool chestCursorOnChest() { return s_pane == 1; }

static void moveAcross(int n) {

    if (g_level.player->inventory->isCreative()) return;
    int slot = s_cursor[s_pane];
    ItemInstance* src = paneItem(s_pane, slot);
    if (!src || src->isNull()) return;
    if (n > src->count) n = src->count;

    ItemInstance taken(src->id, (short)n, src->data);
    short before = taken.count;
    if (!paneAdd(1 - s_pane, taken)) {

        n = before - taken.count;
    }
    if (n <= 0) return;
    src->count -= n;
    if (src->count <= 0) paneClear(s_pane, slot);
    soundPlay("random.pop", 0.3f, 1.4f, SND_CAT_UI);
}

static int s_cx, s_cy, s_cz;
static bool chestGone() { return g_level.getTileEntity(s_cx, s_cy, s_cz) != s_chest; }

void chestOpen(ChestTileEntity* ce) {
    s_chest = ce;
    s_cx = ce->x; s_cy = ce->y; s_cz = ce->z;
    ce->startOpen();
    s_pane = 0;
    s_cursor[0] = s_cursor[1] = 0;
    s_scroll[0] = s_scroll[1] = 0.0f;
    g_chestOpen = true;
    soundPlay("random.click", 1.0f, 1.0f);
}

void chestClose() {
    if (s_chest) {
        s_chest->stopOpen();
        s_chest = nullptr;
    }
    g_chestOpen = false;
}
struct ChestScreen : ContainerScreen {
    void renderContent(MenuState& s);
    void handleInput(MenuState& s, unsigned int pressed, unsigned int held);
};

void ChestScreen::handleInput(MenuState& s, unsigned int pressed, unsigned int held) {
    (void)s;

    if (!s_chest || !g_level.player) { g_chestOpen = false; return; }

    if (chestGone()) { s_chest = nullptr; chestClose(); return; }

    if (!s_chest->stillValid(g_level.player)) { chestClose(); return; }
    const int cols = 3;
    int& cur = s_cursor[s_pane];
    const int n = paneSize(s_pane);
    int before = s_pane * 1000 + cur;

    if (pressed & PSP_CTRL_LEFT) {
        if (cur % cols > 0) cur--;
        else if (s_pane == 1) s_pane = 0;
    }
    if (pressed & PSP_CTRL_RIGHT) {
        if (cur % cols < cols - 1) cur++;
        else if (s_pane == 0) s_pane = 1;
    }
    if ((pressed & PSP_CTRL_UP)   && cur >= cols)    cur -= cols;
    if ((pressed & PSP_CTRL_DOWN) && cur + cols < n) cur += cols;
    if (s_cursor[s_pane] >= paneSize(s_pane)) s_cursor[s_pane] = paneSize(s_pane) - 1;

    int curKey = s_pane * 1000 + s_cursor[s_pane];
    ItemInstance* src = paneItem(s_pane, s_cursor[s_pane]);
    int count = (src && !src->isNull()) ? src->count : 0;

    if ((pressed & PSP_CTRL_TRIANGLE) && count > 0) {
        moveAcross(count);
        s_charge.reset();
    } else if ((pressed & PSP_CTRL_SQUARE) && count > 0) {
        moveAcross((count + 1) / 2);
        s_charge.reset();
    }

    bool charging = (held & PSP_CTRL_CROSS) && !g_level.player->inventory->isCreative();
    if (int move = holdChargeUpdate(s_charge, curKey, count, charging)) moveAcross(move);

    if (before != s_pane * 1000 + s_cursor[s_pane])
        soundPlay("random.click", 1.0f, 1.0f);

    if (pressed & PSP_CTRL_CIRCLE) {
        chestClose();
        soundPlay("random.click", 1.0f, 1.0f);
    }
}

static void drawPane(MenuState& s, int paneIdx, float paneX) {
    Font& font = s.font; bool haveFont = s.haveFont;
    const float paneY = 32.0f, ItemSize = 32.0f;
    const int   cols = 3, visRows = 3;
    const float paneW = cols * ItemSize, paneH = visRows * ItemSize;
    const int n = paneSize(paneIdx);
    const int rows = 1 + (n - 1) / cols;

    drawNinePatch(s, GA_SS_PANE_X, GA_SS_PANE_Y, 8, 8, 2, paneX - 2, paneY - 2, paneW + 4, paneH + 4);

    {
        int curRow = s_cursor[paneIdx] / cols;
        int maxScroll = rows - visRows; if (maxScroll < 0) maxScroll = 0;
        int scrollRow = curRow - visRows / 2; if (scrollRow < 0) scrollRow = 0;
        if (scrollRow > maxScroll) scrollRow = maxScroll;
        float target = scrollRow * ItemSize;
        s_scroll[paneIdx] += (target - s_scroll[paneIdx]) * 0.35f;
    }
    float scroll = s_scroll[paneIdx];

    sceGuScissor((int)G(paneX), (int)G(paneY), (int)G(paneW), (int)G(paneH));
    if (s.haveGui) {
        textureBind(&s.guiAtlas);
        for (int i = 0; i < n; ++i) {
            float cy = paneY + (i / cols) * ItemSize - scroll;
            if (cy < paneY - ItemSize || cy > paneY + paneH) continue;
            spriteDraw(&s.guiAtlas, G(paneX + (i % cols) * ItemSize), G(cy),
                       G(ItemSize), G(ItemSize), GA_SLOT_BG, WHITE);
        }
    }
    for (int i = 0; i < n; ++i) {
        float cy = paneY + (i / cols) * ItemSize - scroll;
        if (cy < paneY - ItemSize || cy > paneY + paneH) continue;
        ItemInstance* it = paneItem(paneIdx, i);
        if (!it || it->isNull()) continue;
        float cx = paneX + (i % cols) * ItemSize;
        drawGuiItem(font, *it, G(cx + 8), G(cy + 8), G(16), WHITE, haveFont);
    }
    sceGuScissor(0, 0, 480, 272);

    if (s.haveGui) {
        int cur = s_cursor[paneIdx];
        float cx = paneX + (cur % cols) * ItemSize;
        float cy = paneY + (cur / cols) * ItemSize - scroll;
        unsigned int tint = (paneIdx == s_pane) ? WHITE : 0xFF808080u;
        textureBind(&s.guiAtlas);
        sceGuScissor(0, (int)G(paneY) - 2, 480, (int)G(paneH) + 7);
        spriteDraw(&s.guiAtlas, G(cx - 2), G(cy - 2), G(ItemSize + 4), G(ItemSize + 5), GA_SEL_FRAME, tint);
        sceGuScissor(0, 0, 480, 272);

        guiScrollbar(G(paneX + paneW) + G(2.0f), G(paneY), G(2.0f), G(paneH),
                     G(rows * ItemSize), G(scroll));

        if (paneIdx == s_pane && s_charge.share > 0.0f) {
            float bw = ItemSize - 8.0f;
            guiFill(G(cx + 4), G(cy + ItemSize - 7), G(bw),              G(4), 0xFF404040u);
            guiFill(G(cx + 4), G(cy + ItemSize - 7), G(bw * s_charge.share), G(4), 0xFF00FF00u);
        }
    }
}

void ChestScreen::renderContent(MenuState& s) {

    if (!s_chest || !g_level.player || chestGone()) return;

    sceGuDisable(GU_DEPTH_TEST);

    const float paneW = 3 * 32.0f;
    drawHeaderTitle(s, "Inventory", s_pane == 0 ? 0xFFFFFFFFu : 0xFFB0B0B0u, 20.0f,  paneW);
    drawHeaderTitle(s, "Chest",     s_pane == 1 ? 0xFFFFFFFFu : 0xFFB0B0B0u, 124.0f, paneW);

    drawPane(s, 0, 20.0f);
    drawPane(s, 1, 124.0f);

    sceGuEnable(GU_DEPTH_TEST);
}

static ChestScreen s_chestScreen;
Screen& chestScreen() { return s_chestScreen; }
