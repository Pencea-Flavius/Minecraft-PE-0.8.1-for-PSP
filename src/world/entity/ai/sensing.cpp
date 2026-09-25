
#include "world/entity/ai/sensing.h"
#include "world/entity/mob.h"

bool Sensing::canSee(Entity* e) {
    int id = e->entityId;
    for (int i = 0; i < count; i++)
        if (ids[i] == id) return seen[i];

    bool r = holder->canSee(e);

    if (count < SEEN_MAX) { ids[count] = id; seen[count] = r; count++; }
    return r;
}
