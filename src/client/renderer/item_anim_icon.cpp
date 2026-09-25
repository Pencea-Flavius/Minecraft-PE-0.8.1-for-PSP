#include "client/renderer/item_anim_icon.h"
#include "world/item/item.h"
#include "world/item/clock_item.h"
#include "world/item/compass_item.h"
#include "world/item/dial_math.h"
#include "gpu/texture.h"
#include "platform/path.h"
#include <pspgu.h>

static Texture s_watch, s_compass;
static bool    s_loaded = false, s_watchOk = false, s_compassOk = false;

static void ensureAssets() {
    if (s_loaded) return;
    s_loaded = true;
    s_watchOk = textureLoad16(assetPath("data/images/item/watch-atlas.png"), &s_watch, GU_PSM_5551)
             || textureLoad16("data/images/item/watch-atlas.png", &s_watch, GU_PSM_5551);
    s_compassOk = textureLoad16(assetPath("data/images/item/compass.png"), &s_compass, GU_PSM_5551)
               || textureLoad16("data/images/item/compass.png", &s_compass, GU_PSM_5551);
}

const Texture* itemAnimIcon(short id, int frame, int* sx, int* sy) {
    if (id == ITEM_CLOCK) {
        ensureAssets();
        if (!s_watchOk) return 0;
        if (frame < 0) frame = 0;
        if (frame >= ClockItem::FRAMES) frame = ClockItem::FRAMES - 1;
        int col, row;
        Dial::clockCell(frame, &col, &row);
        *sx = col * 16;
        *sy = row * 16;
        return &s_watch;
    }
    if (id == ITEM_COMPASS) {
        ensureAssets();
        if (!s_compassOk) return 0;
        if (frame < 0) frame = 0;
        if (frame >= CompassItem::FRAMES) frame = CompassItem::FRAMES - 1;
        *sx = 0;
        *sy = frame * 16;
        return &s_compass;
    }
    return 0;
}

int itemAnimFrame(short id, const Player* holder) {
    if (id <= 0 || id >= 4096) return 0;
    Item* item = Item::items[id];
    return item ? item->getAnimationFrameFor(holder) : 0;
}

int itemAnimStage(short id, const Player* holder) {
    int sx, sy;
    if (!itemAnimIcon(id, 0, &sx, &sy)) return -1;
    return itemAnimFrame(id, holder);
}
