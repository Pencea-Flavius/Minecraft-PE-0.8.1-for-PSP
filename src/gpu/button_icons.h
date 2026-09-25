
#ifndef MCPSP_GPU_BUTTON_ICONS_H
#define MCPSP_GPU_BUTTON_ICONS_H

#include "gpu/sprite.h"
#include "gpu/texture.h"

extern Texture g_btnIcons;
extern bool    g_btnIconsHave;

enum ButtonIcon {
    BTN_ICON_L, BTN_ICON_R,
    BTN_ICON_UP, BTN_ICON_DOWN, BTN_ICON_LEFT, BTN_ICON_RIGHT,
    BTN_ICON_TRIANGLE, BTN_ICON_CIRCLE, BTN_ICON_SQUARE, BTN_ICON_CROSS,
    BTN_ICON_START, BTN_ICON_SELECT,
    BTN_ICON_L1, BTN_ICON_R1, BTN_ICON_L2, BTN_ICON_R2,
    BTN_ICON_L3, BTN_ICON_R3,

    BTN_ICON_PS_START, BTN_ICON_PS_SELECT,
    BTN_ICON_DPAD,
    BTN_ICON_COUNT
};

struct ButtonIconRect { float x, y, w, h, pressedDx, pressedDy; };

inline ButtonIconRect buttonIconRect(ButtonIcon i, bool pressed = false) {

    static const ButtonIconRect kRects[BTN_ICON_COUNT] = {
        {  0.0f,  80.0f, 29.0f, 13.0f, 0.0f, 16.0f },
        { 32.0f,  80.0f, 29.0f, 13.0f, 0.0f, 16.0f },

        { 32.0f,  64.0f, 15.0f, 15.0f,  0.0f, 0.0f },
        {  0.0f,  64.0f, 15.0f, 15.0f,  0.0f, 0.0f },
        { 16.0f,  64.0f, 15.0f, 15.0f,  0.0f, 0.0f },
        { 48.0f,  64.0f, 15.0f, 15.0f,  0.0f, 0.0f },
        { 16.0f,  32.0f, 15.0f, 15.0f, 0.0f, 16.0f },
        { 16.0f,   0.0f, 15.0f, 15.0f, 0.0f, 16.0f },
        {  0.0f,  32.0f, 15.0f, 15.0f, 0.0f, 16.0f },
        {  0.0f,   0.0f, 15.0f, 15.0f, 0.0f, 16.0f },
        { 32.0f,  33.0f, 31.0f, 13.0f, 0.0f, 16.0f },
        { 32.0f,   1.0f, 31.0f, 13.0f, 0.0f, 16.0f },
        { 64.0f,  96.0f, 29.0f, 13.0f, 0.0f, 16.0f },
        { 96.0f,  96.0f, 29.0f, 13.0f, 0.0f, 16.0f },
        { 64.0f,  64.0f, 29.0f, 15.0f, 0.0f, 16.0f },
        { 96.0f,  64.0f, 29.0f, 15.0f, 0.0f, 16.0f },
        { 98.0f,  33.0f, 13.0f, 15.0f,  0.0f, 0.0f },
        { 114.0f, 33.0f, 13.0f, 15.0f,  0.0f, 0.0f },

        {  2.0f, 116.0f, 12.0f,  8.0f, 32.0f, 0.0f },
        { 18.0f, 116.0f, 12.0f,  8.0f, 32.0f, 0.0f },
        { 96.0f,  48.0f, 15.0f, 15.0f,  0.0f, 0.0f },
    };
    ButtonIconRect r = kRects[i];
    if (pressed) { r.x += r.pressedDx; r.y += r.pressedDy; }
    return r;
}

inline void buttonIconDraw(ButtonIcon i, float x, float y, bool pressed = false,
                           float scale = 1.0f, unsigned int color = 0xFFFFFFFF) {
    if (!g_btnIconsHave) return;
    const ButtonIconRect r = buttonIconRect(i, pressed);
    textureBind(&g_btnIcons);
    spriteDraw(&g_btnIcons, x, y, r.w * scale, r.h * scale, r.x, r.y, r.w, r.h, color);
}

#endif
