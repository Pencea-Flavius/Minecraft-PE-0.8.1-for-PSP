
#ifndef MCPSP_GAMEMODE_CLICK_REPEAT_H
#define MCPSP_GAMEMODE_CLICK_REPEAT_H

#include <cmath>

#include "platform/time.h"

static const unsigned int CLICK_REPEAT_US = 250000;

struct ClickRepeat {
    unsigned int nextUs;
    bool  drag;
    float ax, ay, az;

    void release()                     { nextUs = 0; drag = false; }

    void arm(unsigned int now)          { nextUs = now + CLICK_REPEAT_US; if (!nextUs) nextUs = 1; }
    void pressed(unsigned int now)      { arm(now); }
    void placed(bool on, float x, float y, float z) {
        drag = on;
        if (on) { ax = x; ay = y; az = z; }
    }

    bool repeatDue(unsigned int now, float x, float y, float z) {
        if (!nextUs || !timeReached(now, nextUs)) return false;
        if (drag) {

            float dx = x - ax, dy = y - ay, dz = z - az;
            if (std::fabs(dx) >= 1.0f)      ax += dx < 0 ? std::ceil(dx) : std::floor(dx);
            else if (std::fabs(dy) >= 1.0f) ay += dy < 0 ? std::ceil(dy) : std::floor(dy);
            else if (std::fabs(dz) >= 1.0f) az += dz < 0 ? std::ceil(dz) : std::floor(dz);
            else return false;
        }
        arm(now);
        return true;
    }
};

#endif
