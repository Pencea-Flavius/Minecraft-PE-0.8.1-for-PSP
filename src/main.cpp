
#include <pspkernel.h>
#include <pspsysmem.h>
#include <pspiofilemgr.h>
#include <pspctrl.h>
#include <psppower.h>
#include <pspfpu.h>
#include <cstring>
#include <malloc.h>
#include <cstdlib>
#include <cstdio>
#include <cmath>

#include "gpu/gu.h"
#include "gpu/texture.h"
#include "gpu/sprite.h"
#include "gpu/font.h"
#include "platform/path.h"
#include "platform/power.h"
#include "util/prof.h"
#include "platform/audio/sound.h"
#include "world/level/storage/worldlist.h"
#include "world/level/world.h"
#include "world/level/chunk/chunk.h"
#include "world/level/chunk/chunk_cache.h"
#include "world/level/storage/chunk_storage.h"
#include "world/level/tile/tile.h"
#include "client/gui/screens/menu.h"
#include "client/gui/screens/control_scheme.h"
#include "client/gui/screens/screen.h"
#include "client/gui/screens/panorama.h"
#include "client/player/player.h"
#include "client/renderer/render.h"

#include "platform/time.h"
#include "world/level/level.h"
#include "world/entity/entity.h"
#include "world/entity/local_player.h"
#include "world/entity/item_entity.h"
#include "world/entity/entity_types.h"
#include "client/renderer/item_hand.h"

#include <pspgu.h>
#include <pspgum.h>

SceInt64 g_timeBootUs = 0;
float    g_gameSeconds = 0.0f;
bool     g_gameFrozen  = true;

PSP_MODULE_INFO("MinecraftPSP", 0, 0, 1);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);

PSP_HEAP_SIZE_KB(-1024);

int g_pspIsGo = 0;
static void detectPspGo(void) {
    SceUID d = sceIoDopen("ef0:/");
    if (d >= 0) { sceIoDclose(d); g_pspIsGo = 1; }
}

int g_lowMemPsp  = 0;
int g_lowMemHeap = 0;
static void detectLowMemPsp(void) {
    enum { MAX_BLOCKS = 64 };
    void* blocks[MAX_BLOCKS];
    int n = 0;
    while (n < MAX_BLOCKS) { void* p = malloc(1024 * 1024); if (!p) break; blocks[n++] = p; }
    for (int i = 0; i < n; i++) free(blocks[i]);

    int stillFreeMB = (int)(sceKernelTotalFreeMemSize() / (1024u * 1024u));
    g_lowMemHeap = (n < 32);
    g_lowMemPsp  = (n + stillFreeMB) < 32;
}

static volatile int g_exitRequested = 0;

static volatile int g_powerResumed = 0;

static unsigned int g_powerResumes = 0;

volatile int g_powerSuspended = 0;
static volatile int g_powerStickWait = 0;

static int powerCallback(int , int pwrflags, void* ) {
    if (pwrflags & (PSP_POWER_CB_SUSPENDING | PSP_POWER_CB_STANDBY))
        { g_powerSuspended = 1; g_powerStickWait = 0; }
    if (pwrflags & PSP_POWER_CB_RESUME_COMPLETE)
        g_powerResumed = 1;
    return 0;
}

static int exitCallback(int , int , void* ) {
    g_exitRequested = 1;
    return 0;
}

static int callbackThread(SceSize , void* ) {
    int cbid = sceKernelCreateCallback("Exit Callback", exitCallback, 0);
    sceKernelRegisterExitCallback(cbid);

    int pcb = sceKernelCreateCallback("Power Callback", powerCallback, 0);

    if (pcb >= 0) scePowerRegisterCallback(-1, pcb);
    sceKernelSleepThreadCB();
    return 0;
}

static void setupCallbacks(void) {
    int thid = sceKernelCreateThread("update_thread", callbackThread,
                                     0x11, 0xFA0, 0, 0);
    if (thid >= 0)
        sceKernelStartThread(thid, 0, 0);
}

static bool loadTex(Texture* out, const char* rel) {
    return textureLoad(assetPath(rel), out) || textureLoad(rel, out);
}

static bool loadTex16(Texture* out, const char* rel, int psm) {
    return textureLoad16(assetPath(rel), out, psm) || textureLoad16(rel, out, psm);
}

static bool loadTexVram(Texture* out, const char* rel, int psm) {
    return textureLoadVram(assetPath(rel), out, psm) || textureLoadVram(rel, out, psm);
}

static bool screenNeedsTouchGui(int screen, bool worldLoaded) {
    if (screen == SCREEN_OPTIONS) return !worldLoaded;
    return screen == SCREEN_TITLE  || screen == SCREEN_WORLDS ||
           screen == SCREEN_DELETE || screen == SCREEN_CREATE;
}

