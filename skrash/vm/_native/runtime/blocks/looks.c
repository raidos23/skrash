#include "blocks.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
int sb3_looks_execute(SB3Thread *thread, const SB3Block *block)
{
    if (thread == NULL || thread->target == NULL || block == NULL)
        return 0;

    SB3Target *target = thread->target;

    switch (block->kind) {
    case SB3_BLOCK_KIND_LOOKS_SAY:
    case SB3_BLOCK_KIND_LOOKS_THINK: {
    const char *prefix = (block->kind == SB3_BLOCK_KIND_LOOKS_SAY)
                         ? "(say)" : "(think)";

    char buf[512];
    if (!sb3_input_evaluate_string(thread, &block->input_message,
                                   buf, sizeof(buf))) {
        printf("%s \n", prefix);
        return 1;
    }

    printf("%s %s\n", prefix, buf);
    return 1;
}

    case SB3_BLOCK_KIND_LOOKS_SHOW:
        target->visible = 1;
        return 1;

    case SB3_BLOCK_KIND_LOOKS_HIDE:
        target->visible = 0;
        return 1;

    case SB3_BLOCK_KIND_LOOKS_SETSIZETO: {
        double size;
        if (!sb3_input_evaluate(thread, &block->input_size, &size))
            return 0;
        target->size = size;
        return 1;
    }

    case SB3_BLOCK_KIND_LOOKS_CHANGESIZEBY: {
        double change;
        if (!sb3_input_evaluate(thread, &block->input_change, &change))
            return 0;
        target->size += change;
        return 1;
    }

    case SB3_BLOCK_KIND_LOOKS_NEXTCOSTUME:
    if (target->costume_count > 0) {
        target->current_costume = (target->current_costume + 1) % target->costume_count;
    }
    return 1;
    
    default:
        return 0;
    }
}
