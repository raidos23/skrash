#ifndef SKRASH_BLOCKS_H
#define SKRASH_BLOCKS_H

#include "../thread.h"

int sb3_block_execute(SB3Thread *thread, const SB3Block *block);

int sb3_motion_execute(SB3Thread *thread, const SB3Block *block);
int sb3_looks_execute(SB3Thread *thread, const SB3Block *block);
int sb3_control_execute(SB3Thread *thread, const SB3Block *block);
int sb3_events_execute(SB3Thread *thread, const SB3Block *block);

#endif