static void touchGuiSetLoaded(MenuState& s, bool want) {
    if (want == s.haveTouch) return;
    if (want) {

        textureForgetFailures();
        s.haveTouch = loadTex(&s.touchGui, "data/images/gui/touchgui.png");
    }
    else { textureFree(&s.touchGui); s.haveTouch = false; }
}

static bool loadFnt(Font* out, const char* rel) {
    return fontLoad(assetPath(rel), out) || fontLoad(rel, out);
}

World g_world;
bool  g_worldBuilt = false;

#include "world/level/level.h"
Level g_level(&g_world);

#include "world/difficulty.h"
int g_difficulty = Difficulty::NORMAL;

#include "world/item/item.h"

int main(int argc, char* argv[]) {
    Item::initItems();
    Tile::initTiles();
    scePowerSetClockFrequency(333, 333, 166);

    pspFpuSetEnable(0);
    setupCallbacks();

    powerHoldInit();
    pathInit(argc > 0 ? argv[0] : 0);
    savePathInit();

    detectLowMemPsp();
    detectPspGo();
    soundInit();
    optionsLoad();

    sceCtrlSetSamplingCycle(0);
    sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);

    guInit();

    static const float INTRO_WHITE    = 0.9f;
    static const float INTRO_FADE_IN  = 0.7f;
    static const float INTRO_HOLD     = 1.1f;
    static const float INTRO_FADE_OUT = 0.7f;
    static const float INTRO_LOGO_UP  = INTRO_WHITE + INTRO_FADE_IN;
    static const float INTRO_DARK_AT  = INTRO_LOGO_UP + INTRO_HOLD;
    static const float INTRO_END      = INTRO_DARK_AT + INTRO_FADE_OUT;

    Texture mojangSplash;
    if (loadTex(&mojangSplash, "data/images/logo.png")) {
        const float startTime = nowSeconds();
        float skipAt = -1.0f;
        while (!g_exitRequested) {
            float t = nowSeconds() - startTime;

            if (skipAt >= 0.0f) t = INTRO_DARK_AT + (nowSeconds() - skipAt);
            if (t >= INTRO_END) break;

            const unsigned int ground = 0xFFFFFFFFu;
            float dark = 0.0f;
            if (t > INTRO_DARK_AT) {
                dark = (t - INTRO_DARK_AT) / INTRO_FADE_OUT;
                if (dark > 1.0f) dark = 1.0f;
            }

            scePowerTick(0);
            if (!guStartFrame(ground)) continue;
            guOrtho();
            sceGuDisable(GU_DEPTH_TEST);

            sceGuEnable(GU_BLEND);
            sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0);

            float scaleW = 480.0f / mojangSplash.realW;
            float scaleH = 272.0f / mojangSplash.realH;
            float scale  = (scaleW < scaleH) ? scaleW : scaleH;
            float w = mojangSplash.realW * scale, h = mojangSplash.realH * scale;
            float x = (480.0f - w) / 2.0f,        y = (272.0f - h) / 2.0f;

            float a = 0.0f;
            if (t >= INTRO_WHITE)   a = (t - INTRO_WHITE) / INTRO_FADE_IN;
            if (a > 1.0f)           a = 1.0f;
            a *= (1.0f - dark);
            if (a > 0.0f) {
                const unsigned int al = (unsigned int)(255.0f * a);
                textureBind(&mojangSplash);
                spriteDraw(&mojangSplash, x, y, w, h,
                           0, 0, mojangSplash.realW, mojangSplash.realH,
                           (al << 24) | 0x00FFFFFFu);
            }
            guEndFrame();

            panoramaSetLoaded(true);

            if (skipAt < 0.0f && t >= INTRO_WHITE) {
                SceCtrlData splashPad;
                sceCtrlReadBufferPositive(&splashPad, 1);
                if (splashPad.Buttons & PSP_CTRL_CROSS) skipAt = nowSeconds();
            }
        }
        textureFree(&mojangSplash);
    }

    static MenuState s;

    s.haveFont         = loadFnt(&s.font, "data/images/font/default8.png");

    s.haveGui          = loadTexVram(&s.guiAtlas, "data/images/gui/gui_game.png", GU_PSM_4444);

    g_btnIconsHave     = loadTexVram(&g_btnIcons, "data/images/gui/tooltips.png", GU_PSM_5551);

    extern bool g_haveGuiBlocks;
    extern Texture g_guiBlocks;
    if (!g_haveGuiBlocks)
        g_haveGuiBlocks = loadTexVram(&g_guiBlocks, "data/images/gui/gui_blocks.png", GU_PSM_5551);
    s.haveLogo         = loadTex16(&s.logo, "data/images/gui/title.png", GU_PSM_5551);
    s.haveBg           = loadTex(&s.dirtBg, "data/images/gui/background.png");
    s.haveTouch        = loadTex(&s.touchGui, "data/images/gui/touchgui.png");

    loadCharIfNeeded();

    s.screen = SCREEN_TITLE;
    worldListScan(&s.worlds);
    s.worldSelected = 0;
    s.deleteSelected = 1;
    createFormReset(s);
    s.uiRow = 1;
    s.topSelected = 0;
    s.listScroll = 0.0f;
    s.worldEditMode = 0;
    s.selected = 0;
    s.optFocus = 1;
    s.optCategory = 0;
    s.optTabHighlight = 0;
    s.optItemHighlight = 0;
    s.optScroll = 0.0f;
    s.statusMsg[0] = '\0';

    s.statusMsg[0] = '\0';

    SceCtrlData initPad;
    sceCtrlReadBufferPositive(&initPad, 1);
    unsigned int lastBtn = initPad.Buttons;

    float fps = 0.0f;
    float fpsLastTime = nowSeconds();
    int fpsFrames = 0;

    while (!g_exitRequested) {

        if (g_powerResumed) {
            g_powerResumed = 0;
            g_powerResumes++;

            guResumeFromSleep();
            chunkStorageDropOpenFiles();

            extern bool g_worldBuilt, g_paused;
            if (g_worldBuilt) g_paused = true;
            soundPowerResume();
            g_powerStickWait = 1;
            scePowerTick(0);
        }

        if (g_powerStickWait && g_powerStickWait++ < 300) {
            SceUID d = sceIoDopen(savePath(""));
            if (d >= 0) { sceIoDclose(d); g_powerStickWait = 300; }
        }
        if (g_powerStickWait >= 300) { g_powerStickWait = 0; g_powerSuspended = 0; }

        float now = nowSeconds();

        if (now - fpsLastTime >= 1.0f) {
            fps = fpsFrames / (now - fpsLastTime);
            fpsFrames = 0;
            fpsLastTime = now;
        }

        scePowerTick(0);

        SceCtrlData pad;
        sceCtrlPeekBufferPositive(&pad, 1);

        controlSchemeNotePad(pad.Buttons, pad.Rx, pad.Ry);

        const bool systemFrame = (pad.Buttons & (PSP_CTRL_HOME | PSP_CTRL_HOLD)) != 0;
        if (systemFrame) {
            pad.Buttons = 0;
            pad.Lx = pad.Ly = 128;
            pad.Rx = pad.Ry = 128;
        }

        pad.Buttons &= ~(PSP_CTRL_HOME | PSP_CTRL_HOLD |
                         PSP_CTRL_NOTE | PSP_CTRL_SCREEN |
                         PSP_CTRL_VOLUP | PSP_CTRL_VOLDOWN |
                         PSP_CTRL_WLAN_UP | PSP_CTRL_REMOTE |
                         PSP_CTRL_DISC | PSP_CTRL_MS);
        unsigned int currentBtn = pad.Buttons;

        extern bool g_invOpen, g_chestOpen, g_furnaceOpen, g_craftOpen, g_armorOpen;
        extern bool g_paused, g_optionsOpen;
        bool inGameMenu = g_invOpen || g_chestOpen || g_furnaceOpen || g_craftOpen ||
                          g_armorOpen || g_paused || g_optionsOpen;

        if (menuScreen(s.screen) || inGameMenu) {

            const int DZ = 48;
            if (pad.Lx < 128 - DZ) currentBtn |= PSP_CTRL_LEFT;
            if (pad.Lx > 128 + DZ) currentBtn |= PSP_CTRL_RIGHT;
            if (pad.Ly < 128 - DZ) currentBtn |= PSP_CTRL_UP;
            if (pad.Ly > 128 + DZ) currentBtn |= PSP_CTRL_DOWN;
        }

        unsigned int pressed = currentBtn & ~lastBtn;

        g_heldButtons = currentBtn;
        lastBtn = currentBtn;

        unsigned int repeat = 0;
        {
            static const unsigned int RDIRS = PSP_CTRL_UP | PSP_CTRL_DOWN | PSP_CTRL_LEFT | PSP_CTRL_RIGHT;
            static unsigned int s_lastDirs = 0;
            static unsigned int s_holdUs[4] = {0,0,0,0}, s_repUs[4] = {0,0,0,0};
            const unsigned int dirs[4] = {PSP_CTRL_UP, PSP_CTRL_DOWN, PSP_CTRL_LEFT, PSP_CTRL_RIGHT};
            unsigned int nowUs = sceKernelGetSystemTimeLow();
            for (int i = 0; i < 4; i++) {
                if (currentBtn & dirs[i]) {
                    if (!(s_lastDirs & dirs[i])) { s_holdUs[i] = nowUs; s_repUs[i] = nowUs; }
                    else if (nowUs - s_holdUs[i] >= 350000 && nowUs - s_repUs[i] >= 80000) {
                        repeat |= dirs[i]; s_repUs[i] = nowUs;
                    }
                }
            }
            s_lastDirs = currentBtn & RDIRS;
        }

        if (menuOskUpdate(s)) continue;

        if (pressed & PSP_CTRL_START) {
            if (s.screen == SCREEN_TITLE) {
                s.screen = SCREEN_WORLDS;
                s.statusMsg[0] = '\0';
            } else if (s.screen == SCREEN_WORLDS) {
                if (s.worldSelected < s.worlds.count) {
                    std::snprintf(s.statusMsg, sizeof(s.statusMsg), "Loading: %s", s.worlds.names[s.worldSelected]);
                    s.screen = SCREEN_GAME;
                }
            }

        }

        if (pressed & PSP_CTRL_SELECT) {
            if (s.screen == SCREEN_WORLDS) {
                createFormReset(s);
                s.screen = SCREEN_CREATE;
            }
        }

        const unsigned int NAV = PSP_CTRL_UP | PSP_CTRL_DOWN | PSP_CTRL_LEFT | PSP_CTRL_RIGHT
                               | PSP_CTRL_LTRIGGER | PSP_CTRL_RTRIGGER;
        bool navOnly = pressed && (pressed & ~NAV) == 0;
        unsigned int sigBefore = menuSelectionSig(s);
        AppScreen screenBefore = s.screen;

        unsigned int pMenu = pressed | repeat;
        if (Screen* cur = menuScreen(s.screen)) {

            cur->handleInput(s, menuFaceSwap(controlSchemeMenuAlias(pMenu)),
                             menuFaceSwap(controlSchemeMenuAlias(pad.Buttons)));
        } else {

            gameUpdate(s, inGameMenu ? pMenu
                       : (pressed | (repeat & (PSP_CTRL_LEFT | PSP_CTRL_RIGHT))), pad);
        }

        if (pressed && (screenBefore != SCREEN_GAME || g_optionsOpen) &&
            (!navOnly || menuSelectionSig(s) != sigBefore))
            soundPlay("random.click", 1.0f, 1.0f);

        touchGuiSetLoaded(s, screenNeedsTouchGui(s.screen, g_worldBuilt));

        panoramaSetLoaded(s.screen != SCREEN_GAME && !g_worldBuilt);

        if (!guStartFrame(s.screen == SCREEN_GAME ? g_clearColorNow : 0xFF000000u)) continue;
        fpsFrames++;

        if (s.screen == SCREEN_GAME) {
            gameRender(s);

            guOrtho();
            sceGuDisable(GU_DEPTH_TEST);

            if (gameProgressScreenUp()) { guEndFrame(); continue; }
            extern bool g_invOpen;
            extern int g_showFps, g_showCoords;
            extern int g_hideGui;
            if (!g_invOpen && !g_hideGui) {

                float ty = 10.0f;
                if (g_showFps) {
                    char fpsBuf[32];
                    std::snprintf(fpsBuf, sizeof(fpsBuf), "FPS: %d", (int)(fps + 0.5f));
                    fontDrawTextShadow(&s.font, 10, ty, fpsBuf, 0xFFE0E0E0u, 1.0f);
                    ty += 12.0f;
                }

                if (g_showCoords && g_worldBuilt && g_level.player) {
                    char posBuf[48];
                    std::snprintf(posBuf, sizeof(posBuf), "X %d  Y %d  Z %d",
                                  (int)floorf(g_level.player->x),
                                  (int)floorf(g_level.player->y - 1.62f),
                                  (int)floorf(g_level.player->z));
                    fontDrawTextShadow(&s.font, 10, ty, posBuf, 0xFFE0E0E0u, 1.0f);
                }
            }

            if (Screen* over = overlayScreen()) { over->render(s); }

            gameHintsDraw(s);

            sceGuEnable(GU_DEPTH_TEST);

            guEndFrame();
            continue;
        }

        if (Screen* cur = menuScreen(s.screen)) { cur->render(s); }
        menuHintsDraw(s);


        guEndFrame();
    }

    soundShutdown();

    if (g_worldBuilt) releaseWorldAndPlayer();
    worldGenWorkerStop();

    if (s.haveFont)  fontFree(&s.font);
    if (s.haveGui)   textureFree(&s.guiAtlas);
    if (g_btnIconsHave) textureFree(&g_btnIcons);
    if (s.haveLogo)  textureFree(&s.logo);
    if (s.haveBg) textureFree(&s.dirtBg);
    if (s.haveTouch) textureFree(&s.touchGui);
    panoramaSetLoaded(false);

    guTerm();
    sceKernelExitGame();
    return 0;
}
