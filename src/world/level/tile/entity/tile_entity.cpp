#include "world/level/tile/entity/tile_entity.h"
#include "world/level/level.h"
#include "world/entity/player.h"
#include "nbt/compound_tag.h"

TileEntity::TileEntity(int type)
    : type(type), x(0), y(0), z(0), level(0), rendererId(TR_NO_RENDER), removed(false) {}

void TileEntity::setLevelAndPos(Level* lvl, int px, int py, int pz) {
    level = lvl;
    x = px; y = py; z = pz;
}

bool TileEntity::save(CompoundTag* tag) {
    tag->putInt("x", x);
    tag->putInt("y", y);
    tag->putInt("z", z);
    return true;
}

void TileEntity::load(CompoundTag* tag) {
    x = tag->getInt("x");
    y = tag->getInt("y");
    z = tag->getInt("z");
}

bool TileEntity::stillValid(Player* player) {
    if (!level || !player) return false;
    if (level->getTileEntity(x, y, z) != this) return false;
    if (player->distanceToSqr(x + 0.5f, y + 0.5f, z + 0.5f) > 8 * 8) return false;
    return true;
}

int TileEntity::getTile() const { return level ? level->getTile(x, y, z) : 0; }
int TileEntity::getData() const { return level ? level->getData(x, y, z) : 0; }
