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
    case SB3_BLOCK_KIND_LOOKS_SAY: {
    char buf[512];
    if (!sb3_input_evaluate_string(thread, &block->input_message, buf, sizeof(buf)))
        return 0;
    
    printf("[%s] (say) %s\n", 
           thread->target->is_clone ? "CLONE" : thread->target->name, 
           buf);
    return 1;
    }
    case SB3_BLOCK_KIND_LOOKS_THINK: {
    char buf[512];
    if (!sb3_input_evaluate_string(thread, &block->input_message, buf, sizeof(buf)))
        return 0;
    printf("[%s] (think) %s\n", 
           thread->target->is_clone ? "CLONE" : thread->target->name, 
           buf);
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
    case SB3_BLOCK_KIND_LOOKS_SWITCHCOSTUMETO: {
    const char *costume_name = block->message;
    
    /* Si le nom est NULL, on va le chercher dans le shadow */
    if (costume_name == NULL && block->event_option != NULL) {
        const SB3Block *inner = sb3_target_find_block(thread->target, block->event_option);
        if (inner && inner->message) {
            costume_name = inner->message;
        }
    }
    
    if (!costume_name) {
        /* On affiche un message si on ne trouve pas (pour debug) */
        fprintf(stderr, "[COSTUME] Switch to NULL (shadow ID: %s)\n", 
                block->event_option ? block->event_option : "(null)");
        return 1;
    }
    
    fprintf(stderr, "[COSTUME] Switch to '%s'\n", costume_name);
    
    int found = 0;
    for (size_t i = 0; i < target->costume_count; ++i) {
        if (target->costumes[i].name && strcmp(target->costumes[i].name, costume_name) == 0) {
            target->current_costume = i;
            found = 1;
            break;
        }
    }
    
    if (!found) {
        int idx = atoi(costume_name);
        if (idx > 0 && idx <= (int)target->costume_count) {
            target->current_costume = idx - 1;
        }
    }
    return 1;
}
   case SB3_BLOCK_KIND_LOOKS_COSTUME:
    return 1;
    default:
        return 0;
    }
}

int sb3_looks_evaluate(SB3Thread *thread, const SB3Block *block, double *out)
{
    if (thread == NULL || block == NULL || out == NULL)
        return 0;


    switch (block->kind) {

        default:
            return 0;
    }
}