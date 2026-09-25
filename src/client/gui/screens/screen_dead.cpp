
#include <pspctrl.h>
#include <pspgu.h>
#include <cstdio>

#include "client/player/player.h"
#include "client/gui/screens/screen.h"
#include "world/level/level.h"
#include "world/entity/local_player.h"
#include "gpu/sprite.h"
#include "platform/audio/sound.h"
#include "platform/time.h"
#include "gpu/gui_atlas.h"
#include "client/gui/hud.h"

bool g_deadScreen = false;
static int   s_deadSel  = 0;
static float s_openTime = 0.0f;

static const float DEAD_WAIT = 1.5f;

void deadScreenOpen() {

    extern bool g_invOpen, g_craftOpen, g_armorOpen, g_furnaceOpen;
    g_invOpen = g_craftOpen = g_armorOpen = g_furnaceOpen = false;
    chestClose();
    g_deadScreen = true;
    s_deadSel = 0;
    s_openTime = nowSeconds();
}
struct DeadScreen : Screen {
    void renderBackground(MenuState& s);
    void renderContent(MenuState& s);
    void handleInput(MenuState& s, unsigned int pressed, unsigned int held);
};

void DeadScreen::handleInput(MenuState& s, unsigned int pressed, unsigned int ) {
    int before = s_deadSel;
    if (pressed & PSP_CTRL_LEFT)  s_deadSel = 0;
    if (pressed & PSP_CTRL_RIGHT) s_deadSel = 1;
    if (s_deadSel != before) soundPlay("random.click", 1.0f, 1.0f);

    if (nowSeconds() - s_openTime < DEAD_WAIT) return;

    if (pressed & (PSP_CTRL_CROSS | PSP_CTRL_CIRCLE)) {
        soundPlay("random.click", 1.0f, 1.0f);
        if (s_deadSel == 0) {
            playerRespawn();
            g_deadScreen = false;
        } else {
            g_saveRequested = true;
            g_quitAfterSave = true;
            g_deadScreen = false;
        }
    }
    (void)s;
}

void DeadScreen::renderBackground(MenuState& s) {
    (void)s;
    sceGuDisable(GU_DEPTH_TEST);
    guiFillGradient(0.0f, 0.0f, 480.0f, 272.0f, 0x60000050u, 0xA0303080u);
}

void DeadScreen::renderContent(MenuState& s) {
    Font& font = s.font; bool haveFont = s.haveFont;
    bool haveTouch = s.haveGui;

    sceGuDisable(GU_DEPTH_TEST);

    if (haveFont) {
        const char* title = "You died!";
        float tw = fontTextWidth(&font, title) * UI_SCALE * 2.0f;
        fontDrawTextShadow(&font, (VW * UI_SCALE - tw) / 2.0f, 20.0f * UI_SCALE,
                           title, 0xFFFFFFFFu, UI_SCALE * 2.0f);

        char line[32];
        int sc = g_level.player ? g_level.player->getScore() : 0;
        snprintf(line, sizeof(line), "Score: %d", sc);
        float sw = fontTextWidth(&font, line) * UI_SCALE;
        fontDrawTextShadow(&font, (VW * UI_SCALE - sw) / 2.0f, 44.0f * UI_SCALE,
                           line, 0xFF55FFFFu, UI_SCALE);
    }

    if (haveTouch && haveFont) {
        bool active = (nowSeconds() - s_openTime >= DEAD_WAIT);
        const float btnW = 90.0f, btnH = 20.0f, gap = 12.0f;
        const float totalW = btnW * 2 + gap;
        const float startX = (VW - totalW) / 2.0f;
        const float by = 64.0f;
        const char* labels[2] = { "Respawn", "Main menu" };

        for (int i = 0; i < 2; ++i) {
            bool hover = active && (s_deadSel == i);
            float bx = startX + i * (btnW + gap);

            guiTButton(s, bx, by, btnW, btnH, hover);
            guiTButtonLabel(s, bx, by, btnW, btnH, labels[i], hover, active);
        }
    }

    sceGuEnable(GU_DEPTH_TEST);
}

static DeadScreen s_deadScreen;
Screen& deadScreen() { return s_deadScreen; }
