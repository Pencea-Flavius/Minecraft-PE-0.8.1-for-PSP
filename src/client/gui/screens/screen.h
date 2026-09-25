
#ifndef MCPSP_CLIENT_GUI_SCREENS_SCREEN_H
#define MCPSP_CLIENT_GUI_SCREENS_SCREEN_H

#include "client/gui/screens/menu.h"

class Screen {
public:
    virtual ~Screen() {}

    virtual void render(MenuState& s);

    virtual void renderBackground(MenuState& s);

    virtual void renderContent(MenuState& s) = 0;
    virtual void handleInput(MenuState& s, unsigned int pressed, unsigned int held) = 0;
};

class ContainerScreen : public Screen {
public:
    virtual void renderBackground(MenuState& s);
};

Screen& titleScreen();
Screen& worldsScreen();
Screen& deleteScreen();
Screen& createScreen();

void createFormReset(MenuState& s);

Screen& optionsScreen();
Screen& pauseScreen();
Screen& craftScreen();
Screen& furnaceScreen();
Screen& chestScreen();
Screen& armorScreen();
Screen& inBedScreen();
Screen& deadScreen();
Screen& signScreen();

Screen* menuScreen(AppScreen which);
Screen* overlayScreen();

#endif
