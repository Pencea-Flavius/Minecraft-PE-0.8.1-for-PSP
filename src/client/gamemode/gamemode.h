#pragma once

struct BlockHit;
struct ItemInstance;

class GameMode {
public:
    virtual ~GameMode() {}
    virtual bool isCreative() = 0;
    virtual void handleInput(unsigned int pressed, unsigned int held);

    virtual bool useItemOn(ItemInstance* item, const BlockHit& hit, bool* usedItem = 0);
};

class CreativeMode : public GameMode {
public:
    bool isCreative() { return true; }
};

class SurvivalMode : public GameMode {
public:
    bool isCreative() { return false; }

};

struct MiningState { bool active; int x, y, z; float progress; };
extern MiningState g_mining;

void breakRefuse(const char* why);
extern unsigned int g_breakRefuse, g_breakRefuseFirstMin;
extern const char*  g_breakRefuseWhy;
extern const char*  g_breakRefuseFirst;

extern GameMode* g_gameMode;
void gameModeInit(int gameType);
void gameModeShutdown();

extern int g_useItemDelay;
static const int USE_ITEM_DELAY_TICKS = 8;

void gameModeHandleInput(unsigned int pressed, unsigned int held);

void playerDropSelected(bool all);

struct CrosshairTarget { const char* useLabel; const char* breakLabel; };
CrosshairTarget gameModeCrosshairTarget();
