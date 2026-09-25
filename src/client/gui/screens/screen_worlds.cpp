#include <pspctrl.h>
#include <pspgu.h>
#include <cstdio>

#include "client/gui/screens/menu.h"
#include "client/gui/screens/screen.h"
#include "client/gui/hud.h"
#include "gpu/sprite.h"
#include "gpu/gui_atlas.h"

struct WorldsScreen : Screen {
    void renderContent(MenuState& s);
    void handleInput(MenuState& s, unsigned int pressed, unsigned int held);
};

static const float ROW_H = 32.0f;
static const float DEL_W = 32.0f;

static const float PANEL_X = 10.0f;
static float panelY() { return MENU_BAR_H + 6.0f; }
static const float PANEL_W = VW - 20.0f;

static const int VISIBLE_ROWS = 3;
static float panelH() { return VISIBLE_ROWS * ROW_H; }

static const char* kBarLabels[3] = { "Back", "New", "Edit" };
static float barButtonX(MenuState& s, int i) {
    if (i == 0) return 4.0f * MENU_PX;
    float editW = menuBarButtonW(s, kBarLabels[2]);
    float editX = VW - editW - 4.0f * MENU_PX;
    if (i == 2) return editX;
    return editX - menuBarButtonW(s, kBarLabels[1]) - 4.0f * MENU_PX;
}

void WorldsScreen::handleInput(MenuState& s, unsigned int pressed, unsigned int ) {
    WorldList& worlds = s.worlds;
    int& row = s.worldSelected;
    int& uiRow = s.uiRow;
    int& barSel = s.topSelected;
    AppScreen& screen = s.screen;

    if (row >= worlds.count) row = worlds.count - 1;
    if (row < 0) row = 0;
    if (worlds.count == 0) uiRow = 0;

    if (pressed & PSP_CTRL_DOWN) {
        if (uiRow == 0 && worlds.count > 0) uiRow = 1;
        else if (uiRow == 1 && row < worlds.count - 1) row++;
    }
    if (pressed & PSP_CTRL_UP) {
        if (uiRow == 1 && row > 0) row--;
        else if (uiRow == 1) uiRow = 0;
    }
    if (pressed & PSP_CTRL_RIGHT) {
        if (uiRow == 0 && barSel < 2) barSel++;
    }
    if (pressed & PSP_CTRL_LEFT) {
        if (uiRow == 0 && barSel > 0) barSel--;
    }

    if (pressed & PSP_CTRL_CIRCLE) {
        s.worldEditMode = 0;
        screen = SCREEN_TITLE;
    }

    if (pressed & PSP_CTRL_CROSS) {
        if (uiRow == 0) {
            if (barSel == 0) { s.worldEditMode = 0; screen = SCREEN_TITLE; }
            else if (barSel == 1) { createFormReset(s); screen = SCREEN_CREATE; }
            else s.worldEditMode = !s.worldEditMode;
        } else if (row < worlds.count) {
            if (s.worldEditMode) {
                s.deleteSelected = 1;
                screen = SCREEN_DELETE;
            } else {
                snprintf(s.statusMsg, sizeof(s.statusMsg), "Loading: %s", worlds.names[row]);
                screen = SCREEN_GAME;
            }
        }
    }
}

