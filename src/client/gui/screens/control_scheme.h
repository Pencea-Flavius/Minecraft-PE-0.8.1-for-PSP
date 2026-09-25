
#ifndef MCPSP_CLIENT_GUI_SCREENS_CONTROL_SCHEME_H
#define MCPSP_CLIENT_GUI_SCREENS_CONTROL_SCHEME_H

#include "client/gui/screens/menu.h"

#define CONTROL_SCHEMES 4

extern int g_controlScheme;

#define ACT_DROP        0x04000000u
#define ACT_INVENTORY   0x08000000u
#define ACT_CRAFT       0x10000000u
#define ACT_THIRDPERSON 0x20000000u

void controlSchemeNotePad(unsigned int buttons, unsigned char rx, unsigned char ry);
bool controlSchemeAvailable(int scheme);

bool controlSchemeIsPadLayout();

unsigned int controlSchemeMenuAlias(unsigned int buttons);

extern int g_japaneseLayout;
unsigned int menuFaceSwap(unsigned int buttons);

unsigned int controlSchemeRemap(unsigned int buttons);

unsigned int controlSchemeButtonFor(unsigned int logical);

void controlSchemeCombos(unsigned int& pressed, unsigned int& held);

bool controlsPageIsOpen();

bool controlsPageEditing();
void controlsPageOpen();
void controlsPageRender(MenuState& s);
void controlsPageInput(MenuState& s, unsigned int pressed);

#endif
