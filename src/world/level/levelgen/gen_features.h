
#ifndef MCPSP_WORLD_LEVEL_LEVELGEN_GEN_FEATURES_H
#define MCPSP_WORLD_LEVEL_LEVELGEN_GEN_FEATURES_H

enum { GEN_FEATURE_CAVES = 0, GEN_FEATURE_VEGETATION = 1, GEN_FEATURE_COUNT = 2 };

struct GenFeatureDef {
    const char* label;
    bool        defaultOn;
};
extern const GenFeatureDef kGenFeatures[GEN_FEATURE_COUNT];

#define GEN_FEATURES_ALL_ON (-1)

int  genFeaturesDefaultMask();
bool genFeatureEnabled(int mask, int feature);
int  genFeatureToggled(int mask, int feature);

#endif
