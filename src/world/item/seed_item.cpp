#include "world/entity/player.h"
#include "world/inventory/inventory.h"
#include "world/item/seed_item.h"
#include "world/item/food_item.h"
#include "world/level/world.h"

bool seedPlantOn(Player* player, World* world, int x, int y, int z, int face, short resultTile) {
    if (face != F_TOP) return false;
    if (worldBlock(world, x, y, z) != BLOCK_FARMLAND) return false;
    if (worldBlock(world, x, y + 1, z) != BLOCK_AIR) return false;

    worldSetTileUpdate(world, x, y + 1, z, (unsigned char)resultTile, 0);
    worldUpdateLights(world);
    worldRebuildAroundNow(world, x, y + 1, z);
    if (player) player->inventory->consumeSelected();
    return true;
}

bool SeedItem::useOn(ItemInstance* item, Player* player, World* world, int x, int y, int z, int face, float, float, float) {
    (void)item;
    return seedPlantOn(player, world, x, y, z, face, resultTile);
}

bool SeedFoodItem::useOn(ItemInstance* item, Player* player, World* world, int x, int y, int z, int face, float, float, float) {
    (void)item;
    return seedPlantOn(player, world, x, y, z, face, plantTile);
}
