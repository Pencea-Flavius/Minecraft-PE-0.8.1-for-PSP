#include "world/level/tile/entity/chest_tile_entity.h"
#include "world/item/item_instance.h"
#include "world/level/level.h"
#include "world/level/chunk/chunk.h"
#include "nbt/compound_tag.h"
#include "nbt/list_tag.h"
#include "world/level/tile/tile_gui_hooks.h"
#include <stdlib.h>

ChestTileEntity::ChestTileEntity()
    : super(TE_CHEST),
      container(CONTAINER_SIZE, 0, ContainerType::CONTAINER, false),
      pair(0), pairPending(false), pairX(0), pairZ(0),
      openness(0.0f), oOpenness(0.0f), openCount(0),
      openedBy(false), openCountdown(0) {
    rendererId = TR_CHEST_RENDERER;
}

ChestTileEntity::~ChestTileEntity() {
    if (pair) {
        pair->pair = 0;
        pair = 0;
    }
}

static ChestTileEntity* ownerOf(ChestTileEntity* self, int& i) {
    if (!self->pair) return self;
    ChestTileEntity* m = self->isMaster() ? self : self->pair;
    if (i < ChestTileEntity::CONTAINER_SIZE) return m;
    i -= ChestTileEntity::CONTAINER_SIZE;
    return (m == self) ? self->pair : self;
}

ItemInstance* ChestTileEntity::getItem(int i) const {
    if (i < 0 || i >= getContainerSize()) return 0;
    ChestTileEntity* o = ownerOf(const_cast<ChestTileEntity*>(this), i);
    return o->container.getItem(i);
}

void ChestTileEntity::clearSlot(int i) {
    if (i < 0 || i >= getContainerSize()) return;
    ChestTileEntity* o = ownerOf(this, i);
    o->container.clearSlot(i);
}

bool ChestTileEntity::add(ItemInstance& it) {
    if (!pair) return container.add(it);

    ChestTileEntity* m = isMaster() ? this : pair;
    ChestTileEntity* s = (m == this) ? pair : this;
    if (m->container.add(it)) return true;
    return s->container.add(it);
}

bool ChestTileEntity::isMaster() const {
    if (!pair) return true;
    int d = faceFromMcpe(getData());
    if (pair->x != x)
        return (d == F_BACK) ? (x > pair->x) : (x < pair->x);
    if (pair->z != z)
        return (d == F_RIGHT) ? (z > pair->z) : (z < pair->z);
    return true;
}

void ChestTileEntity::pairWith(ChestTileEntity* other) {
    pair = other;
    pairPending = false;
    pairX = other->x; pairZ = other->z;
}

void ChestTileEntity::unpair() {
    if (!pair) return;
    pair->pair = 0;
    pair = 0;
}

bool ChestTileEntity::canPairWith(TileEntity* te) const {
    if (!te) return false;
    if (te->type != TE_CHEST) return false;
    if (te->y != y) return false;
    const ChestTileEntity* other = (const ChestTileEntity*)te;
    if (other->pair) return false;
    if (!level) return false;
    int d = level->getData(x, y, z);
    if (d != level->getData(te->x, te->y, te->z)) return false;
    d = faceFromMcpe(d);

    if (te->x == x) return (d == F_LEFT || d == F_RIGHT);
    else            return (d == F_BACK || d == F_FORWARD);
}

float ChestTileEntity::getModelOffsetX() const {
    if (!pair || !isMaster()) return 0.0f;
    int a, b;
    if (pair->x == x) { a = pair->z; b = z; }
    else              { a = pair->x; b = x; }
    return (a >= b) ? 0.5f : -0.5f;
}

void ChestTileEntity::resolvePendingPair() {
    if (!pairPending || pair || !level) return;

    if (level->getTile(pairX, y, pairZ) != BLOCK_CHEST) { pairPending = false; return; }
    TileEntity* t = level->getTileEntity(pairX, y, pairZ);
    if (!t || t->type != TE_CHEST) return;
    ChestTileEntity* other = (ChestTileEntity*)t;
    if (other->pair) { pairPending = false; return; }
    pairWith(other);
    other->pairWith(this);
}

