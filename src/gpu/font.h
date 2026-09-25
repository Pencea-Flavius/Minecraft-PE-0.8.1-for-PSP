
#ifndef MCPSP_GPU_FONT_H
#define MCPSP_GPU_FONT_H

#include "gpu/texture.h"

struct Font {
    Texture tex;
    unsigned char charWidth[256];

    unsigned char charBottom[256];
    int lineHeight;
};

int fontTextInkRows(const Font* f, const char* text);

bool fontLoad(const char* path, Font* out);

void fontFree(Font* f);

inline float fontSnapScale(float s) {
    int n = (int)(s + 0.5f);
    return n < 1 ? 1.0f : (float)n;
}

void fontDrawText(const Font* f, float x, float y, const char* text, unsigned int color, float scale = 1.0f);

void fontDrawTextShadow(const Font* f, float x, float y, const char* text, unsigned int color, float scale = 1.0f);

void fontDrawTransformed(const Font* f, float x, float y, const char* text,
                         unsigned int color, float rotDeg, float scale,
                         bool centered);

int fontTextWidth(const Font* f, const char* text);

#endif
