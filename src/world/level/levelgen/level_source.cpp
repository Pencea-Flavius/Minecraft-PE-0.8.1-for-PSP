#include "world/level/levelgen/level_source.h"
#include "world/level/world.h"
#include "world/level/chunk/chunk.h"
#include "world/level/levelgen/mcpegen.h"
#include "world/level/levelgen/gen_features.h"
#include "world/level/storage/level_storage.h"

#include <cstring>

namespace {

class RandomLevelSource : public LevelSource {
public:
    void buildTerrain(World* w, long seed) {

        worldGenerateMCPE(w, seed, LevelStorage::getActiveGenMask());
        worldSettleLiquids(w);
    }
    void buildChunk(World* w, int cx, int cz) { chunkGenerateTerrain(w, cx, cz); }
    const char* label() const { return "Old"; }
};

class FlatLevelSource : public LevelSource {
public:

    void buildTerrain(World* w, long ) { worldGenerateWindow(w); }

    void buildChunk(World* w, int cx, int cz) {

        unsigned char col[WORLD_H];
        std::memset(col, BLOCK_AIR, WORLD_H);
        col[0] = BLOCK_BEDROCK;
        col[1] = BLOCK_DIRT;
        col[2] = BLOCK_DIRT;
        col[3] = BLOCK_GRASS;

        for (int gz = cz * CHUNK_SZ; gz < cz * CHUNK_SZ + CHUNK_SZ; gz++)
            for (int gx = cx * CHUNK_SX; gx < cx * CHUNK_SX + CHUNK_SX; gx++)
                blockColumnPut(w, gx, gz, col);
    }

    bool spawnsMobs() const { return false; }

    bool supportsGenFeatures() const { return false; }


    float horizonHeight() const { return 0.0f; }

    int forcedGameType() const { return 1; }
    const char* label() const { return "Flat"; }
};

class SkyLevelSource : public LevelSource {
public:

    void buildTerrain(World* w, long seed) {
        worldGenerateMCPE(w, seed, LevelStorage::getActiveGenMask());
        worldGuaranteeSkyLiquids(w, seed);
        worldSettleLiquids(w);
    }
    void buildChunk(World* w, int cx, int cz) { chunkGenerateTerrain(w, cx, cz); }
    bool floatingIslands() const { return true; }

    bool genFeatureAllowed(int feature) const { return feature != GEN_FEATURE_CAVES; }

    float horizonHeight() const { return 0.0f; }

    float cloudHeight() const { return -16.0f; }
    const char* label() const { return "Sky"; }
};

RandomLevelSource s_random;
FlatLevelSource   s_flat;
SkyLevelSource    s_sky;

}

LevelSource& levelSourceFor(int worldType) {

    if (worldType == WORLD_TYPE_FLAT) return s_flat;
    if (worldType == WORLD_TYPE_SKY)  return s_sky;
    return s_random;
}

LevelSource& activeLevelSource() {
    return levelSourceFor(LevelStorage::getActiveWorldType());
}
