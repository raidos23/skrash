#include "blocks.h"

#include "../time_util.h"

int sb3_control_execute(SB3Thread *thread, const SB3Block *block)
{
    if (thread == NULL || block == NULL)
        return 0;

    switch (block->kind) {

    case SB3_BLOCK_KIND_CONTROL_WAIT: {
        double seconds;

        if (!sb3_input_evaluate(thread, &block->input_duration, &seconds))
            return 0;

        if (seconds <= 0.0)
            return 1;

        thread->is_waiting = 1;
        thread->wait_until = sb3_now_seconds() + seconds;
        return 1;
    }

    default:
        return 0;
    }
}