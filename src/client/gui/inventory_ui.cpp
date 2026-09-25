#include "client/gui/inventory_ui.h"
#include "client/gui/hud.h"
#include "client/player/player.h"
#include "world/level/level.h"
#include "world/entity/local_player.h"
#include "world/inventory/inventory.h"
#include "world/item/creative_items.h"
#include "world/item/item.h"
#include "client/gui/screens/menu.h"
#include "gpu/gu.h"
#include "gpu/texture.h"
#include "gpu/sprite.h"
#include "gpu/font.h"
#include <pspgu.h>
#include <cmath>
#include "gpu/gui_atlas.h"

#define HUD_S   2.0f

static void drawItemFrame(float x, float y, float w, float h) {
    const float BORDER_PX = 2.0f * HUD_S;
    const float b1 = BORDER_PX * 0.5f;
    const float b2 = BORDER_PX;
    const float b3 = BORDER_PX * 1.5f;

    const unsigned int BLACK  = 0xFF000000u;
    const unsigned int TOP2   = 0xFF6B7077u, BOT2  = 0xFF2A2729u;
    const unsigned int L2_T   = 0xFF434249u, L2_B  = 0xFF605E69u;
    const unsigned int R2_T   = 0xFF353136u, R2_B  = 0xFF4C464Eu;
    const unsigned int MID_T  = 0xFF51545Du, MID_B = 0xFF747885u;
    const unsigned int SHADOW = 0x4029211Eu;

    guiFill(x, y, w, b1, BLACK);
    guiFill(x, y + h - b1, w, b1, BLACK);
    guiFill(x, y, b1, h, BLACK);
    guiFill(x + w - b1, y, b1, h, BLACK);

    float o = b1;
    guiFill(x + o, y + o, w - 2 * o, b2, TOP2);
    guiFill(x + o, y + h - o - b2, w - 2 * o, b2, BOT2);
    guiFillGradient(x + o, y + o, b2, h - 2 * o, L2_T, L2_B);
    guiFillGradient(x + w - o - b2, y + o, b2, h - 2 * o, R2_T, R2_B);

    o = b1 + b2;
    guiFill(x + o, y + o, w - 2 * o, b3, MID_T);
    guiFill(x + o, y + h - o - b3, w - 2 * o, b3, MID_B);
    guiFillGradient(x + o, y + o, b3, h - 2 * o, MID_T, MID_B);
    guiFillGradient(x + w - o - b3, y + o, b3, h - 2 * o, MID_T, MID_B);

    o = b1 + b2 + b3;
    guiFill(x + o, y + o, w - 2 * o, b1, BLACK);
    guiFill(x + o, y + h - o - b1, w - 2 * o, b1, BLACK);
    guiFill(x + o, y + o, b1, h - 2 * o, BLACK);
    guiFill(x + w - o - b1, y + o, b1, h - 2 * o, BLACK);
    guiFill(x + o + b1, y + o + b1, w - 2 * (o + b1), b1, SHADOW);
}

static const float INV_BTN_H = 15.0f;
static const float INV_BTN_Y = (HEADER_H - INV_BTN_H) / 2.0f;

static const char* invHeaderLabel(int i) {
    return i == INV_BTN_BACK ? "Back" : i == INV_BTN_CRAFT ? "Craft" : "Armor";
}

bool invHeaderButton(MenuState& s, int i, float* x, float* w) {
    if (i < 0 || i >= INV_BTN_COUNT) return false;
    if (!g_level.player) return false;

    if (i != INV_BTN_BACK && g_level.player->inventory->isCreative()) return false;
    if (!x) return true;

    float bw[INV_BTN_COUNT];
    for (int k = 0; k < INV_BTN_COUNT; k++) {
        float tw = s.haveFont ? fontTextWidth(&s.font, invHeaderLabel(k)) + 8.0f : 30.0f;
        bw[k] = tw < 30.0f ? 30.0f : tw;
    }
    float armorX = VW - bw[INV_BTN_ARMOR] - 4.0f;
    *x = i == INV_BTN_BACK  ? 4.0f
       : i == INV_BTN_ARMOR ? armorX
                            : armorX - bw[INV_BTN_CRAFT] - 4.0f;
    if (w) *w = bw[i];
    return true;
}

