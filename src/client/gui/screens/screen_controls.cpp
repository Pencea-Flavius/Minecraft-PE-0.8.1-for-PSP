
#include <pspctrl.h>
#include <pspgu.h>
#include <cstdio>
#include <cmath>
#include <cstring>

#include "client/gui/screens/screen.h"
#include "client/gui/screens/control_scheme.h"
#include "gpu/gui_atlas.h"
#include "gpu/sprite.h"
#include "platform/path.h"

#define CONTROLS_ANCHOR_EDITOR 0

extern int g_invertY;
extern int g_pspIsGo;
extern int g_southpaw;
extern int g_fineAim;

int g_controlScheme = 0;

static bool s_open = false;
bool controlsPageIsOpen() { return s_open; }

namespace {

enum {
    CA_USE, CA_ATTACK, CA_JUMP, CA_SNEAK, CA_HOTBAR_UP,
    CA_PREV, CA_NEXT,
    CA_LOOK_L, CA_LOOK_R, CA_LOOK_U, CA_LOOK_D,

    CA_DROP, CA_INVENTORY, CA_CRAFT, CA_THIRD,
    CA_COUNT
};

const unsigned int kLogical[CA_COUNT] = {
    PSP_CTRL_LTRIGGER, PSP_CTRL_RTRIGGER, PSP_CTRL_START, PSP_CTRL_DOWN, PSP_CTRL_UP,
    PSP_CTRL_LEFT, PSP_CTRL_RIGHT,
    PSP_CTRL_SQUARE, PSP_CTRL_CIRCLE, PSP_CTRL_TRIANGLE, PSP_CTRL_CROSS,
    ACT_DROP, ACT_INVENTORY, ACT_CRAFT, ACT_THIRDPERSON,
};

const unsigned int kPhys[CONTROL_SCHEMES][CA_COUNT] = {

    { PSP_CTRL_LTRIGGER, PSP_CTRL_RTRIGGER, PSP_CTRL_START, PSP_CTRL_DOWN, PSP_CTRL_UP,
      PSP_CTRL_LEFT, PSP_CTRL_RIGHT,
      PSP_CTRL_SQUARE, PSP_CTRL_CIRCLE, PSP_CTRL_TRIANGLE, PSP_CTRL_CROSS,
      0, 0, 0, 0 },

    { PSP_CTRL_LTRIGGER, PSP_CTRL_RTRIGGER, PSP_CTRL_UP, PSP_CTRL_DOWN, PSP_CTRL_START,
      PSP_CTRL_LEFT, PSP_CTRL_RIGHT,
      PSP_CTRL_SQUARE, PSP_CTRL_CIRCLE, PSP_CTRL_TRIANGLE, PSP_CTRL_CROSS,
      0, 0, 0, 0 },

    { PSP_CTRL_TRIANGLE, PSP_CTRL_SQUARE, PSP_CTRL_CROSS, PSP_CTRL_CIRCLE, PSP_CTRL_START,
      PSP_CTRL_LTRIGGER, PSP_CTRL_RTRIGGER,
      PSP_CTRL_LEFT, PSP_CTRL_RIGHT, PSP_CTRL_UP, PSP_CTRL_DOWN,
      0, 0, 0, 0 },

    { PSP_CTRL_LTRIGGER, PSP_CTRL_RTRIGGER, PSP_CTRL_CROSS, PSP_CTRL_R3, 0,
      PSP_CTRL_L1, PSP_CTRL_R1,
      0, 0, 0, 0,
      PSP_CTRL_CIRCLE, PSP_CTRL_TRIANGLE, PSP_CTRL_SQUARE, PSP_CTRL_L3 },
};

const bool kComboInv[CONTROL_SCHEMES] = { false, true, true, false };

bool s_haveController = false;

const float CTL_TEXT      = 1.0f;

const float CTL_PANEL_W   = 172.0f;
const float CTL_PANEL_X   = (VW - CTL_PANEL_W) * 0.5f;
const float CTL_PANEL_Y   = 21.0f;
const float CTL_PANEL_H   = 30.0f;

const float CTL_CAPTION_Y = CTL_PANEL_Y +  3.0f;
const float CTL_BTN_W     = 16.0f;
const float CTL_BTN_H     =  9.0f;
const float CTL_BTN_PITCH = 19.0f;
const float CTL_BTN_Y     = CTL_PANEL_Y +  9.0f;

const float CTL_SW_W      = TOGGLE_CELL_W * 0.5f;
const float CTL_SW_H      = TOGGLE_CELL_H * 0.5f;
const float CTL_CHECK_Y   = CTL_PANEL_Y + 20.0f;
const float CTL_CHECK_GAP = 3.0f;

Texture s_art;
bool    s_artHave = false;
int     s_artKey  = -1;

#define ART_KEY_PSP 0
#define ART_KEY_GO  1
#define ART_KEY_DS3 2

int artKey(int layout) {
    if (layout == CONTROL_SCHEMES - 1) return ART_KEY_DS3;
    return g_pspIsGo ? ART_KEY_GO : ART_KEY_PSP;
}

void artFree() {
    if (s_artHave) { textureFree(&s_art); s_artHave = false; }
    s_artKey = -1;
}

void artEnsure(int layout) {
    const int key = artKey(layout);
    if (s_artKey == key) return;
    artFree();
    const char* name = (key == ART_KEY_DS3) ? "ds3" : (key == ART_KEY_GO) ? "go" : "psp";
    char rel[64];
    snprintf(rel, sizeof(rel), "data/images/gui/controls/%s.png", name);

    s_artHave = textureLoad16Optional(assetPath(rel), &s_art, GU_PSM_5551);

    if (!s_artHave && key == ART_KEY_GO) {
        s_artHave = textureLoad16Optional(assetPath("data/images/gui/controls/psp.png"),
                                          &s_art, GU_PSM_5551);
        if (s_artHave) { s_artKey = ART_KEY_PSP; return; }
    }
    s_artKey  = key;
}

const unsigned int LCE_LINE_COL = 0xFF0FCCEBu;

struct Pseudo { const char* name; float x, y; float box; };

struct Anchor { unsigned int phys; float x, y; };

Anchor kAnchorsDs3[] = {
    { PSP_CTRL_LTRIGGER, 20.5f,  4.0f },
    { PSP_CTRL_RTRIGGER, 68.5f,  4.0f },
    { PSP_CTRL_L1,       19.0f,  8.5f },
    { PSP_CTRL_R1,       70.0f,  8.5f },
    { PSP_CTRL_TRIANGLE, 70.5f, 23.5f },
    { PSP_CTRL_SQUARE,   63.5f, 30.5f },
    { PSP_CTRL_CIRCLE,   77.5f, 30.5f },
    { PSP_CTRL_CROSS,    70.5f, 37.5f },

    { PSP_CTRL_L3,       32.5f, 48.5f },
    { PSP_CTRL_R3,       56.5f, 48.5f },
    { PSP_CTRL_UP,       18.5f, 30.5f },
};Pseudo kPseudoDs3[] = {
    { "Move", 32.5f, 44.5f, 0.0f },
    { "Look", 56.5f, 44.5f, 0.0f },

    { "Pause", 38.0f, 28.5f, 0.0f },
};

Anchor kAnchorsPsp[] = {
    { PSP_CTRL_LTRIGGER, 11.0f,  6.0f },
    { PSP_CTRL_RTRIGGER, 70.0f,  6.0f },
    { PSP_CTRL_UP,       11.5f, 13.5f },
    { PSP_CTRL_LEFT,      7.5f, 17.5f },
    { PSP_CTRL_RIGHT,    15.5f, 17.5f },
    { PSP_CTRL_DOWN,     11.5f, 21.5f },

    { PSP_CTRL_TRIANGLE, 69.5f, 13.5f },
    { PSP_CTRL_SQUARE,   65.5f, 17.5f },
    { PSP_CTRL_CIRCLE,   73.5f, 17.5f },
    { PSP_CTRL_CROSS,    69.5f, 21.5f },
    { PSP_CTRL_START,    61.0f, 34.0f },
};Pseudo kPseudoPsp[] = {
    { "Move", 12.0f, 28.5f, 0.0f },

    { "Inventory", 50.0f, 25.0f, 4.0f },

    { "Pause", 56.0f, 34.0f, 0.0f },
};

Anchor kAnchorsGo[] = {
    { PSP_CTRL_LTRIGGER,  0.5f, 38.0f },
    { PSP_CTRL_RTRIGGER, 78.0f, 38.0f },
    { PSP_CTRL_UP,       16.0f, 47.0f },
    { PSP_CTRL_LEFT,     12.0f, 51.5f },
    { PSP_CTRL_RIGHT,    20.5f, 51.5f },
    { PSP_CTRL_DOWN,     16.5f, 55.5f },
    { PSP_CTRL_TRIANGLE, 62.0f, 47.5f },
    { PSP_CTRL_SQUARE,   58.0f, 51.5f },
    { PSP_CTRL_CIRCLE,   66.5f, 51.5f },
    { PSP_CTRL_CROSS,    62.5f, 55.5f },

    { PSP_CTRL_START,    47.5f, 57.5f },
};
Pseudo kPseudoGo[] = {
    { "Move", 31.0f, 55.5f, 0.0f },
    { "Inventory", 46.5f, 29.0f, 4.0f },
    { "Pause", 47.5f, 53.5f, 0.0f },
};

const char* const kActionName[CA_COUNT] = {
    "Place", "Break", "Jump", "Sneak", "Drop",
    "Prev Item", "Next Item",
    "Look", "Look", "Look", "Look",
    "Drop", "Inventory", "Crafting", "Camera Mode",
};

struct ArtAnchors {
    Anchor* an; int anCount;
    Pseudo* ps; int psCount;
};
ArtAnchors anchorsForArt(int artKey) {
    ArtAnchors a = { 0, 0, 0, 0 };
    if (artKey == ART_KEY_DS3) {
        a.an = kAnchorsDs3; a.anCount = (int)(sizeof(kAnchorsDs3) / sizeof(kAnchorsDs3[0]));
        a.ps = kPseudoDs3;  a.psCount = (int)(sizeof(kPseudoDs3) / sizeof(kPseudoDs3[0]));
    } else if (artKey == ART_KEY_GO) {
        a.an = kAnchorsGo;  a.anCount = (int)(sizeof(kAnchorsGo) / sizeof(kAnchorsGo[0]));
        a.ps = kPseudoGo;   a.psCount = (int)(sizeof(kPseudoGo) / sizeof(kPseudoGo[0]));
    } else if (artKey == ART_KEY_PSP) {
        a.an = kAnchorsPsp; a.anCount = (int)(sizeof(kAnchorsPsp) / sizeof(kAnchorsPsp[0]));
        a.ps = kPseudoPsp;  a.psCount = (int)(sizeof(kPseudoPsp) / sizeof(kPseudoPsp[0]));
    }
    return a;
}

struct Lead { const char* text; float ax, ay; float box; float bx, by; };

#if CONTROLS_ANCHOR_EDITOR

bool  s_editing  = false;
int   s_editIdx  = 0;
float s_editStep = 0.5f;

struct ButtonName { unsigned int mask; const char* token; const char* label; };
const ButtonName kButtonNames[] = {
    { PSP_CTRL_LTRIGGER, "PSP_CTRL_LTRIGGER", "L / L2" },
    { PSP_CTRL_RTRIGGER, "PSP_CTRL_RTRIGGER", "R / R2" },
    { PSP_CTRL_L1,       "PSP_CTRL_L1",       "L1" },
    { PSP_CTRL_R1,       "PSP_CTRL_R1",       "R1" },
    { PSP_CTRL_L3,       "PSP_CTRL_L3",       "L3" },
    { PSP_CTRL_R3,       "PSP_CTRL_R3",       "R3" },
    { PSP_CTRL_UP,       "PSP_CTRL_UP",       "D-pad Up" },
    { PSP_CTRL_DOWN,     "PSP_CTRL_DOWN",     "D-pad Down" },
    { PSP_CTRL_LEFT,     "PSP_CTRL_LEFT",     "D-pad Left" },
    { PSP_CTRL_RIGHT,    "PSP_CTRL_RIGHT",    "D-pad Right" },
    { PSP_CTRL_TRIANGLE, "PSP_CTRL_TRIANGLE", "Triangle" },
    { PSP_CTRL_SQUARE,   "PSP_CTRL_SQUARE",   "Square" },
    { PSP_CTRL_CIRCLE,   "PSP_CTRL_CIRCLE",   "Circle" },
    { PSP_CTRL_CROSS,    "PSP_CTRL_CROSS",    "Cross" },
    { PSP_CTRL_START,    "PSP_CTRL_START",    "Start" },
};
const ButtonName* buttonName(unsigned int mask) {
    for (unsigned i = 0; i < sizeof(kButtonNames) / sizeof(kButtonNames[0]); i++)
        if (kButtonNames[i].mask == mask) return &kButtonNames[i];
    return 0;
}
#endif

const float CTL_ART_X = 20.0f;

const float CTL_ART_Y = 60.0f;
const float CTL_ART_W = 200.0f;
const float CTL_ART_H = 66.0f;

int collectLeads(int scheme, const ArtAnchors& a, Lead* out, int max) {
    int n = 0;
    int count[CA_COUNT + 4];
    for (int act = 0; act < CA_COUNT && n < max; act++) {
        unsigned int phys = kPhys[scheme][act];
        if (!phys) continue;
        const Anchor* hit = 0;
        for (int i = 0; i < a.anCount; i++)
            if (a.an[i].phys == phys) { hit = &a.an[i]; break; }
        if (!hit) continue;
        int at = -1;
        for (int i = 0; i < n; i++)
            if (out[i].text == kActionName[act]) { at = i; break; }
        if (at < 0) {
            out[n].text = kActionName[act]; out[n].ax = hit->x; out[n].ay = hit->y;
            out[n].box = 0.0f; out[n].bx = out[n].by = -1.0f; count[n] = 1; n++;
        } else {
            count[at]++;
            out[at].ax += (hit->x - out[at].ax) / (float)count[at];
            out[at].ay += (hit->y - out[at].ay) / (float)count[at];
        }
    }

    if (kComboInv[scheme] && n < max) {
        const Anchor* l = 0; const Anchor* r = 0;
        for (int i = 0; i < a.anCount; i++) {
            if (a.an[i].phys == PSP_CTRL_LTRIGGER) l = &a.an[i];
            if (a.an[i].phys == PSP_CTRL_RTRIGGER) r = &a.an[i];
        }
        if (l && r) {
            out[n].text = "Inventory"; out[n].box = 0.0f;
            out[n].ax = l->x; out[n].ay = l->y;
            out[n].bx = r->x; out[n].by = r->y;
            n++;
        }
    }

    for (int i = 0; i < a.psCount && n < max; i++) {
        bool taken = false;
        for (int j = 0; j < n && !taken; j++)
            if (strcmp(out[j].text, a.ps[i].name) == 0) taken = true;
        if (taken) continue;
        out[n].text = a.ps[i].name; out[n].ax = a.ps[i].x; out[n].ay = a.ps[i].y;
        out[n].box = a.ps[i].box; out[n].bx = out[n].by = -1.0f; n++;
    }

    if (g_southpaw) {
        Lead* look = 0; Lead* move = 0;
        for (int i = 0; i < n; i++) {
            if (!strcmp(out[i].text, "Look")) look = &out[i];
            if (!strcmp(out[i].text, "Move")) move = &out[i];
        }
        if (look && move) {
            const float x = look->ax, y = look->ay;
            look->ax = move->ax; look->ay = move->ay;
            move->ax = x;        move->ay = y;
        }
    }
    return n;
}

#if CONTROLS_ANCHOR_EDITOR

struct EditTarget { float* x; float* y; const char* label; };
int editCount(const ArtAnchors& a) { return a.anCount + a.psCount; }
EditTarget editTarget(const ArtAnchors& a, int idx) {
    EditTarget t = { 0, 0, "?" };
    if (idx >= 0 && idx < a.anCount) {
        t.x = &a.an[idx].x; t.y = &a.an[idx].y;
        const ButtonName* bn = buttonName(a.an[idx].phys);
        t.label = bn ? bn->label : "?";
    } else if (idx >= a.anCount && idx - a.anCount < a.psCount) {
        Pseudo& p = a.ps[idx - a.anCount];
        t.x = &p.x; t.y = &p.y; t.label = p.name;
    }
    return t;
}

int collectEditing(const ArtAnchors& a, Lead* out, int max) {
    int n = 0;
    for (int i = 0; i < a.anCount && n < max; i++) {
        const ButtonName* bn = buttonName(a.an[i].phys);
        out[n].text = bn ? bn->label : "?";
        out[n].ax = a.an[i].x; out[n].ay = a.an[i].y; out[n].box = 0.0f;
        out[n].bx = out[n].by = -1.0f; n++;
    }
    for (int i = 0; i < a.psCount && n < max; i++) {
        out[n].text = a.ps[i].name; out[n].ax = a.ps[i].x;
        out[n].ay = a.ps[i].y; out[n].box = a.ps[i].box;
        out[n].bx = out[n].by = -1.0f; n++;
    }
    return n;
}

void anchorsSave(int artKey) {
    const ArtAnchors a = anchorsForArt(artKey);
    if (!a.anCount) return;
    FILE* f = fopen(assetPath("controls_anchors.txt"), "w");
    if (!f) return;
    const char* which = (artKey == ART_KEY_DS3) ? "Ds3"
                      : (artKey == ART_KEY_GO)  ? "Go" : "Psp";
    fprintf(f, "// Written by the in-game anchor editor: Options > Controls > Open,\n"
               "// then START. Paste these over the tables in screen_controls.cpp.\n\n");
    fprintf(f, "Anchor kAnchors%s[] = {\n", which);
    for (int i = 0; i < a.anCount; i++) {
        const ButtonName* bn = buttonName(a.an[i].phys);
        char tok[32];
        snprintf(tok, sizeof(tok), "%s,", bn ? bn->token : "0");
        fprintf(f, "    { %-20s %5.1ff, %5.1ff },\n", tok, a.an[i].x, a.an[i].y);
    }
    fprintf(f, "};\n\nPseudo kPseudo%s[] = {\n", which);
    for (int i = 0; i < a.psCount; i++) {
        char tok[32];
        snprintf(tok, sizeof(tok), "\"%s\",", a.ps[i].name);
        fprintf(f, "    { %-14s %5.1ff, %5.1ff, %4.1ff },\n",
                tok, a.ps[i].x, a.ps[i].y, a.ps[i].box);
    }
    fprintf(f, "};\n");
    fclose(f);
}

void drawEditor(MenuState& s, float artX, float artY, float k) {
    if (!s_editing) return;
    const ArtAnchors a = anchorsForArt(s_artKey);
    EditTarget t = editTarget(a, s_editIdx);
    if (!t.x) return;
    const unsigned int CYAN = 0xFFFFFF00u;
    const float cx = floorf(artX + *t.x * k);
    const float cy = floorf(artY + *t.y * k);
    drawRect(cx - 5.0f, cy, 11.0f, 1.0f, CYAN);
    drawRect(cx, cy - 5.0f, 1.0f, 11.0f, CYAN);

    const char* art = (s_artKey == ART_KEY_DS3) ? "ds3"
                    : (s_artKey == ART_KEY_GO)  ? "go" : "psp";
    char line[80];
    snprintf(line, sizeof(line), "%s  %d/%d  %s  %.1f, %.1f  step %.1f",
             art, s_editIdx + 1, editCount(a), t.label, *t.x, *t.y, s_editStep);
    fontDrawTextShadow(&s.font, CTL_ART_X * UI_SCALE,
                       (CTL_ART_Y - 5.0f) * UI_SCALE, line, CYAN, CTL_TEXT);
}

#endif

void drawCombLead(MenuState& s, const Lead& L, float artX, float artY, float k) {
    const float ax = floorf(artX + L.ax * k), ay = floorf(artY + L.ay * k);
    const float bx = floorf(artX + L.bx * k), by = floorf(artY + L.by * k);
    const float ly = floorf(artY) - 4.0f;
    const float x0 = ax < bx ? ax : bx, x1 = ax < bx ? bx : ax;
    drawRect(x0, ly, x1 - x0 + 1.0f, 1.0f, LCE_LINE_COL);
    drawRect(ax, ly, 1.0f, ay - ly + 1.0f, LCE_LINE_COL);
    drawRect(bx, ly, 1.0f, by - ly + 1.0f, LCE_LINE_COL);

    drawRect(floorf((x0 + x1) * 0.5f), ly - 4.0f, 1.0f, 5.0f, LCE_LINE_COL);

    const float tw = fontTextWidth(&s.font, L.text) * CTL_TEXT;
    fontDrawTextShadow(&s.font, floorf((x0 + x1) * 0.5f - tw * 0.5f),
                       ly - 12.0f * CTL_TEXT, L.text, 0xFFFFFFFFu, CTL_TEXT);
}

void drawLeaders(MenuState& s, int scheme, int artKey,
                 float artX, float artY, float artW, float artH, float k) {
    const ArtAnchors a = anchorsForArt(artKey);
    if (!a.anCount) return;

    Lead all[CA_COUNT + 8];
    const int cap = CA_COUNT + 8;
#if CONTROLS_ANCHOR_EDITOR
    int n = s_editing ? collectEditing(a, all, cap)
                      : collectLeads(scheme, a, all, cap);
#else
    int n = collectLeads(scheme, a, all, cap);
#endif

    Lead col[2][CA_COUNT + 8];
    int cn[2] = { 0, 0 };
    for (int i = 0; i < n; i++) {

        if (all[i].bx >= 0.0f) { drawCombLead(s, all[i], artX, artY, k); continue; }
        int c = (all[i].ax * k < artW * 0.5f) ? 0 : 1;
        col[c][cn[c]++] = all[i];
    }

    const float boxY0 = CTL_ART_Y * UI_SCALE;
    const float boxH  = CTL_ART_H * UI_SCALE;
    const float lineX[2] = { artX - 3.0f, artX + artW + 3.0f };

    for (int c = 0; c < 2; c++) {

        for (int i = 1; i < cn[c]; i++)
            for (int j = i; j > 0 && col[c][j].ay < col[c][j - 1].ay; j--) {
                Lead t = col[c][j]; col[c][j] = col[c][j - 1]; col[c][j - 1] = t;
            }
        if (!cn[c]) continue;

        const float GAP  = 9.0f;
        const float minY = boxY0 + 4.0f;
        const float maxY = boxY0 + boxH - 4.0f;
        float rowY[CA_COUNT + 8];
        for (int i = 0; i < cn[c]; i++) rowY[i] = artY + col[c][i].ay * k;
        for (int i = 1; i < cn[c]; i++)
            if (rowY[i] < rowY[i - 1] + GAP) rowY[i] = rowY[i - 1] + GAP;
        const float over = rowY[cn[c] - 1] - maxY;
        if (over > 0.0f) for (int i = 0; i < cn[c]; i++) rowY[i] -= over;
        for (int i = cn[c] - 2; i >= 0; i--)
            if (rowY[i] > rowY[i + 1] - GAP) rowY[i] = rowY[i + 1] - GAP;
        if (rowY[0] < minY) {
            rowY[0] = minY;
            for (int i = 1; i < cn[c]; i++)
                if (rowY[i] < rowY[i - 1] + GAP) rowY[i] = rowY[i - 1] + GAP;
        }

        for (int i = 0; i < cn[c]; i++) {
            const float ly = floorf(rowY[i]);
            const float ax = floorf(artX + col[c][i].ax * k);
            const float ay = floorf(artY + col[c][i].ay * k);

            const float half = floorf(col[c][i].box * k * 0.5f);
            float endX = ax, endY = ay;
            bool level = half > 0.0f && ly >= ay - half && ly <= ay + half;
            if (level)          endX = (lineX[c] < ax) ? ax - half : ax + half;
            else if (half > 0)  endY = (ly < ay) ? ay - half : ay + half;

            const float x0 = lineX[c] < endX ? lineX[c] : endX;
            const float x1 = lineX[c] < endX ? endX : lineX[c];
            drawRect(x0, ly, x1 - x0 + 1.0f, 1.0f, LCE_LINE_COL);
            if (!level) {
                const float y0 = ly < endY ? ly : endY;
                drawRect(ax, y0, 1.0f, (ly < endY ? endY - ly : ly - endY) + 1.0f,
                         LCE_LINE_COL);
            }
            if (half > 0.0f) {
                const float bx = ax - half, by = ay - half, bs = half * 2.0f + 1.0f;
                drawRect(bx, by, bs, 1.0f, LCE_LINE_COL);
                drawRect(bx, by + bs - 1.0f, bs, 1.0f, LCE_LINE_COL);
                drawRect(bx, by, 1.0f, bs, LCE_LINE_COL);
                drawRect(bx + bs - 1.0f, by, 1.0f, bs, LCE_LINE_COL);
            }

            const float tw = fontTextWidth(&s.font, col[c][i].text) * CTL_TEXT;
            const float tx = (c == 0) ? lineX[0] - 3.0f - tw : lineX[1] + 3.0f;
            fontDrawTextShadow(&s.font, tx, ly - 4.0f * CTL_TEXT, col[c][i].text,
                               0xFFFFFFFFu, CTL_TEXT);
        }
    }
}

enum { SEL_LAYOUT = 0, SEL_INVERT, SEL_SOUTHPAW, SEL_FINEAIM, SEL_COUNT };
int s_sel = SEL_LAYOUT;

}

