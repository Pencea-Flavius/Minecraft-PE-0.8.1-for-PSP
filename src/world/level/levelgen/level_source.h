
#ifndef MCPSP_WORLD_LEVEL_LEVELGEN_LEVEL_SOURCE_H
#define MCPSP_WORLD_LEVEL_LEVELGEN_LEVEL_SOURCE_H

struct World;

enum { WORLD_TYPE_OLD = 0, WORLD_TYPE_FLAT = 1, WORLD_TYPE_SKY = 2, WORLD_TYPE_COUNT = 3 };

class LevelSource {
public:
    virtual ~LevelSource() {}

    virtual void buildTerrain(World* w, long seed) = 0;

    virtual void buildChunk(World* w, int cx, int cz) = 0;

    virtual bool spawnsMobs() const { return true; }

    virtual bool supportsGenFeatures() const { return true; }


    virtual float horizonHeight() const { return 63.0f; }

    virtual bool floatingIslands() const { return false; }

    virtual bool genFeatureAllowed(int ) const { return true; }

    virtual float cloudHeight() const { return 128.33f; }

    virtual int forcedGameType() const { return -1; }

    virtual const char* label() const = 0;
};

LevelSource& levelSourceFor(int worldType);

LevelSource& activeLevelSource();

#endif
