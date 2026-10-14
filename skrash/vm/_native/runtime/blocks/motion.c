#include "blocks.h"

#include <math.h>

#define PI 3.14159265358979323846

static void normalize_direction(SB3Target *target)
{
    while (target->direction > 180.0)
        target->direction -= 360.0;

    while (target->direction <= -180.0)
        target->direction += 360.0;
}

int sb3_motion_execute(SB3Thread *thread, const SB3Block *block)
{
    if (thread == NULL || thread->target == NULL || block == NULL)
        return 0;

    SB3Target *target = thread->target;

    switch (block->kind) {
    case SB3_BLOCK_KIND_MOTION_MOVE_STEPS: {
        if (!block->has_motion_steps)
            return 0;

        double radians = target->direction * PI / 180.0;

        target->x += block->motion_steps * sin(radians);
        target->y += block->motion_steps * cos(radians);

        return 1;
    }

    case SB3_BLOCK_KIND_MOTION_TURN_RIGHT:
        if (!block->has_motion_turn_degrees)
            return 0;

        target->direction += block->motion_turn_degrees;
        normalize_direction(target);
        return 1;

    case SB3_BLOCK_KIND_MOTION_TURN_LEFT:
        if (!block->has_motion_turn_degrees)
            return 0;

        target->direction -= block->motion_turn_degrees;
        normalize_direction(target);
        return 1;

    case SB3_BLOCK_KIND_MOTION_GOTOXY:
        if (!block->has_motion_gotoxy)
            return 0;

        target->x = block->motion_x;
        target->y = block->motion_y;
        return 1;

    case SB3_BLOCK_KIND_MOTION_SETX:
        if (!block->has_motion_setx)
            return 0;

        target->x = block->motion_x;
        return 1;

    case SB3_BLOCK_KIND_MOTION_SETY:
        if (!block->has_motion_sety)
            return 0;

        target->y = block->motion_y;
        return 1;

    case SB3_BLOCK_KIND_MOTION_POINTINDIRECTION:
        if (!block->has_motion_pointindirection)
            return 0;

        target->direction = block->motion_direction;
        normalize_direction(target);
        return 1;

    case SB3_BLOCK_KIND_MOTION_CHANGEXBY:
        if (!block->has_motion_setx)
            return 0;

        target->x += block->motion_x;
        return 1;

    case SB3_BLOCK_KIND_MOTION_CHANGEYBY:
        if (!block->has_motion_sety)
            return 0;

        target->y += block->motion_y;
        return 1;

    default:
        return 0;
    }
}