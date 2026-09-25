#include "world/item/minecart_item.h"
#include "world/entity/minecart.h"
#include "world/entity/entity_types.h"
#include "world/level/level.h"
#include "world/level/world.h"
#include "world/entity/player.h"
#include "world/inventory/inventory.h"
#include "client/gui/hud.h"

static const int MAX_MINECARTS = 40;

static int countMinecarts() {
    int n = 0;
    for (size_t i = 0; i < g_level.entities.size(); i++) {
        Entity* e = g_level.entities[i];
        if (e && !e->removed && e->getEntityTypeId() == EntityTypes::IdMinecart) n++;
    }
    return n;
}

bool MinecartItem::useOn(ItemInstance* , Player* player, World* world,
                         int x, int y, int z, int ,
                         float, float, float) {
    if (!isRail(worldBlock(world, x, y, z))) return false;

    if (countMinecarts() >= MAX_MINECARTS) {

        hudChatMessage("The maximum number of Minecarts in a world has been reached.");
        return true;
    }

    g_level.addEntity(new Minecart(&g_level, x + 0.5f, y + 0.5f, z + 0.5f));

    if (player) player->inventory->consumeSelected();
    return true;
}
