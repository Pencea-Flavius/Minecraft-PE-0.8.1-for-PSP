
#ifndef MCPSP_WORLD_ENTITY_MOB_CATEGORY_H
#define MCPSP_WORLD_ENTITY_MOB_CATEGORY_H

#include "world/entity/entity_types.h"

namespace MobCategory {
    struct Category { int baseType; int maxPerLevel; bool friendly; bool water; };

    static const Category monster  = { EntityTypes::BaseEnemy,         20, false, false };
    static const Category creature = { EntityTypes::BaseCreature,      15, true,  false };
    static const Category water    = { EntityTypes::BaseWaterCreature, 10, true,  true  };

    static const int MAX_MONSTERS_EGG   = 32;
    static const int MAX_ANIMALS_BREED  = 23;
    static const int MAX_ANIMALS_EGG    = 27;

    static const int MAX_CHICKENS_BREED = 8;
    static const int MAX_CHICKENS_EGG   = 12;

    inline int baseTypeOf(int mobType) {
        switch (mobType) {
            case EntityTypes::IdChicken:
            case EntityTypes::IdCow:
            case EntityTypes::IdPig:
            case EntityTypes::IdSheep:     return EntityTypes::BaseCreature;
            case EntityTypes::IdZombie:
            case EntityTypes::IdCreeper:
            case EntityTypes::IdSkeleton:
            case EntityTypes::IdSpider:
            case EntityTypes::IdPigZombie: return EntityTypes::BaseEnemy;
        }
        return 0;
    }
}

namespace MobCap {
    const int OK              = 0;
    const int TOO_MANY_ANIMALS = 1;
    const int TOO_MANY_CHICKENS = 2;
    const int TOO_MANY_ENEMIES  = 3;
    const int PEACEFUL          = 4;
    const int NO_ROOM           = 5;
}

#endif
