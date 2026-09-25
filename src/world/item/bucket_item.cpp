
#include "world/item/bucket_item.h"
#include "world/item/item_instance.h"
#include "world/entity/player.h"
#include "world/inventory/inventory.h"
#include "world/level/level.h"
#include "world/level/world.h"
#include "world/level/chunk/chunk.h"
#include "world/level/tile/material.h"
#include "gpu/item_icons.h"
#include <stdlib.h>

extern Level g_level;

int BucketItem::getMaxStackSize(const ItemInstance* item) const {
    return (item && item->data != BUCKET_EMPTY) ? 1 : 16;
}

int BucketItem::getIcon(short data) const {
    if (isWaterId((unsigned char)data)) return II_BUCKET_WATER;
    if (isLavaId((unsigned char)data))  return II_BUCKET_LAVA;
    if (data == BUCKET_MILK)            return II_BUCKET_MILK;
    return II_BUCKET_EMPTY;
}

bool BucketItem::emptyBucket(World* world, int type, int x, int y, int z) {
    if (type <= 0) return false;

    unsigned char old = worldBlock(world, x, y, z);
    if (old != BLOCK_AIR && materialOf(old).isSolid()) return false;

    if (liquidStopsWater(old)) return false;

    worldSetBlockAndData(world, x, y, z, (unsigned char)type, 0);
    worldScheduleTick(world, x, y, z, (unsigned char)type,
                      liquidTickDelay((unsigned char)type));
    worldNotifyNeighborsChanged(world, x, y, z);
    worldUpdateLights(world);
    worldRebuildAroundNow(world, x, y, z);

    if (isWaterId((unsigned char)type) && (old == BLOCK_FIRE || isLavaId(old))) {
        float r = (rand() / (float)RAND_MAX) - (rand() / (float)RAND_MAX);
        g_level.playSound(x + 0.5f, y + 0.5f, z + 0.5f, "random.fizz", 0.5f, r * 0.8f + 2.6f);
    }
    return true;
}

static void swapBucket(ItemInstance* item, Player* player, short data) {
    if (--item->count <= 0) {
        item->id = ITEM_BUCKET; item->count = 1; item->data = data;
        return;
    }
    ItemInstance made(item->id, 1, data);

    if (player->inventory->add(made))
        player->inventory->ensureHotbar(ITEM_BUCKET, data);
}

bool BucketItem::useOn(ItemInstance* item, Player* player, World* world,
                       int x, int y, int z, int face, float, float, float) {
    if (!item || !player) return false;
    short aux = item->data;
    bool creative = player->inventory->isCreative();

    bool room = player->inventory->getFreeSlot() >= 0 || item->count == 1;
    if (!room && aux == BUCKET_EMPTY && !creative) return false;

    if (aux == BUCKET_EMPTY) {

        unsigned char id = worldBlock(world, x, y, z);
        if (!isLiquidId(id) || worldData(world, x, y, z) != 0) return false;
        worldSetBlockAndData(world, x, y, z, BLOCK_AIR, 0);
        worldNotifyNeighborsChanged(world, x, y, z);
        worldUpdateLights(world);
        worldRebuildAroundNow(world, x, y, z);
        if (creative) return true;

        swapBucket(item, player, isWaterId(id) ? BLOCK_WATER : BLOCK_LAVA);
        return true;
    }

    if (aux <= BUCKET_MILK) return false;

    x += kFaceNeighbor[face][0];
    y += kFaceNeighbor[face][1];
    z += kFaceNeighbor[face][2];
    if (!emptyBucket(world, aux, x, y, z)) return false;

    swapBucket(item, player, BUCKET_EMPTY);
    return true;
}
