
#ifndef MCPSP_WORLD_ENTITY_AI_SENSING_H
#define MCPSP_WORLD_ENTITY_AI_SENSING_H

class Mob;
class Entity;

class Sensing {
public:
    Sensing(Mob* holder) : holder(holder), count(0) {}
    bool canSee(Entity* e);
    void clear() { count = 0; }

private:
    Mob* holder;

    static const int SEEN_MAX = 8;
    int  ids[SEEN_MAX];
    bool seen[SEEN_MAX];
    int  count;
};

#endif
