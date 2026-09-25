
#include <pspctrl.h>
#include <pspgu.h>

#include "client/player/player.h"
#include "client/gui/screens/screen.h"
#include "world/level/level.h"
#include "world/entity/local_player.h"
#include "gpu/sprite.h"
#include "client/gui/hud.h"
#include "platform/audio/sound.h"
#include "gpu/gui_atlas.h"
struct InBedScreen : Screen {
    void renderBackground(MenuState& s) {}
    void renderContent(MenuState& s);
    void handleInput(MenuState& s, unsigned int pressed, unsigned int held);
};

void InBedScreen::handleInput(MenuState& s, unsigned int pressed, unsigned int ) {
    (void)s;

    if (pressed & (PSP_CTRL_CROSS | PSP_CTRL_CIRCLE | PSP_CTRL_LTRIGGER)) {
        soundPlay("random.click", 1.0f, 1.0f);

        g_level.player->stopSleepInBed(true, true);
    }
}

void inBedRenderFade(MenuState& s) {
    (void)s;
    LocalPlayer* p = g_level.player;
    if (!p) return;

    float share = (float)p->sleepCounter / (float)Player::SLEEP_DURATION;
    if (share < 0.0f) share = 0.0f;
    if (share > 1.0f) share = 1.0f;
    unsigned int a = (unsigned int)(share * 220.0f);
    guiFill(0.0f, 0.0f, 480.0f, 272.0f, (a << 24));
}

void InBedScreen::renderContent(MenuState& s) {
    bool haveTouch = s.haveGui;
    bool haveFont  = s.haveFont;
    LocalPlayer* p = g_level.player;
    if (!p) return;

    sceGuDisable(GU_DEPTH_TEST);

    if (haveTouch && haveFont) {
        const char* label = "Leave Bed";
        const float btnW = 100.0f, btnH = 20.0f;

        const float bx = (VW - btnW) / 2.0f, by = VH - btnH - 26.0f;

        guiTButton(s, bx, by, btnW, btnH, true);
        guiTButtonLabel(s, bx, by, btnW, btnH, label, true, true);
    }

    sceGuEnable(GU_DEPTH_TEST);
}

static InBedScreen s_inBedScreen;
Screen& inBedScreen() { return s_inBedScreen; }
