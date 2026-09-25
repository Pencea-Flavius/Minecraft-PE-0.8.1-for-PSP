
#include "world/entity/ai/jump_control.h"
#include "world/entity/mob.h"

void JumpControl::tick() {
    mob->jumping = jumping;
    jumping = false;
}
