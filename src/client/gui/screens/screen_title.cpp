
#include <pspctrl.h>
#include <pspgu.h>

#include "client/gui/screens/menu.h"
#include "client/gui/screens/screen.h"
#include "gpu/sprite.h"

#include <cmath>
#include <ctime>
#include <pspkernel.h>

#include <cstdio>
#include <cstring>
#include "platform/path.h"

static char s_splash[128];
static bool s_splashPicked = false;

static void pickSplash(unsigned seed) {
    s_splashPicked = true;
    FILE* f = fopen(assetPath("data/splashes.txt"), "r");
    if (!f) return;
    char line[128];
    unsigned n = 0;
    while (fgets(line, sizeof(line), f)) {
        char* e = line + strlen(line);
        while (e > line && (e[-1] == '\n' || e[-1] == '\r' || e[-1] == ' ')) *--e = '\0';
        if (line[0] == '\0' || line[0] == '#') continue;
        seed = seed * 1664525u + 1013904223u;
        if (seed % ++n == 0) strcpy(s_splash, line);
    }
    fclose(f);
}

static const float BTN_W = 100.0f;
static const float BTN_H = 30.0f;
static const float BTN_X = (float)((int)VW / 2 - 50);
static const float PLAY_Y = (float)((int)VH / 2 + 10);

static const float SET_SIZE = 32.0f;
static const float SET_X = VW - SET_SIZE - 2.0f;
static const float SET_Y = VH - SET_SIZE - 2.0f;

enum { BTN_PLAY = 0, BTN_SETTINGS = 1, numButtons = 2 };

static const unsigned int kTitleSeed[3] = {
    0x0251B8B0u, 0x1360B0C0u, 0x00000275u
};
#define TITLE_SEED_LEN 14
static int s_seedHold = 0;

struct TitleScreen : Screen {
    void renderContent(MenuState& s);
    void handleInput(MenuState& s, unsigned int pressed, unsigned int held);
};

void TitleScreen::handleInput(MenuState& s, unsigned int pressed, unsigned int held) {

    static const unsigned int SEED_MASK = PSP_CTRL_LTRIGGER | PSP_CTRL_RTRIGGER | PSP_CTRL_UP;
    s_seedHold = ((held & SEED_MASK) == SEED_MASK) ? (s_seedHold + 1) : 0;

    int& selected = s.selected;
    AppScreen& screen = s.screen;
    char (&statusMsg)[128] = s.statusMsg;
    int& optFocus = s.optFocus;
    int& optTabHighlight = s.optTabHighlight;
    int& optItemHighlight = s.optItemHighlight;
    int& optCategory = s.optCategory;

    if (selected < 0) selected = BTN_PLAY;
    if (pressed & (PSP_CTRL_UP | PSP_CTRL_LEFT))    selected = BTN_PLAY;
    if (pressed & (PSP_CTRL_DOWN | PSP_CTRL_RIGHT)) selected = BTN_SETTINGS;

    if (pressed & PSP_CTRL_CROSS) {
        if (selected == BTN_PLAY) {
            screen = SCREEN_WORLDS;
            statusMsg[0] = '\0';
        } else if (selected == BTN_SETTINGS) {
            optFocus = 1;
            optTabHighlight = optCategory;
            optItemHighlight = 0;
            screen = SCREEN_OPTIONS;
            statusMsg[0] = '\0';
        }
    }
}

