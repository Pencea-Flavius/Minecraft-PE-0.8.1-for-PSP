
#include "world/level/tile/material.h"
#include "world/level/chunk/chunk.h"

const Material Material::air             (0,   0,    0,     0,    0,   0,     1,          1);
const Material Material::dirt            (1,   1,    1,     0,    0,   0,     1,          0);
const Material Material::wood            (1,   1,    1,     0,    1,   0,     1,          0);
const Material Material::stone           (1,   1,    1,     0,    0,   0,     0,          0);
const Material Material::metal           (1,   1,    1,     0,    0,   0,     0,          0);
const Material Material::water           (0,   1,    0,     1,    0,   0,     1,          1);
const Material Material::lava            (0,   1,    0,     1,    0,   0,     1,          1);
const Material Material::leaves          (1,   1,    1,     0,    1,   1,     1,          0,  1);
const Material Material::plant           (0,   0,    0,     0,    0,   0,     1,          0);
const Material Material::replaceablePlant(0,   0,    0,     0,    1,   0,     1,          1);
const Material Material::cloth           (1,   1,    1,     0,    1,   0,     1,          0);
const Material Material::fire            (0,   0,    0,     0,    0,   0,     1,          1);
const Material Material::sand            (1,   1,    1,     0,    0,   0,     1,          0);
const Material Material::decoration      (0,   0,    0,     0,    0,   0,     1,          0);
const Material Material::glass           (1,   1,    1,     0,    0,   1,     1,          0,  1);
const Material Material::explosive       (1,   1,    1,     0,    1,   1,     1,          0,  1);
const Material Material::ice             (1,   1,    1,     0,    0,   1,     1,          0,  1);
const Material Material::topSnow         (0,   0,    0,     0,    0,   1,     0,          1,  1);
const Material Material::snow            (1,   1,    1,     0,    0,   0,     0,          0);
const Material Material::cactus          (1,   1,    1,     0,    0,   1,     1,          0,  1);
const Material Material::clay            (1,   1,    1,     0,    0,   0,     1,          0);
const Material Material::vegetable       (1,   1,    1,     0,    0,   0,     1,          0);
const Material Material::cake            (1,   1,    1,     0,    0,   0,     1,          0);
const Material Material::web             (1,   1,    0,     0,    0,   0,     0,          0);

const Material& materialOf(unsigned char id) {
    switch (id) {
        case BLOCK_AIR: return Material::air;

        case BLOCK_GRASS:
        case BLOCK_DIRT: case BLOCK_FARMLAND: case BLOCK_UPDATE1: case BLOCK_UPDATE2:
        case BLOCK_SPONGE:
        case BLOCK_HAY_BLOCK:
            return Material::dirt;

        case BLOCK_LEAVES:
            return Material::leaves;
        case BLOCK_PLANKS: case BLOCK_LOG: case BLOCK_BOOKSHELF: case BLOCK_STAIRS_PLANKS:
        case BLOCK_STAIRS_SPRUCE: case BLOCK_STAIRS_BIRCH: case BLOCK_STAIRS_JUNGLE:
        case BLOCK_CHEST: case BLOCK_CRAFTING_TABLE: case BLOCK_SIGN: case BLOCK_DOOR_WOOD:
        case BLOCK_WALL_SIGN: case BLOCK_FENCE: case BLOCK_TRAPDOOR: case BLOCK_FENCE_GATE:

        case BLOCK_WOOD_SLAB: case BLOCK_WOOD_SLAB_DOUBLE:
            return Material::wood;
        case BLOCK_GOLD_BLOCK: case BLOCK_IRON_BLOCK: case BLOCK_DIAMOND_BLOCK: case BLOCK_DOOR_IRON:
        case BLOCK_COAL_BLOCK:
        case BLOCK_IRON_BARS:
        case BLOCK_NETHER_REACTOR:
            return Material::metal;
        case BLOCK_WATER: case BLOCK_CALM_WATER:
            return Material::water;
        case BLOCK_LAVA: case BLOCK_CALM_LAVA:
            return Material::lava;
        case BLOCK_SAPLING: case BLOCK_FLOWER: case BLOCK_ROSE: case BLOCK_MUSHROOM_BROWN:
        case BLOCK_MUSHROOM_RED: case BLOCK_WHEAT: case BLOCK_REEDS: case BLOCK_MELON_STEM:

        case BLOCK_CARROTS: case BLOCK_POTATOES: case BLOCK_BEETROOT:
        case BLOCK_PUMPKIN_STEM:
            return Material::plant;
        case BLOCK_TALLGRASS:
            return Material::replaceablePlant;
        case BLOCK_BED: case BLOCK_WOOL: case BLOCK_CARPET:
            return Material::cloth;
        case BLOCK_SAND: case BLOCK_GRAVEL:
            return Material::sand;

        case BLOCK_TORCH: case BLOCK_LADDER: case BLOCK_RAIL: case BLOCK_GOLDEN_RAIL:
            return Material::decoration;
        case BLOCK_GLASS: case BLOCK_GLOWSTONE: case BLOCK_GLASS_PANE:
            return Material::glass;
        case BLOCK_TNT:
            return Material::explosive;
        case BLOCK_ICE:
            return Material::ice;
        case BLOCK_TOPSNOW:
            return Material::topSnow;
        case BLOCK_SNOW_BLOCK:
            return Material::snow;
        case BLOCK_CACTUS:
            return Material::cactus;
        case BLOCK_CLAY:
            return Material::clay;
        case BLOCK_MELON:

        case BLOCK_PUMPKIN: case BLOCK_PUMPKIN_LIT:
            return Material::vegetable;

        case BLOCK_CAKE:
            return Material::cake;
        case BLOCK_COBWEB:
            return Material::web;

        case BLOCK_FIRE:
            return Material::fire;

        default: return Material::stone;
    }
}