bool ChestTileEntity::_canOpenThis() const {
    if (!level) return true;
    return !isOpaque((unsigned char)level->getTile(x, y + 1, z));
}

bool ChestTileEntity::canOpen() const {
    if (pair) return _canOpenThis() && pair->_canOpenThis();
    return _canOpenThis();
}

ChestTileEntity* ChestTileEntity::openStateOwner() {
    return (pair && !isMaster()) ? pair : this;
}

void ChestTileEntity::openBy() {
    ChestTileEntity* m = openStateOwner();
    if (m->openedBy) return;
    m->openCountdown = OPEN_DELAY;
    m->openedBy = true;
    m->startOpen();
}

void ChestTileEntity::startOpen() { openStateOwner()->openCount++; }
void ChestTileEntity::stopOpen()  {
    ChestTileEntity* m = openStateOwner();
    if (m->openCount > 0) m->openCount--;
}

int ChestTileEntity::stepOpenness() {
    int snd = LID_SOUND_NONE;
    if (openCount > 0 && openness == 0.0f) snd |= LID_SOUND_OPEN;

    if ((openCount > 0 && openness < 1.0f) || (openCount == 0 && openness > 0.0f)) {
        float old = openness;
        oOpenness = openness;
        openness += (openCount > 0) ? 0.1f : -0.1f;
        if (openness > 1.0f) openness = 1.0f;
        if (openness < 0.0f) openness = 0.0f;
        if (openness < 0.5f && old >= 0.5f) snd |= LID_SOUND_CLOSE;
    } else {
        oOpenness = openness;
    }
    return snd;
}

void ChestTileEntity::tick() {
    resolvePendingPair();

    if (openedBy && --openCountdown <= 0) {
        openedBy = false;
        if (!chestGuiSuppressed()) guiOpenChest(this);
        stopOpen();
    }

    int snd = stepOpenness();
    if (snd && level) {
        float pitch = (rand() / (float)RAND_MAX) * 0.1f + 0.9f;
        if (snd & LID_SOUND_OPEN)
            level->playSound(x + 0.5f, y + 0.5f, z + 0.5f, "random.chestopen", 0.5f, pitch);
        if (snd & LID_SOUND_CLOSE)
            level->playSound(x + 0.5f, y + 0.5f, z + 0.5f, "random.chestclosed", 0.5f, pitch);
    }
}

bool ChestTileEntity::save(CompoundTag* tag) {
    if (!super::save(tag)) return false;
    ListTag* items = new ListTag("Items");
    for (int i = 0; i < CONTAINER_SIZE; i++) {
        ItemInstance* it = container.getItem(i);
        if (it && !it->isNull()) {
            CompoundTag* c = new CompoundTag();
            c->putByte("Slot", (char)i);
            c->putShort("id", it->id);
            c->putByte("Count", (char)it->count);
            c->putShort("Damage", it->data);
            items->add(c);
        }
    }
    tag->put("Items", items);

    if (pair) {
        tag->putInt("pairx", pair->x);
        tag->putInt("pairz", pair->z);
    }
    return true;
}

void ChestTileEntity::load(CompoundTag* tag) {
    super::load(tag);
    container.clearInventory();
    ListTag* items = tag->getList("Items");
    for (int i = 0; i < items->size(); i++) {
        CompoundTag* c = (CompoundTag*)items->get(i);
        if (!c) continue;
        int slot = (unsigned char)c->getByte("Slot");
        if (slot < 0 || slot >= CONTAINER_SIZE) continue;
        ItemInstance stack(c->getShort("id"),
                           (unsigned char)c->getByte("Count"),
                           c->getShort("Damage"));
        container.setItem(slot, &stack);
    }

    pairPending = tag->contains("pairx");
    if (pairPending) { pairX = tag->getInt("pairx"); pairZ = tag->getInt("pairz"); }
}
