#include "blocks.h"
#include <stdio.h>
#include <math.h>

#define PI 3.14159265358979323846

static void normalize_direction(SB3Target *target) {
    while (target->direction > 180.0)
        target->direction -= 360.0;

    while (target->direction <= -180.0)
        target->direction += 360.0;
}

/* Empêche le sprite de sortir de la scène */
static void keep_in_fence(SB3Target *target) {
    /* Scratch bloque le CENTRE du sprite, pas ses bords ( c'est pas exactement cela mais bon ) */
    if (target->x < -240.0) target->x = -240.0;
    if (target->x >  240.0) target->x =  240.0;

    if (target->y < -180.0) target->y = -180.0;
    if (target->y >  180.0) target->y =  180.0;
}

int sb3_motion_execute(SB3Thread *thread, const SB3Block *block) {
    if (thread == NULL || thread->target == NULL || block == NULL)
        return 0;

    SB3Target *target = thread->target;

    switch (block->kind) {
    case SB3_BLOCK_KIND_MOTION_MOVE_STEPS: {
        double steps;
        if (!sb3_input_evaluate(thread, &block->input_steps, &steps))
            return 0;
        double radians = target->direction * PI / 180.0;
        target->x += steps * sin(radians);
        target->y += steps * cos(radians);
        keep_in_fence(target);

        return 1;
    }

    case SB3_BLOCK_KIND_MOTION_TURN_RIGHT: {
        double degrees;
        if (!sb3_input_evaluate(thread, &block->input_degrees, &degrees))
            return 0;
        target->direction += degrees;
        normalize_direction(target);
        return 1;
    }

    case SB3_BLOCK_KIND_MOTION_TURN_LEFT: {
        double degrees;
        if (!sb3_input_evaluate(thread, &block->input_degrees, &degrees))
            return 0;
        target->direction -= degrees;
        normalize_direction(target);
        return 1;
    }

    case SB3_BLOCK_KIND_MOTION_GOTOXY: {
        double x, y;
        if (!sb3_input_evaluate(thread, &block->input_x, &x))
            return 0;
        if (!sb3_input_evaluate(thread, &block->input_y, &y))
            return 0;
        target->x = x;
        target->y = y;
        keep_in_fence(target);
        return 1;
    }

    case SB3_BLOCK_KIND_MOTION_SETX: {
        double x;
        if (!sb3_input_evaluate(thread, &block->input_x, &x))
            return 0;
        target->x = x;
        keep_in_fence(target);
        return 1;
    }

    case SB3_BLOCK_KIND_MOTION_SETY: {
        double y;
        if (!sb3_input_evaluate(thread, &block->input_y, &y))
            return 0;
        target->y = y;
        keep_in_fence(target);
        return 1;
    }

    case SB3_BLOCK_KIND_MOTION_POINTINDIRECTION: {
        double direction;
        if (!sb3_input_evaluate(thread, &block->input_direction, &direction))
            return 0;
        target->direction = direction;
        normalize_direction(target);
        return 1;
    }

    case SB3_BLOCK_KIND_MOTION_CHANGEXBY: {
        double dx;
        if (!sb3_input_evaluate(thread, &block->input_x, &dx))
            return 0;
        target->x += dx;
        keep_in_fence(target);
        return 1;
    }

    case SB3_BLOCK_KIND_MOTION_CHANGEYBY: {
        double dy;
        if (!sb3_input_evaluate(thread, &block->input_y, &dy))
            return 0;
        target->y += dy;
        keep_in_fence(target);
        return 1;
    }

    default:
        return 0;
    }
}

int sb3_motion_evaluate(SB3Thread *thread, const SB3Block *block, double *out) {
    if (thread == NULL || block == NULL || out == NULL)
        return 0;

    switch (block->kind) {

    case SB3_BLOCK_KIND_MOTION_XPOSITION:
        *out = thread->target->x;
        return 1;

    case SB3_BLOCK_KIND_MOTION_YPOSITION:
        *out = thread->target->y;
        return 1;

    case SB3_BLOCK_KIND_MOTION_DIRECTION:
        *out = thread->target->direction;
        return 1;

    default:
        return 0;
    }
}