void controlsPageOpen() {
    s_open = true;
    artEnsure(g_controlScheme);
}

void controlSchemeNotePad(unsigned int buttons, unsigned char rx, unsigned char ry) {
    if (s_haveController) return;

    if (buttons & (PSP_CTRL_L1 | PSP_CTRL_R1 | PSP_CTRL_L3 | PSP_CTRL_R3)) {
        s_haveController = true;
        return;
    }

    if ((rx || ry) && rx > 88 && rx < 168 && ry > 88 && ry < 168) s_haveController = true;
}

#if CONTROLS_ANCHOR_EDITOR
bool controlsPageEditing() { return s_editing; }
#else
bool controlsPageEditing() { return false; }
#endif

bool fineAimApplies() {
    int scheme = g_controlScheme;
    if (!controlSchemeAvailable(scheme) || scheme < 0 || scheme >= CONTROL_SCHEMES) scheme = 0;
    return kPhys[scheme][CA_LOOK_L] != 0 && !g_southpaw;
}

bool controlSchemeIsPadLayout() {
    return g_controlScheme == CONTROL_SCHEMES - 1 && controlSchemeAvailable(g_controlScheme);
}

unsigned int controlSchemeMenuAlias(unsigned int buttons) {
    if (!controlSchemeIsPadLayout()) return buttons;
    if (buttons & PSP_CTRL_L1) buttons |= PSP_CTRL_LTRIGGER;
    if (buttons & PSP_CTRL_R1) buttons |= PSP_CTRL_RTRIGGER;
    return buttons;
}

