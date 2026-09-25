#include "world/entity/player.h"
#include "world/inventory/inventory.h"
#include "world/item/bonemeal_item.h"
#include "world/level/world.h"
#include "world/level/tile/tile.h"

bool BonemealItem::useOn(ItemInstance* item, Player* player, World* world, int x, int y, int z, int face, float, float, float) {

    if (item->data != DYE_WHITE) return false;
    Tile* tile = Tile::tiles[worldBlock(world, x, y, z)];
    if (!tile || !tile->onFertilized(world, x, y, z)) return false;
    if (player) player->inventory->consumeSelected();
    return true;
}
