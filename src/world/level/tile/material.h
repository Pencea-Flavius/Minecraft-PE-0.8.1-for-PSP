
#ifndef MCPSP_WORLD_LEVEL_TILE_MATERIAL_H
#define MCPSP_WORLD_LEVEL_TILE_MATERIAL_H

class Material {
public:

    Material(bool solid, bool blocksLight, bool blocksMotion, bool liquid,
             bool flammable, bool neverBuildable, bool alwaysDestroyable, bool replaceable,
             bool translucent = false)
        : solid_(solid), blocksLight_(blocksLight), blocksMotion_(blocksMotion),
          liquid_(liquid), flammable_(flammable), neverBuildable_(neverBuildable),
          alwaysDestroyable_(alwaysDestroyable), replaceable_(replaceable),
          translucent_(translucent) {}

    bool isSolid() const             { return solid_; }
    bool blocksLight() const         { return blocksLight_; }
    bool blocksMotion() const        { return blocksMotion_; }
    bool isLiquid() const            { return liquid_; }
    bool isFlammable() const         { return flammable_; }
    bool neverBuildable() const      { return neverBuildable_; }

    bool isAlwaysDestroyable() const { return alwaysDestroyable_; }
    bool isReplaceable() const       { return replaceable_; }

    bool isSolidBlocking() const     { return !translucent_ && blocksMotion_; }

    static const Material air, dirt, wood, stone, metal, water, lava, leaves,
                          plant, replaceablePlant, cloth, fire, sand, decoration,
                          glass, explosive, ice, topSnow, snow, cactus, clay,
                          vegetable, cake, web;

private:
    bool solid_, blocksLight_, blocksMotion_, liquid_;
    bool flammable_, neverBuildable_, alwaysDestroyable_, replaceable_, translucent_;
};

const Material& materialOf(unsigned char id);

#endif