void WorldsScreen::renderContent(MenuState& s) {
    if (!s.haveTouch || !s.haveFont || !s.haveGui) return;
    WorldList& worlds = s.worlds;
    const bool editing = s.worldEditMode != 0;

    sceGuDisable(GU_DEPTH_TEST);

    const float pY = panelY(), pH = panelH();

    drawNinePatch(s, GA_SS_PANEL, 3.0f,
                  PANEL_X - 3.0f, pY - 3.0f, PANEL_W + 6.0f, pH + 6.0f);

    if (worlds.count > 0) {
        float contentH = worlds.count * ROW_H;
        float selY = s.worldSelected * ROW_H;
        float scroll = s.listScroll;
        if (selY < scroll)               scroll = selY;
        if (selY + ROW_H > scroll + pH)  scroll = selY + ROW_H - pH;
        float maxScroll = contentH - pH; if (maxScroll < 0.0f) maxScroll = 0.0f;
        if (scroll > maxScroll) scroll = maxScroll;
        if (scroll < 0.0f) scroll = 0.0f;
        s.listScroll = scroll;

        sceGuScissor((int)(PANEL_X * UI_SCALE), (int)(pY * UI_SCALE),
                     (int)(PANEL_W * UI_SCALE), (int)(pH * UI_SCALE));
        for (int i = 0; i < worlds.count; i++) {
            float rY = pY - scroll + i * ROW_H;
            if (rY > pY + pH || rY + ROW_H < pY) continue;

            bool hovered = (s.uiRow == 1 && s.worldSelected == i);
            float rowW = editing ? PANEL_W - DEL_W : PANEL_W;
            guiTButton(s, PANEL_X, rY, rowW, ROW_H, hovered && !editing, MENU_BEVEL);

            if (editing) {
                float bx = PANEL_X + PANEL_W - DEL_W;
                guiTButton(s, bx, rY, DEL_W, ROW_H, hovered, MENU_BEVEL);
                float is = 11.0f;
                textureBind(&s.touchGui);
                spriteDraw(&s.touchGui, (bx + (DEL_W - is) / 2.0f) * UI_SCALE,
                           (rY + (ROW_H - is) / 2.0f) * UI_SCALE, is * UI_SCALE, is * UI_SCALE,
                           240.0f, 240.0f, is, is, WHITE);
            }

            float textW = rowW - 10.0f;
            unsigned int nameCol = (hovered && !editing) ? 0xFFA0FFFFu : 0xFFFFFFFFu;
            const char* mode = worlds.gameModes[i] == 1 ? "Creative" : "Survival";
            float modeW = (float)fontTextWidth(&s.font, mode);

            if (editing) {
                char seed[24];
                snprintf(seed, sizeof(seed), "%ld", worlds.seeds[i]);
                float right = PANEL_X + rowW - 5.0f;
                float lw = (float)fontTextWidth(&s.font, "Seed:");
                float sw = (float)fontTextWidth(&s.font, seed);
                fontDrawTextShadow(&s.font, (right - lw) * UI_SCALE, (rY + 5.0f) * UI_SCALE,
                                   "Seed:", 0xFFBBBBBBu, UI_SCALE);
                fontDrawTextShadow(&s.font, (right - sw) * UI_SCALE, (rY + 16.0f) * UI_SCALE,
                                   seed, 0xFFBBBBBBu, UI_SCALE);
                textW -= (sw > lw ? sw : lw) + 5.0f;
            }

            fontDrawTextClipped(&s.font, (PANEL_X + 5.0f) * UI_SCALE, (rY + 5.0f) * UI_SCALE,
                                worlds.displayNames[i], nameCol, UI_SCALE, textW);
            fontDrawTextShadow(&s.font, (PANEL_X + 5.0f) * UI_SCALE, (rY + 16.0f) * UI_SCALE,
                               mode, 0xFFBBBBBBu, UI_SCALE);
            fontDrawTextShadow(&s.font, (PANEL_X + 5.0f + modeW + 10.0f) * UI_SCALE,
                               (rY + 16.0f) * UI_SCALE, worlds.dates[i], 0xFFBBBBBBu, UI_SCALE);
        }
        sceGuScissor(0, 0, 480, 272);

        guiScrollbar((PANEL_X + PANEL_W - 2.0f) * UI_SCALE, pY * UI_SCALE, 2.0f * UI_SCALE,
                     pH * UI_SCALE, contentH * UI_SCALE, scroll * UI_SCALE);
    }

    {
        float lb = 4.0f * MENU_PX + menuBarButtonW(s, kBarLabels[0]);
        float rb = barButtonX(s, 1);
        drawMenuHeader(s, "Play", 0.0f, VW, MENU_BAR_H, MENU_BAR_TEXT, lb, rb - lb);
    }
    for (int i = 0; i < 3; i++) {
        bool on = (s.uiRow == 0 && s.topSelected == i) || (i == 2 && editing);
        menuBarButton(s, barButtonX(s, i), menuBarButtonW(s, kBarLabels[i]), kBarLabels[i], on);
    }

    sceGuEnable(GU_DEPTH_TEST);
}

static WorldsScreen s_worldsScreen;
Screen& worldsScreen() { return s_worldsScreen; }
