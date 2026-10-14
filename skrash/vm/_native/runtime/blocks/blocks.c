#include "blocks.h"
#include "../../project/sb3.h"

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