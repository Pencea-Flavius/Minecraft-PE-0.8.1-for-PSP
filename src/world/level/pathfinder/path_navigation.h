
#ifndef MCPSP_WORLD_LEVEL_PATHFINDER_PATH_NAVIGATION_H
#define MCPSP_WORLD_LEVEL_PATHFINDER_PATH_NAVIGATION_H

#include "world/level/pathfinder/vec3.h"

class Mob;
class Level;
class Path;

class PathNavigation {
public:
    PathNavigation(Mob* mob, Level* level, float range);

    bool isDone() const;
    void stop();
    void tick();

    bool moveTo(Mob* target, float speed);
    bool moveTo(float x, float y, float z, float speed);

    void setSpeed(float s) { speedMultiplier = s; }
    void setAvoidSun(bool v) { avoidSun = v; }
    void setAvoidWater(bool v) { avoidWater = v; }
    bool getAvoidWater() const { return avoidWater; }
    void setCanFloat(bool v) { canFloat = v; }
    void setCanOpenDoors(bool v) { _canOpenDoors = v; }
    void setCanPassDoors(bool v) { _canPassDoors = v; }
    bool canOpenDoors() const { return _canOpenDoors; }
    bool canPassDoors() const { return _canPassDoors; }

private:
    bool canUpdatePath() const;
    bool isInLiquid() const;
    int  getSurfaceY() const;
    Vec3 getTempMobPos() const;
    void updatePath();
    void trimPathFromSun();

    bool adoptPath(float speed);
    bool canMoveDirectly(const Vec3& from, const Vec3& to, int xs, int ys, int zs) const;
    bool canWalkOn(int x, int y, int z, int xs, int ys, int zs,
                   const Vec3& from, float dirX, float dirZ) const;
    bool canWalkAbove(int x, int y, int z, int xs, int ys, int zs,
                      const Vec3& from, float dirX, float dirZ) const;

    Path& path() const;

    Mob*   mob;
    Level* level;
    float  range;
    float  speedMultiplier;
    bool   avoidSun;
    int    tickCounter, lastProgressTick;
    Vec3   lastProgressPos;
    bool   _canPassDoors, _canOpenDoors, avoidWater, canFloat;
};

#endif