bool controlSchemeAvailable(int scheme) {
    return scheme != CONTROL_SCHEMES - 1 || s_haveController;
}

int g_japaneseLayout = 0;

unsigned int menuFaceSwap(unsigned int buttons) {
    if (!g_japaneseLayout) return buttons;
    const unsigned int x = buttons & PSP_CTRL_CROSS, o = buttons & PSP_CTRL_CIRCLE;
    buttons &= ~(PSP_CTRL_CROSS | PSP_CTRL_CIRCLE);
    if (x) buttons |= PSP_CTRL_CIRCLE;
    if (o) buttons |= PSP_CTRL_CROSS;
    return buttons;
}

void controlsPageRender(MenuState& s) {
    if (!s.haveFont || !s.haveGui) return;
    sceGuDisable(GU_DEPTH_TEST);

    float bw = menuBarButtonW(s, "Back");
    float lb = 4.0f * MENU_PX + bw;
    drawMenuHeader(s, "Controls", 0.0f, VW, MENU_BAR_H, MENU_BAR_TEXT, lb, VW - lb);
    menuBarButton(s, 4.0f * MENU_PX, bw, "Back", false);

    drawNinePatch(s, GA_SS_PANEL, 3.0f,
                  CTL_PANEL_X, CTL_PANEL_Y, CTL_PANEL_W, CTL_PANEL_H, 1.5f);

    const float panelCx = CTL_PANEL_X + CTL_PANEL_W * 0.5f;

    char caption[48];
    snprintf(caption, sizeof(caption), "Current Controls : Layout %d", g_controlScheme + 1);
    float cw = fontTextWidth(&s.font, caption) * CTL_TEXT;
    fontDrawTextShadow(&s.font, panelCx * UI_SCALE - cw / 2.0f, CTL_CAPTION_Y * UI_SCALE,
                       caption, 0xFFFFFFFFu, CTL_TEXT);

    const bool onLayout = (s_sel == SEL_LAYOUT);
    float span = CTL_BTN_W * CONTROL_SCHEMES + (CTL_BTN_PITCH - CTL_BTN_W) * (CONTROL_SCHEMES - 1);
    float bx0  = panelCx - span * 0.5f;
    for (int i = 0; i < CONTROL_SCHEMES; i++) {
        char num[4]; snprintf(num, sizeof(num), "%d", i + 1);
        bool live = (g_controlScheme == i);

        bool usable = controlSchemeAvailable(i);
        float bx = bx0 + i * CTL_BTN_PITCH;

        guiTButton(s, bx, CTL_BTN_Y, CTL_BTN_W, CTL_BTN_H, live, 1.0f);
        guiTButtonLabel(s, bx, CTL_BTN_Y, CTL_BTN_W, CTL_BTN_H, num,
                        live && onLayout, usable, CTL_TEXT);
    }

    {

        const char* labels[3] = { "Invert Look", "Southpaw", "Fine Aim" };
        const bool  values[3] = { g_invertY != 0, g_southpaw != 0, g_fineAim != 0 };
        const int   sels[3]   = { SEL_INVERT, SEL_SOUTHPAW, SEL_FINEAIM };
        const bool  usable[3] = { true, true, fineAimApplies() };
        const float gap = 6.0f;
        float w[3], total = 0.0f;
        for (int i = 0; i < 3; i++) {
            w[i] = CTL_SW_W + CTL_CHECK_GAP + fontTextWidth(&s.font, labels[i]) * CTL_TEXT / UI_SCALE;
            total += w[i] + (i ? gap : 0.0f);
        }
        float gx = panelCx - total * 0.5f;
        for (int i = 0; i < 3; i++) {
            const bool on   = s_sel == sels[i];
            guiOptionSwitch(s, gx, CTL_CHECK_Y, CTL_SW_W, CTL_SW_H, values[i], on,
                            usable[i] ? WHITE : GUI_DISABLED);

            fontDrawTextShadow(&s.font, (gx + CTL_SW_W + CTL_CHECK_GAP) * UI_SCALE,
                               (CTL_CHECK_Y + CTL_SW_H * 0.5f) * UI_SCALE - 4.0f * CTL_TEXT,
                               labels[i],
                               !usable[i] ? GUI_DISABLED : on ? 0xFFFFFFFFu : 0xFFBBBBBBu,
                               CTL_TEXT);
            gx += w[i] + gap;
        }
    }

    artEnsure(g_controlScheme);
    if (s_artHave) {
        const int boxW = (int)(CTL_ART_W * UI_SCALE), boxH = (int)(CTL_ART_H * UI_SCALE);
        int k = 1;
        while (s_art.realW * (k + 1) <= boxW && s_art.realH * (k + 1) <= boxH) k++;
        const float dw = (float)(s_art.realW * k), dh = (float)(s_art.realH * k);

        textureBind(&s_art);
        spriteDraw(&s_art,
                   floorf((CTL_ART_X + CTL_ART_W * 0.5f) * UI_SCALE - dw * 0.5f),
                   floorf((CTL_ART_Y + CTL_ART_H * 0.5f) * UI_SCALE - dh * 0.5f),
                   dw, dh, 0.0f, 0.0f, (float)s_art.realW, (float)s_art.realH, WHITE);
#if CONTROLS_ANCHOR_EDITOR
        drawEditor(s,
                   floorf((CTL_ART_X + CTL_ART_W * 0.5f) * UI_SCALE - dw * 0.5f),
                   floorf((CTL_ART_Y + CTL_ART_H * 0.5f) * UI_SCALE - dh * 0.5f),
                   (float)k);
#endif
        drawLeaders(s, g_controlScheme, s_artKey,
                    floorf((CTL_ART_X + CTL_ART_W * 0.5f) * UI_SCALE - dw * 0.5f),
                    floorf((CTL_ART_Y + CTL_ART_H * 0.5f) * UI_SCALE - dh * 0.5f),
                    dw, dh, (float)k);
    }
}

