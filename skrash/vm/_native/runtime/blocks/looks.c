#include "blocks.h"

#include <stdio.h>

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

    /* 1. Si on a un littéral string → on l’utilise */
    if (block->input_message.kind == SB3_INPUT_KIND_LITERAL &&
        block->input_message.literal_string != NULL) {
        printf("%s %s\n", prefix, block->input_message.literal_string);
        return 1;
    }

    /* 2. Si c’est un bloc (opérateur / reporter) → on évalue en nombre */
    if (block->input_message.kind == SB3_INPUT_KIND_BLOCK ||
        block->input_message.kind == SB3_INPUT_KIND_BLOCK_ONLY) {
        double value = 0.0;
        if (sb3_input_evaluate(thread, &block->input_message, &value)) {
            printf("%s %g\n", prefix, value);
            return 1;
        }
    }

    /* 3. Fallback sur l’ancien champ message */
    printf("%s %s\n", prefix,
           block->message != NULL ? block->message : "");
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

    default:
        return 0;
    }
}