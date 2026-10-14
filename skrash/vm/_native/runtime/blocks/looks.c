#include "blocks.h"

#include <stdio.h>

int sb3_looks_execute(SB3Thread *thread, const SB3Block *block)
{
    if (thread == NULL || thread->target == NULL || block == NULL)
        return 0;

    SB3Target *target = thread->target;

    switch (block->kind) {
    case SB3_BLOCK_KIND_LOOKS_SAY:
        printf("(say) %s\n",
               block->message != NULL ? block->message : "");
        return 1;

    case SB3_BLOCK_KIND_LOOKS_THINK:
        printf("(think) %s\n",
               block->message != NULL ? block->message : "");
        return 1;

    case SB3_BLOCK_KIND_LOOKS_SHOW:
        target->visible = 1;
        return 1;

    case SB3_BLOCK_KIND_LOOKS_HIDE:
        target->visible = 0;
        return 1;

    case SB3_BLOCK_KIND_LOOKS_SETSIZETO:
        if (!block->has_size_value)
            return 0;

        target->size = block->size_value;
        return 1;

    case SB3_BLOCK_KIND_LOOKS_CHANGESIZEBY:
        if (!block->has_size_value)
            return 0;

        target->size += block->size_value;
        return 1;

    default:
        return 0;
    }
}