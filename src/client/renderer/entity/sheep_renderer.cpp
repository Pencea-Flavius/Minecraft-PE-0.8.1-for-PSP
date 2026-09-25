#include "client/renderer/entity/sheep_renderer.h"
#include "platform/dcache.h"
#include "client/renderer/entity/mob_model.h"
#include "world/entity/animal/sheep.h"
#include "gpu/texture.h"
#include <math.h>
#include <stdio.h>
#include <pspgu.h>

static const float DEG2RAD = 3.14159265f / 180.0f;

enum { P_HEAD, P_BODY, P_LEG0, P_LEG1, P_LEG2, P_LEG3, P_HEAD_FUR, P_LEG_FUR0, P_COUNT = P_LEG_FUR0 + 4 };
enum { SHEARED_COUNT = P_LEG3 + 1 };

static MobPart furred[P_COUNT], sheared[SHEARED_COUNT];
static bool    g_built = false;
static Texture g_tex[16];
static signed char g_texState[16];

static void setPiv(MobPart& p, float x, float y, float z) { p.px = x; p.py = y; p.pz = z; }

static void buildModel(MobPart* p, bool isSheared) {
    const float T = 64.0f;
    mobBuildBox(p[P_HEAD].base, -3,-4,-6, 3,2,2, 0,0, 6,6,8, false, 0, T, T);
    setPiv(p[P_HEAD], 0,6,-8);
    if (isSheared) mobBuildBox(p[P_BODY].base, -4,-10,-7, 4,6,-1, 28,8, 8,16,6, false, 0, T, T);
    else           mobBuildBox(p[P_BODY].base, -4,-10,-7, 4,6,-1, 28,40, 8,16,6, false, 1.75f, T, T);
    setPiv(p[P_BODY], 0,5,2);
    const float lp[4][3] = { {-3,12,7}, {3,12,7}, {-3,12,-5}, {3,12,-5} };
    for (int i = 0; i < 4; i++) {
        mobBuildBox(p[P_LEG0+i].base, -2,0,-2, 2,12,2, 0,16, 4,12,4, false, 0, T, T);
        setPiv(p[P_LEG0+i], lp[i][0], lp[i][1], lp[i][2]);
    }
    p[P_HEAD].head = true;
    if (isSheared) return;
    mobBuildBox(p[P_HEAD_FUR].base, -3,-4,-4, 3,2,2, 0,32, 6,6,6, false, 0.6f, T, T);
    setPiv(p[P_HEAD_FUR], 0,6,-8);
    p[P_HEAD_FUR].head = true;
    for (int i = 0; i < 4; i++) {
        mobBuildBox(p[P_LEG_FUR0+i].base, -2,0,-2, 2,6,2, 0,48, 4,6,4, false, 0.5f, T, T);
        setPiv(p[P_LEG_FUR0+i], lp[i][0], lp[i][1], lp[i][2]);
    }
}

static void build() {
    if (g_built) return;
    buildModel(furred, false);
    buildModel(sheared, true);
    dcacheFlush(furred, sizeof(furred));
    dcacheFlush(sheared, sizeof(sheared));
    g_built = true;
}

static Texture* sheepTexture(int color) {
    color &= 15;
    if (!g_texState[color]) {
        char path[48];
        snprintf(path, sizeof(path), "data/images/mob/sheep_%d.png", color);
        g_texState[color] = textureLoad16(path, &g_tex[color], GU_PSM_5551) ? 1 : -1;
    }
    return g_texState[color] > 0 ? &g_tex[color] : 0;
}

SheepRenderer::SheepRenderer() { shadowRadius = 0.7f; shadowStrength = 1.0f; }

static void setAnim(MobPart* p, int count, float hx, float hy, float pend, float headY) {
    p[P_HEAD].xRot = hx; p[P_HEAD].yRot = hy; p[P_HEAD].zRot = 0; p[P_HEAD].py = headY;
    p[P_BODY].xRot = 90.0f * DEG2RAD; p[P_BODY].yRot = p[P_BODY].zRot = 0;
    p[P_LEG0].xRot = pend; p[P_LEG1].xRot = -pend; p[P_LEG2].xRot = -pend; p[P_LEG3].xRot = pend;
    for (int i = P_LEG0; i <= P_LEG3; i++) { p[i].yRot = p[i].zRot = 0; }
    if (count <= SHEARED_COUNT) return;
    p[P_HEAD_FUR].xRot = p[P_HEAD].xRot; p[P_HEAD_FUR].yRot = hy; p[P_HEAD_FUR].zRot = 0;
    p[P_HEAD_FUR].py = headY;
    for (int i = 0; i < 4; i++) {
        p[P_LEG_FUR0+i].xRot = p[P_LEG0+i].xRot; p[P_LEG_FUR0+i].yRot = p[P_LEG_FUR0+i].zRot = 0;
    }
}

void SheepRenderer::render(Entity* e, float x, float y, float z, float rot, float a) {
    Sheep* sheep = (Sheep*)e;
    Mob* mob = (Mob*)e;
    Texture* tex = sheepTexture(sheep->getColor());
    if (!tex) return;
    build();

    MobAnim m = mobAnimSetup(mob, rot, a);
    float pend = cosf(m.pos * 0.6662f) * 1.4f * m.speed;
    float hx = -m.pitch * DEG2RAD, hy = m.headYaw * DEG2RAD;
    float headY = 6.0f;
    if (sheep->getEatAnimationTick() > 0) {
        headY = 6.0f + sheep->getHeadEatPositionScale(a) * 9.0f;
        hx = sheep->getHeadEatAngleScale(a);
    }

    const bool bare = sheep->isSheared();
    MobPart* parts = bare ? sheared : furred;
    const int count = bare ? (int)SHEARED_COUNT : (int)P_COUNT;
    setAnim(parts, count, hx, hy, pend, headY);
    mobRenderParts(mob, parts, count, tex, x, y, z, m.bodyRot, a, 0xFFFFFFFFu, 8.0f, 4.0f);
}
