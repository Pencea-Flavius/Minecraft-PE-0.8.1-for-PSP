
#ifndef MCPSP_GPU_UI_ALPHA_H
#define MCPSP_GPU_UI_ALPHA_H

extern unsigned char g_uiAlpha;

static inline unsigned int uiAlphaApply(unsigned int abgr) {
    if (g_uiAlpha == 255) return abgr;
    unsigned int a = (abgr >> 24) * (unsigned int)g_uiAlpha / 255u;
    return (abgr & 0x00FFFFFFu) | (a << 24);
}

static inline float hudOpacityCurve(float base, float since) {
    if (base >= 0.8f) return base;
    const float HOLD = 5.5f;
    const float FADE = 0.5f;
    if (since <= HOLD) return 0.8f;
    if (since >= HOLD + FADE) return base;
    return 0.8f + (base - 0.8f) * ((since - HOLD) / FADE);
}

#endif
