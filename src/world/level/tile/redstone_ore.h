
#pragma once

struct World;

static const int REDSTONE_LIT_DELAY = 30;

void redstoneOreInteract(World* w, int x, int y, int z);

void redstoneOreRevert(World* w, int x, int y, int z);
