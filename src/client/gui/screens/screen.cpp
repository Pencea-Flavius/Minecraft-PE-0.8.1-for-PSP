#include "client/gui/screens/screen.h"
#include "client/gui/screens/panorama.h"
#include "client/player/player.h"
#include "world/level/level.h"
#include "world/entity/local_player.h"
#include "gpu/gu.h"

#include <pspgu.h>

extern bool g_worldBuilt;
extern bool g_craftOpen, g_armorOpen, g_furnaceOpen, g_chestOpen, g_deadScreen;

void Screen::render(MenuState& s) {
    renderBackground(s);

    sceGuDisable(GU_DEPTH_TEST);
    renderContent(s);
}

void Screen::renderBackground(MenuState& s) {
    sceGuDisable(GU_DEPTH_TEST);
    if (g_worldBuilt)           drawRect(0.0f, 0.0f, 480.0f, 272.0f, 0x7F000000u);
    else if (!panoramaRender()) drawDirtBackground(s);
    sceGuEnable(GU_DEPTH_TEST);
}

void ContainerScreen::renderBackground(MenuState& s) {
    sceGuDisable(GU_DEPTH_TEST);
    drawWindowFrame(s);
}

Screen* menuScreen(AppScreen which) {
    switch (which) {
        case SCREEN_TITLE:   return &titleScreen();
        case SCREEN_WORLDS:  return &worldsScreen();
        case SCREEN_DELETE:  return &deleteScreen();
        case SCREEN_CREATE:  return &createScreen();
        case SCREEN_OPTIONS: return &optionsScreen();
        case SCREEN_GAME:    break;
    }
    return 0;
}

Screen* overlayScreen() {

    if (g_signEditing)  return &signScreen();
    if (g_deadScreen)   return &deadScreen();
    if (g_paused)       return &pauseScreen();
    if (g_optionsOpen)  return &optionsScreen();
    if (g_worldBuilt && g_level.player && g_level.player->isSleeping())
        return &inBedScreen();
    if (g_craftOpen)    return &craftScreen();
    if (g_armorOpen)    return &armorScreen();
    if (g_furnaceOpen)  return &furnaceScreen();
    if (g_chestOpen)    return &chestScreen();
    return 0;
}
