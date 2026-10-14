#include "blocks.h"

#include <time.h>

static double sb3_now_seconds(void)
{
    return (double)time(NULL);
}

int sb3_control_execute(SB3Thread *thread, const SB3Block *block)
{
    if (thread == NULL || block == NULL)
        return 0;

    switch (block->kind) {
    case SB3_BLOCK_KIND_CONTROL_WAIT:
        if (!block->has_wait_seconds)
            return 0;

        if (block->wait_seconds <= 0.0)
            return 1;

        thread->is_waiting = 1;
        thread->wait_until =
            sb3_now_seconds() + block->wait_seconds;

        return 1;

    default:
        return 0;
    }
}