void TitleScreen::renderContent(MenuState& s) {
    Font& font = s.font; bool haveFont = s.haveFont;
    bool haveGui = s.haveGui;
    Texture& logo = s.logo; bool haveLogo = s.haveLogo;
    int& selected = s.selected;

    float logoYV = 12.0f;

    float half = (float)logo.realW * 0.5f;
    if (VW * 0.5f <= half) half = VW * 0.5f;
    float logoWV = half + half;
    float logoHV = logo.realW ? (logoWV / (float)logo.realW) * (float)logo.realH : 1.0f;
    float logoXV = (float)((int)VW / 2) - half;
    if (haveLogo) {
        textureBind(&logo);
        sceGuDisable(GU_DEPTH_TEST);
        spriteDraw(&logo, logoXV * UI_SCALE, logoYV * UI_SCALE,
                  logoWV * UI_SCALE, logoHV * UI_SCALE,
                  0, 0, (float)logo.realW, (float)logo.realH, WHITE);
        sceGuEnable(GU_DEPTH_TEST);
    }

    if (haveFont) {

        if (!s_splashPicked)
            pickSplash((unsigned)time(0) * 2654435761u + sceKernelGetSystemTimeLow());
        const char* splash = s_splash;

        float t = (float)sceKernelGetSystemTimeLow() * 1e-6f;
        float scale = powf(sinf(t * 3.14f * 2.3f), 4.0f) * 0.06f + 1.3f;

        float len = (float)fontTextWidth(&font, splash);
        float fit = (VW * 0.3125f) / (len * 1.3f);
        if (fit > 1.0f) fit = 1.0f;

        sceGuDisable(GU_DEPTH_TEST);
        fontDrawTransformed(&font, (logoXV + logoWV) * 0.71f * UI_SCALE,
                            (logoYV + logoHV - 15.0f) * UI_SCALE,
                            splash, 0xFF00FFFFu ,
                            -20.0f, scale * fit * UI_SCALE, true);
        sceGuEnable(GU_DEPTH_TEST);
    }

    if (haveGui && haveFont) {
        sceGuDisable(GU_DEPTH_TEST);
        guiTButton(s, BTN_X, PLAY_Y, BTN_W, BTN_H, selected == BTN_PLAY);
        guiTButtonLabel(s, BTN_X, PLAY_Y, BTN_W, BTN_H, "Play",
                        selected == BTN_PLAY, true);
        guiTButton(s, SET_X, SET_Y, SET_SIZE, SET_SIZE, selected == BTN_SETTINGS);
        if (s.haveTouch) {
            const float iw = 22.0f, ih = 21.0f;
            textureBind(&s.touchGui);
            spriteDraw(&s.touchGui, (SET_X + (SET_SIZE - iw) / 2.0f) * UI_SCALE,
                       (SET_Y + (SET_SIZE - ih) / 2.0f) * UI_SCALE, iw * UI_SCALE, ih * UI_SCALE,
                       218.0f, 0.0f, iw, ih, WHITE);
        }
        sceGuEnable(GU_DEPTH_TEST);
    }

    if (haveFont) {
        sceGuDisable(GU_DEPTH_TEST);

        fontDrawTextShadow(&font, 1.0f * UI_SCALE, (VH - 10.0f) * UI_SCALE,
                           "\xffMojang AB", WHITE, UI_SCALE);
        fontDrawTextShadow(&font, 1.0f * UI_SCALE, (VH - 20.0f) * UI_SCALE,
                           "v0.8.1 alpha", WHITE, UI_SCALE);
        if (s_seedHold > 30) {
            char line[TITLE_SEED_LEN + 1];
            for (int i = 0; i < TITLE_SEED_LEN; i++) {
                unsigned int v = (kTitleSeed[i / 6] >> (5 * (i % 6))) & 31u;
                line[i] = v ? (char)(0x40u + v) : ' ';
            }
            line[TITLE_SEED_LEN] = '\0';
            float lw = fontTextWidth(&font, line) * UI_SCALE;
            fontDrawTextShadow(&font, (480.0f - lw) * 0.5f, 272.0f - 30.0f * UI_SCALE,
                               line, 0xFF80FFFFu, UI_SCALE);
        }
        sceGuEnable(GU_DEPTH_TEST);
    }
}

static TitleScreen s_titleScreen;
Screen& titleScreen() { return s_titleScreen; }