static void invHeaderTitleGap(MenuState& s, float* gx, float* gw) {
    float x, w, left = 0.0f, right = VW;
    if (invHeaderButton(s, INV_BTN_BACK, &x, &w)) left = x + w;
    if (invHeaderButton(s, INV_BTN_CRAFT, &x, &w) ||
        invHeaderButton(s, INV_BTN_ARMOR, &x, &w)) right = x;
    *gx = left + 2.0f;
    *gw = right - left - 4.0f;
}

static constexpr float CB       = 22.0f;
static constexpr float CPAD     = 2.0f;
static constexpr float CPANEL_X = CB + 2.0f;
static constexpr float CPANEL_Y = 2.0f;
static constexpr float CPANEL_W = 240.0f - 4.0f - CB;
static constexpr float CPANEL_H = 136.0f - 25.0f;
static constexpr float CTAB_X   = CPANEL_X - CB + 3.0f;

static constexpr float CCELL    = 26.0f;
static constexpr float CPANE_W  = CCELL * CREATIVE_COLS;
static constexpr float CGUT     = ((CPANEL_W - 14.0f) - CPANE_W) / 2.0f;
static constexpr float CPANE_X  = CGUT + CPANEL_X + 7.0f;
static constexpr float CPANE_Y  = CPANEL_Y + 8.0f;
static constexpr float CPANE_H  = CPANEL_H - 16.0f;

static constexpr float CINSET   = 5.0f;

static constexpr float CSCROLL_W = 2.3333f;

static_assert(CGUT >= 0.0f, "CREATIVE_COLS columns of 26 do not fit the panel");
static_assert(CPANE_X + CPANE_W + CGUT <= CPANEL_X + CPANEL_W, "creative grid overflows the panel");

static constexpr float CTAB_RUN = CREATIVE_TABS * CB + (CREATIVE_TABS - 1) * CPAD;
static constexpr float CTAB_TOP = CPANEL_Y + (CPANEL_H - CTAB_RUN) / 2.0f;
static_assert(CTAB_RUN <= CPANEL_H, "the 4 tabs do not fit in the panel's height");

static_assert(CTAB_RUN + 2.0f * CPAD <= CPANEL_H,
              "the tab stack has no clear margin against the panel's top and bottom");

static float ctabY(int i) {
    return CTAB_TOP + (CREATIVE_TABS - 1 - i) * (CB + CPAD);
}

static constexpr float CICON    = 16.0f;

static constexpr float CICON_OFF = (CB - CICON) / 2.0f;

static_assert(CTAB_X + CICON_OFF + CICON <= CPANEL_X,
              "the tab icon runs under the panel -- an unselected tab would show it clipped");

int  g_creativeTab = 0;
static float s_cScrollY = 0.0f;

static inline float alignPx(float guiUnits) {
    return std::floor(guiUnits * UI_SCALE) / UI_SCALE;
}

void creativeRebuild() { CreativeItems::populate(); }
int  creativeCount()   { return CreativeItems::tabCount(g_creativeTab); }

const CreativeEntry* creativeEntry(int i) { return CreativeItems::at(g_creativeTab, i); }

bool creativeLocate(short id, short aux, int* tab, int* idx) {
    for (int t = 0; t < CREATIVE_TABS; t++)
        for (int k = 0; k < CreativeItems::tabCount(t); k++) {
            const CreativeEntry* e = CreativeItems::at(t, k);
            if (e && e->id == id && e->aux == aux) { *tab = t; *idx = k; return true; }
        }
    return false;
}

