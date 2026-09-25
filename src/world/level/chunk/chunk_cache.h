
#ifndef MCPSP_WORLD_CHUNK_CACHE_H
#define MCPSP_WORLD_CHUNK_CACHE_H

struct World;

void worldGetChunk(World* w, int cx, int cz);

void worldEnsureArea(World* w, int cx, int cz, int r);

int worldStream(World* w, float px, float pz, int budgetMs);

bool worldStreamBusy();

void worldSaveResident(World* w);

void worldGenWorkerStart(World* w);
void worldGenWorkerStop();

extern unsigned int g_streamIn, g_streamOut;

#endif
