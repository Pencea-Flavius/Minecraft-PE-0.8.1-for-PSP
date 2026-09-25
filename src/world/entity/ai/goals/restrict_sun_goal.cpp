#include "world/entity/ai/goals/restrict_sun_goal.h"
#include "world/entity/path_finder_mob.h"
#include "world/level/level.h"
#include "world/level/pathfinder/path_navigation.h"

bool RestrictSunGoal::canUse() { return mob->level->isDay(); }
void RestrictSunGoal::start()  { mob->getNavigation()->setAvoidSun(true); }
void RestrictSunGoal::stop()   { mob->getNavigation()->setAvoidSun(false); }
