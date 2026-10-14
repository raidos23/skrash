#include "blocks.h"

int sb3_events_execute(SB3Thread *thread, const SB3Block *block)
{
    if (thread == NULL || block == NULL)
        return 0;

    switch (block->kind) {
    case SB3_BLOCK_KIND_EVENT_WHEN_FLAG_CLICKED:
        return 1;

    default:
        return 0;
    }
}