static ItemInstance creativeTabIcon(int tab) {
    switch (tab) {
        case 1:  return ItemInstance(BLOCK_BOOKSHELF, 1, 0);
        case 2:  return ItemInstance(ITEM_SWORD_IRON, 1, 0);
        case 3:  return ItemInstance(ITEM_SEEDS_WHEAT, 1, 0);
        default: return ItemInstance(BLOCK_BRICKS, 1, 0);
    }
}

static void creativeTabButton(MenuState& s, int i, bool sel) {
    const float by = ctabY(i);
    drawNinePatch(s, GA_SS_TABBTN, 3, CTAB_X, by, CB, CB, 3, sel ? 0xFFFFFFFFu : 0xFFBFBFBFu);
    ItemInstance icon = creativeTabIcon(i);
    drawGuiItem(s.font, icon, G(CTAB_X + CICON_OFF), G(by + CICON_OFF),
                G(CICON), sel ? 0xFFFFFFFFu : 0xFFB3B3B3u, false);
}

static void creativeInventoryDraw(MenuState& s) {
    CreativeItems::populate();

    const int n       = creativeCount();
    const int rows    = (n + CREATIVE_COLS - 1) / CREATIVE_COLS;
    const int visRows = (int)(CPANE_H / CCELL);

    if (g_invCursor >= n) g_invCursor = n > 0 ? n - 1 : 0;

    {
        int maxScroll = rows - visRows; if (maxScroll < 0) maxScroll = 0;
        int scrollRow = g_invCursor / CREATIVE_COLS - visRows / 2; if (scrollRow < 0) scrollRow = 0;
        if (scrollRow > maxScroll) scrollRow = maxScroll;
        const float target = scrollRow * CCELL;
        s_cScrollY += (target - s_cScrollY) * 0.35f;
        if (std::fabs(target - s_cScrollY) < 0.3f) s_cScrollY = target;
    }

    guiFill(0, 0, 480, 272, 0x66000000u);

    for (int i = 0; i < CREATIVE_TABS; i++)
        if (i != g_creativeTab) creativeTabButton(s, i, false);

    drawNinePatch(s, GA_SS_PANEL, 3, CPANEL_X, CPANEL_Y, CPANEL_W, CPANEL_H, 3);

    guiFill(G(CPANE_X - CGUT - 2.0f), G(CPANE_Y - 4.0f),
            G(CPANE_W + 2.0f * (CGUT + 2.0f)), G(CPANE_H + 8.0f), 0xFF333333u);

    const int firstRow = (int)(s_cScrollY / CCELL) - 1 < 0 ? 0 : (int)(s_cScrollY / CCELL) - 1;
    const int lastRow  = (int)((s_cScrollY + CPANE_H) / CCELL) + 1;
    int iStart = firstRow * CREATIVE_COLS, iEnd = (lastRow + 1) * CREATIVE_COLS;
    if (iEnd > n) iEnd = n;

    sceGuScissor((int)G(CPANE_X), (int)G(CPANE_Y), (int)G(CPANE_W), (int)G(CPANE_H) + 1);
    textureBind(&s.guiAtlas);
    for (int i = iStart; i < iEnd; i++)
        spriteDraw(&s.guiAtlas, G(CPANE_X + (i % CREATIVE_COLS) * CCELL),
                   G(CPANE_Y + (i / CREATIVE_COLS) * CCELL - s_cScrollY),
                   G(CCELL), G(CCELL), GA_SLOT_BG, 0xFFFFFFFFu);
    for (int i = iStart; i < iEnd; i++) {
        const CreativeEntry* e = creativeEntry(i);
        if (!e) continue;
        const float cx = CPANE_X + (i % CREATIVE_COLS) * CCELL;
        const float cy = CPANE_Y + (i / CREATIVE_COLS) * CCELL - s_cScrollY;
        unsigned int tint = 0xFFFFFFFFu;
        if (g_invFlashTicks > 0 && i == g_invFlashCursor) {
            int gv = 255 - g_invFlashTicks * 15; if (gv < 0) gv = 0;
            tint = 0xFF000000u | (gv << 16) | (gv << 8) | gv;
        }
        ItemInstance it(e->id, e->count, e->aux);
        drawGuiItem(s.font, it, G(alignPx(cx + CINSET)), G(alignPx(cy + CINSET)),
                    G(16), tint, false);
    }
    sceGuScissor(0, 0, 480, 272);

    if (s.haveGui && n > 0) {
        const float SEL_OVER = 2.0f;
        const float cx = CPANE_X + (g_invCursor % CREATIVE_COLS) * CCELL;
        const float cy = CPANE_Y + (g_invCursor / CREATIVE_COLS) * CCELL - s_cScrollY;
        textureBind(&s.guiAtlas);
        sceGuScissor(0, (int)G(CPANE_Y - SEL_OVER), 480, (int)G(CPANE_H + 2.0f * SEL_OVER + 1.0f));
        spriteDraw(&s.guiAtlas, G(cx - SEL_OVER), G(cy - SEL_OVER),
                   G(CCELL + 2.0f * SEL_OVER), G(CCELL + 2.0f * SEL_OVER + 1.0f), GA_SEL_FRAME, 0xFFFFFFFFu);
        sceGuScissor(0, 0, 480, 272);
    }

    if (rows > visRows)
        guiScrollbar(G(CPANE_X + CPANE_W + 1.0f), G(CPANE_Y), G(CSCROLL_W), G(CPANE_H),
                     G(rows * CCELL), G(s_cScrollY));

    creativeTabButton(s, g_creativeTab, true);

}