static void cycleScheme(int dir) {
    int n = g_controlScheme + dir;
    if (n < 0) n = CONTROL_SCHEMES - 1;
    if (n >= CONTROL_SCHEMES) n = 0;

    optionsSetControlScheme(n);
}

void controlsPageInput(MenuState& s, unsigned int pressed) {
    (void)s;

#if CONTROLS_ANCHOR_EDITOR

    const ArtAnchors ed = anchorsForArt(s_artKey);
    if ((pressed & PSP_CTRL_START) || (s_editing && (pressed & PSP_CTRL_CIRCLE))) {
        if (s_editing) { anchorsSave(s_artKey); s_editing = false; }
        else if (editCount(ed)) { s_editing = true; s_editIdx = 0; }
        return;
    }
    if (s_editing) {
        const int n = editCount(ed);
        if (!n) { s_editing = false; return; }
        if (pressed & PSP_CTRL_LTRIGGER) s_editIdx = (s_editIdx + n - 1) % n;
        if (pressed & PSP_CTRL_RTRIGGER) s_editIdx = (s_editIdx + 1) % n;
        if (s_editIdx >= n) s_editIdx = 0;

        if (pressed & PSP_CTRL_CROSS) s_editStep = (s_editStep < 0.75f) ? 1.0f : 0.5f;
        EditTarget t = editTarget(ed, s_editIdx);
        if (t.x) {
            if (pressed & PSP_CTRL_LEFT)  *t.x -= s_editStep;
            if (pressed & PSP_CTRL_RIGHT) *t.x += s_editStep;
            if (pressed & PSP_CTRL_UP)    *t.y -= s_editStep;
            if (pressed & PSP_CTRL_DOWN)  *t.y += s_editStep;
        }
        return;
    }

#endif

    if (pressed & (PSP_CTRL_LTRIGGER | PSP_CTRL_RTRIGGER)) {
        cycleScheme((pressed & PSP_CTRL_RTRIGGER) ? 1 : -1);
        return;
    }

    if (pressed & PSP_CTRL_CIRCLE) {
        s_open = false;
        artFree();
        return;
    }

    if (pressed & (PSP_CTRL_UP | PSP_CTRL_DOWN))
        s_sel = (s_sel == SEL_LAYOUT) ? SEL_INVERT : SEL_LAYOUT;

    int dir = (pressed & PSP_CTRL_RIGHT) ? 1 : (pressed & PSP_CTRL_LEFT) ? -1 : 0;

    if (s_sel != SEL_LAYOUT) {

        if (dir) {
            int i = s_sel + dir;
            if (i < SEL_INVERT)  i = SEL_FINEAIM;
            if (i > SEL_FINEAIM) i = SEL_INVERT;
            s_sel = i;
            return;
        }
        if (pressed & PSP_CTRL_CROSS) {
            if (s_sel == SEL_INVERT)        optionsSetInvertLook(!g_invertY);
            else if (s_sel == SEL_SOUTHPAW) optionsSetSouthpaw(!g_southpaw);
            else if (fineAimApplies())      optionsSetFineAim(!g_fineAim);
        }
        return;
    }

    if (!dir && (pressed & PSP_CTRL_CROSS)) dir = 1;
    if (!dir) return;
    cycleScheme(dir);
}

