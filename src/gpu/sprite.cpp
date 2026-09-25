#include "gpu/sprite.h"
#include "gpu/gu.h"

#include "gpu/texture.h"

#include <pspgu.h>

unsigned char g_uiAlpha = 255;

namespace {
struct TexVertex {
    float u, v;
    unsigned int color;
    float x, y, z;
};

inline float uvLo(float s, float d) { return s + (d < 0.0f ? -0.015625f : 0.015625f); }
inline float uvHi(float s, float d) { return s + d + (d < 0.0f ? 0.015625f : -0.015625f); }
}

void spriteDraw(const Texture* tex,
                float dx, float dy, float dw, float dh,
                float sx, float sy, float sw, float sh,
                unsigned int color) {
    (void)tex;

    TexVertex* v = (TexVertex*)guFrameAllocPriority(2 * sizeof(TexVertex));
    if (!v) return;

    const unsigned int c = uiAlphaApply(color);
    v[0].u = uvLo(sx, sw); v[0].v = uvLo(sy, sh);
    v[0].color = c;     v[0].x = dx;      v[0].y = dy;      v[0].z = 0.0f;
    v[1].u = uvHi(sx, sw); v[1].v = uvHi(sy, sh);
    v[1].color = c;     v[1].x = dx + dw; v[1].y = dy + dh; v[1].z = 0.0f;

    sceGuDrawArray(GU_SPRITES,
                   GU_TEXTURE_32BITF | GU_COLOR_8888 | GU_VERTEX_32BITF | GU_TRANSFORM_2D,
                   2, 0, v);
}

void spriteDrawRot(const Texture* tex,
                   float ox, float oy, float cs, float sn,
                   float lx, float ly, float lw, float lh,
                   float sx, float sy, float sw, float sh,
                   unsigned int color) {
    (void)tex;

    const float cx[4] = { lx,      lx + lw, lx + lw, lx      };
    const float cy[4] = { ly,      ly,      ly + lh, ly + lh };
    const float u0 = uvLo(sx, sw), u1 = uvHi(sx, sw);
    const float v0 = uvLo(sy, sh), v1 = uvHi(sy, sh);
    const float cu[4] = { u0, u1, u1, u0 };
    const float cv[4] = { v0, v0, v1, v1 };

    TexVertex* v = (TexVertex*)guFrameAllocPriority(4 * sizeof(TexVertex));
    if (!v) return;
    const unsigned int c = uiAlphaApply(color);
    for (int i = 0; i < 4; i++) {
        v[i].u = cu[i]; v[i].v = cv[i];
        v[i].color = c;
        v[i].x = ox + cx[i] * cs - cy[i] * sn;
        v[i].y = oy + cx[i] * sn + cy[i] * cs;
        v[i].z = 0.0f;
    }
    sceGuDrawArray(GU_TRIANGLE_FAN,
                   GU_TEXTURE_32BITF | GU_COLOR_8888 | GU_VERTEX_32BITF | GU_TRANSFORM_2D,
                   4, 0, v);
}

void spriteDrawFull(const Texture* tex, float dx, float dy, unsigned int color) {
    spriteDraw(tex, dx, dy, (float)tex->realW, (float)tex->realH,
               0.0f, 0.0f, (float)tex->realW, (float)tex->realH, color);
}

void spriteDrawTiled(const Texture* tex,
                     float dx, float dy, float dw, float dh,
                     float tileScreenPx, unsigned int color) {
    sceGuTexWrap(GU_REPEAT, GU_REPEAT);

    float sw = dw * ((float)tex->realW / tileScreenPx);
    float sh = dh * ((float)tex->realH / tileScreenPx);
    spriteDraw(tex, dx, dy, dw, dh, 0.0f, 0.0f, sw, sh, color);
    sceGuTexWrap(GU_CLAMP, GU_CLAMP);
}