void inventoryDraw(MenuState& s) {
    if (!g_level.player) return;
    if (g_level.player->inventory->isCreative()) { creativeInventoryDraw(s); return; }

    const int   cols   = INV_COLS;
    const int   By = 6, ItemSize = 32, BlockBorder = 4, clipBottom = 0;
    const int   rows   = 1 + (g_level.player->inventory->gridSize() - 1) / cols;
    const float realW  = (float)(cols * ItemSize);
    const float realBx = (240.0f - realW) * 0.5f;
    const float paneX = realBx, paneY = 24.0f + By;
    const float paneW = realW,  paneH = 136.0f - By - By - 20.0f - 24.0f;

    const int visRows = (int)(paneH - clipBottom) / ItemSize;
    int curRow = g_invCursor / cols;
    int maxScroll = rows - visRows; if (maxScroll < 0) maxScroll = 0;
    int scrollRow = curRow - visRows / 2; if (scrollRow < 0) scrollRow = 0;
    if (scrollRow > maxScroll) scrollRow = maxScroll;

    const float targetScrollY = (float)(scrollRow * ItemSize);
    static float scrollY = 0.0f;
    scrollY += (targetScrollY - scrollY) * 0.35f;
    if (fabsf(targetScrollY - scrollY) < 0.3f) scrollY = targetScrollY;

    guiFill(0, 0, 480, 272, 0x66000000u);

    guiFill(G(paneX - realBx - 1), G(paneY - 4), G(paneW + 2 * realBx + 2),
            G(paneH + 8), 0xFF333333u);

    drawHeaderBar(s, true);

    sceGuScissor((int)G(paneX), (int)G(paneY), (int)G(paneW), (int)G(paneH - clipBottom) + 1);

    int firstRow = (int)floorf(scrollY / (float)ItemSize) - 1; if (firstRow < 0) firstRow = 0;
    int lastRow  = (int)floorf((scrollY + paneH) / (float)ItemSize) + 1;
    int iStart = firstRow * cols;
    int iEnd   = (lastRow + 1) * cols; if (iEnd > g_level.player->inventory->gridSize()) iEnd = g_level.player->inventory->gridSize();

    textureBind(&s.guiAtlas);
    for (int i = iStart; i < iEnd; i++) {
        float cx = paneX + (i % cols) * ItemSize;
        float cy = paneY + (i / cols) * ItemSize - scrollY;
        spriteDraw(&s.guiAtlas, G(cx), G(cy), G(ItemSize), G(ItemSize), GA_SLOT_BG, 0xFFFFFFFFu);
    }
    for (int i = iStart; i < iEnd; i++) {
        ItemInstance* it = g_level.player->inventory->gridItem(i);
        if (!it) continue;
        float cx = paneX + (i % cols) * ItemSize;
        float cy = paneY + (i / cols) * ItemSize - scrollY;
        unsigned int iconTint = 0xFFFFFFFFu;
        if (g_invFlashTicks > 0 && i == g_invFlashCursor) {
            int gv = 255 - g_invFlashTicks * 15;
            if (gv < 0) gv = 0;
            iconTint = 0xFF000000u | (gv << 16) | (gv << 8) | gv;
        }
        drawGuiItem(s.font, *it, G(cx + BlockBorder + 4), G(cy + BlockBorder + 4), G(16),
                    iconTint, !g_level.player->inventory->isCreative());
    }
    sceGuScissor(0, 0, 480, 272);

    if (s.haveGui && g_invHeaderSel < 0) {
        float cx = paneX + (g_invCursor % cols) * ItemSize;
        float cy = paneY + (g_invCursor / cols) * ItemSize - scrollY;
        textureBind(&s.guiAtlas);
        sceGuScissor(0, (int)G(paneY) - 2, 480, (int)G(paneH) + 7);
        spriteDraw(&s.guiAtlas, G(cx - 2), G(cy - 2), G(ItemSize + 4), G(ItemSize + 5), GA_SEL_FRAME, 0xFFFFFFFFu);
        sceGuScissor(0, 0, 480, 272);
    }

    {
        const float gx = G(paneX - realBx - 1), gw = G(paneW + 2 * realBx + 2), fade = G(16);

        guiFillGradient(gx, G(paneY) - 2.0f, gw, fade + 2.0f, 0x99000000u, 0x00000000u);
        guiFillGradient(gx, G(paneY + paneH) - fade, gw, fade + 2.0f, 0x00000000u, 0x99000000u);
    }

    if (rows > visRows) {
        static int fadeTimer = 0;
        static float scrollAlpha = 0.0f;
        if (fabsf(targetScrollY - scrollY) > 0.01f) {
            fadeTimer = 45;
            scrollAlpha += 0.33f;
            if (scrollAlpha > 1.0f) scrollAlpha = 1.0f;
        } else {
            if (fadeTimer > 0) fadeTimer--;
            else if (scrollAlpha > 0.0f) {
                scrollAlpha -= 0.10f;
                if (scrollAlpha < 0.0f) scrollAlpha = 0.0f;
            }
        }

        guiScrollbar(G(paneX + paneW) + G(2.0f), G(paneY), G(2.0f), G(paneH - clipBottom),
                     G(rows * ItemSize), G(scrollY),
                     (unsigned int)(scrollAlpha * 255.0f));
    }

    drawItemFrame(0, G(paneY - By), 480, G(paneH + 2 * By));

    float gapX, gapW;
    invHeaderTitleGap(s, &gapX, &gapW);
    drawHeaderTitle(s, "Select blocks", 0xFFFFFFFFu, gapX, gapW);

    for (int i = 0; i < INV_BTN_COUNT; i++) {
        float bx, bw;
        if (!invHeaderButton(s, i, &bx, &bw)) continue;
        bool sel = (g_invHeaderSel == i);
        guiTButton(s, bx, INV_BTN_Y, bw, INV_BTN_H, sel, MENU_BEVEL);
        guiTButtonLabel(s, bx, INV_BTN_Y, bw, INV_BTN_H, invHeaderLabel(i), sel, true);
    }
}
