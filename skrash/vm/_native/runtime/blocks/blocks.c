#include "blocks.h"
#include "../../project/sb3.h"

#include "blocks.h"

#include <stdio.h>
#include <string.h>

// evaluation d'une entrée 
int sb3_input_evaluate(SB3Thread *thread, const SB3Input *in, double *out)
{
    if (thread == NULL || in == NULL || out == NULL)
        return 0;

    switch (in->kind) {

    case SB3_INPUT_KIND_LITERAL:
        *out = in->literal_number;
        return 1;

    case SB3_INPUT_KIND_BLOCK: {
        const SB3Block *inner =
            sb3_target_find_block(thread->target, in->block_id);
        if (inner != NULL && sb3_block_evaluate(thread, inner, out))
            return 1;
        *out = in->literal_number;   /* fallback shadow */
        return 1;
    }

    case SB3_INPUT_KIND_BLOCK_ONLY: {
        const SB3Block *inner =
            sb3_target_find_block(thread->target, in->block_id);
        if (inner == NULL) return 0;
        return sb3_block_evaluate(thread, inner, out);
    }

    default:
        return 0;
    }
}


// dispatch d'evaluation  
int sb3_block_evaluate(SB3Thread *thread, const SB3Block *block, double *out)
{
    if (thread == NULL || block == NULL || out == NULL)
        return 0;

    switch (block->kind) {

    case SB3_BLOCK_KIND_OPERATOR_ADD:
    case SB3_BLOCK_KIND_OPERATOR_SUBTRACT:
    case SB3_BLOCK_KIND_OPERATOR_MULTIPLY:
    case SB3_BLOCK_KIND_OPERATOR_DIVIDE:
    case SB3_BLOCK_KIND_OPERATOR_MOD:
    case SB3_BLOCK_KIND_OPERATOR_ROUND:
    case SB3_BLOCK_KIND_OPERATOR_RANDOM:
    case SB3_BLOCK_KIND_OPERATOR_LT:
    case SB3_BLOCK_KIND_OPERATOR_GT:
    case SB3_BLOCK_KIND_OPERATOR_EQUALS:
    case SB3_BLOCK_KIND_OPERATOR_AND:
    case SB3_BLOCK_KIND_OPERATOR_OR:
    case SB3_BLOCK_KIND_OPERATOR_NOT:
        return sb3_operator_evaluate(thread, block, out);

    default:
        return 0;
    }
}

int sb3_block_execute(SB3Thread *thread, const SB3Block *block)
{
    if (thread == NULL || thread->target == NULL || block == NULL)
        return 0;

    switch (block->kind) {
    case SB3_BLOCK_KIND_MOTION_MOVE_STEPS:
    case SB3_BLOCK_KIND_MOTION_TURN_RIGHT:
    case SB3_BLOCK_KIND_MOTION_TURN_LEFT:
    case SB3_BLOCK_KIND_MOTION_GOTOXY:
    case SB3_BLOCK_KIND_MOTION_SETX:
    case SB3_BLOCK_KIND_MOTION_SETY:
    case SB3_BLOCK_KIND_MOTION_POINTINDIRECTION:
    case SB3_BLOCK_KIND_MOTION_CHANGEXBY:
    case SB3_BLOCK_KIND_MOTION_CHANGEYBY:
        return sb3_motion_execute(thread, block);

    case SB3_BLOCK_KIND_LOOKS_SAY:
    case SB3_BLOCK_KIND_LOOKS_THINK:
    case SB3_BLOCK_KIND_LOOKS_SHOW:
    case SB3_BLOCK_KIND_LOOKS_HIDE:
    case SB3_BLOCK_KIND_LOOKS_SETSIZETO:
    case SB3_BLOCK_KIND_LOOKS_CHANGESIZEBY:
        return sb3_looks_execute(thread, block);

    case SB3_BLOCK_KIND_CONTROL_FOREVER:
    case SB3_BLOCK_KIND_CONTROL_REPEAT:
    case SB3_BLOCK_KIND_CONTROL_WAIT:
        return sb3_control_execute(thread, block);

    case SB3_BLOCK_KIND_EVENT_WHEN_FLAG_CLICKED:
        return sb3_events_execute(thread, block);

    default:
        return 0;
    }
}