unsigned int controlSchemeButtonFor(unsigned int logical) {
    int scheme = g_controlScheme;
    if (!controlSchemeAvailable(scheme)) scheme = 0;
    if (scheme <= 0 || scheme >= CONTROL_SCHEMES) return logical;
    for (int a = 0; a < CA_COUNT; a++)
        if (kLogical[a] == logical) return kPhys[scheme][a];
    return logical;
}

void controlSchemeCombos(unsigned int& pressed, unsigned int& held) {
    static bool was = false;
    int scheme = g_controlScheme;
    if (!controlSchemeAvailable(scheme)) scheme = 0;

    unsigned int both = 0;
    for (int a = 0; a < CA_COUNT; a++)
        if (kPhys[scheme][a] == PSP_CTRL_LTRIGGER || kPhys[scheme][a] == PSP_CTRL_RTRIGGER)
            both |= kLogical[a];
    const bool on = kComboInv[scheme] && both && ((held & both) == both);
    if (on) {
        if (!was) pressed |= ACT_INVENTORY;
        pressed &= ~both;
        held    &= ~both;
    }
    was = on;
}

unsigned int controlSchemeRemap(unsigned int buttons) {
    int scheme = g_controlScheme;

    if (!controlSchemeAvailable(scheme)) scheme = 0;
    if (scheme <= 0 || scheme >= CONTROL_SCHEMES) return buttons;

    unsigned int owned = 0;
    for (int a = 0; a < CA_COUNT; a++) owned |= kLogical[a] | kPhys[scheme][a];
    unsigned int out = buttons & ~owned;
    for (int a = 0; a < CA_COUNT; a++)
        if (buttons & kPhys[scheme][a]) out |= kLogical[a];
    return out;
}
