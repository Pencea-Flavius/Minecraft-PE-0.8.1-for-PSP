#include "world/level/levelgen/biome.h"
#include "world/level/levelgen/mcpegen_internal.h"
#include "world/level/world.h"

BiomeId classifyBiome(float temperature, float downfall) {
    float t = (int)(temperature * 63) / 63.0f;
    float d = (int)(downfall * 63) / 63.0f;
    d *= t;
    if (t < 0.10f) return B_TUNDRA;
    if (d < 0.20f) {
        if (t < 0.50f) return B_TUNDRA;
        else if (t < 0.95f) return B_SAVANNA;
        else return B_DESERT;
    }
    if (d > 0.5f && t < 0.7f) return B_SWAMP;
    if (t < 0.50f) return B_TAIGA;
    if (t < 0.97f) return (d < 0.35f) ? B_SHRUB : B_FOREST;
    if (d < 0.45f) return B_PLAINS;
    else if (d < 0.90f) return B_SEASONAL;
    else return B_RAIN;
}

void biomeSurface(BiomeId b, unsigned char* top, unsigned char* material) {
    if (b == B_DESERT) { *top = BLOCK_SAND; *material = BLOCK_SAND; }
    else               { *top = BLOCK_GRASS; *material = BLOCK_DIRT; }
}

void McpeGen::computeBiome(int chunkX, int chunkZ) {
    int x = chunkX * 16, z = chunkZ * 16;
    rawTemp     = temperatureMap.getRegion(rawTemp,     x, z, 16, 16, BIOME_TEMP_SCALE,  BIOME_TEMP_SCALE,  0.25f);
    rawDownfall = downfallMap.getRegion(rawDownfall,    x, z, 16, 16, BIOME_DOWN_SCALE,  BIOME_DOWN_SCALE,  0.3333f);
    rawNoise    = noiseMap.getRegion(rawNoise,          x, z, 16, 16, BIOME_NOISE_SCALE, BIOME_NOISE_SCALE, 0.588f);

    for (int pp = 0; pp < 16 * 16; pp++) {
        float noise = (rawNoise[pp] * 1.1f + 0.5f);

        float split2 = 0.01f, split1 = 1 - split2;
        float temperature = (rawTemp[pp] * 0.15f + 0.7f) * split1 + noise * split2;
        split2 = 0.002f; split1 = 1 - split2;
        float downfall = (rawDownfall[pp] * 0.15f + 0.5f) * split1 + noise * split2;

        temperature = 1 - ((1 - temperature) * (1 - temperature));
        if (temperature < 0) temperature = 0;
        if (downfall < 0) downfall = 0;
        if (temperature > 1) temperature = 1;
        if (downfall > 1) downfall = 1;

        mTemp[pp] = temperature;
        mDownfall[pp] = downfall;
    }
}

static void biomeClimate(float rawTemp, float rawDownfall, float rawNoise, float* temp, float* down) {
    float noise = (rawNoise * 1.1f + 0.5f);
    float split2 = 0.01f, split1 = 1 - split2;
    float temperature = (rawTemp * 0.15f + 0.7f) * split1 + noise * split2;
    split2 = 0.002f; split1 = 1 - split2;
    float downfall = (rawDownfall * 0.15f + 0.5f) * split1 + noise * split2;
    temperature = 1 - ((1 - temperature) * (1 - temperature));
    if (temperature < 0) temperature = 0;
    if (downfall < 0) downfall = 0;
    if (temperature > 1) temperature = 1;
    if (downfall > 1) downfall = 1;
    *temp = temperature; *down = downfall;
}

namespace {
struct BiomeNoise {
    Random rndTemp, rndDownfall, rndNoise;
    PerlinNoise temperatureMap, downfallMap, noiseMap;
    explicit BiomeNoise(long seed)
      : rndTemp((long)((unsigned int)seed * 9871u)),
        rndDownfall((long)((unsigned int)seed * 39811u)),
        rndNoise((long)((unsigned int)seed * 543321u)),
        temperatureMap(&rndTemp, 4), downfallMap(&rndDownfall, 4), noiseMap(&rndNoise, 2) {}
};
}

BiomeId worldBiomeAt(long seed, int x, int z) {
    static BiomeNoise* s_noise = 0;
    static long s_seed = 0;
    if (!s_noise || s_seed != seed) {
        delete s_noise;
        s_noise = new BiomeNoise(seed);
        s_seed = seed;
    }
    float t[1], d[1], n[1];
    s_noise->temperatureMap.getRegion(t, x, z, 1, 1, BIOME_TEMP_SCALE,  BIOME_TEMP_SCALE,  0.25f);
    s_noise->downfallMap.getRegion(d,    x, z, 1, 1, BIOME_DOWN_SCALE,  BIOME_DOWN_SCALE,  0.3333f);
    s_noise->noiseMap.getRegion(n,       x, z, 1, 1, BIOME_NOISE_SCALE, BIOME_NOISE_SCALE, 0.588f);
    float temp, down;
    biomeClimate(t[0], d[0], n[0], &temp, &down);
    return classifyBiome(temp, down);
}

float biomeTemperature(BiomeId b) {
    switch (b) {
        case B_RAIN:   return 1.2f;
        case B_SWAMP:  return 0.8f;
        case B_FOREST: return 0.7f;
        case B_SAVANNA: return 0.8f;
        case B_TAIGA:  return 0.05f;
        case B_DESERT: return 2.0f;
        case B_PLAINS: return 0.8f;
        case B_TUNDRA: return 0.05f;
        default:       return 0.5f;
    }
}

bool biomeHasCreatures(BiomeId b) { return b != B_DESERT; }
