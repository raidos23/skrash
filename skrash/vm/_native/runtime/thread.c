#include "thread.h"

#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <time.h>

#define PI 3.14159265358979323846

static double sb3_now_seconds(void)
{
    return (double)time(NULL);
}

static int execute_block(SB3Thread *thread, const SB3Block *block)
{
    if (thread == NULL || thread->target == NULL || block == NULL)
        return 0;

    switch (block->kind) {
    case SB3_BLOCK_KIND_MOTION_MOVE_STEPS: {
        if (!block->has_motion_steps)
            return 0;

        double radians = thread->target->direction * PI / 180.0;
        thread->target->x += block->motion_steps * sin(radians);
        thread->target->y += block->motion_steps * cos(radians);
        return 1;
    }

    case SB3_BLOCK_KIND_EVENT_WHEN_FLAG_CLICKED:
    case SB3_BLOCK_KIND_CONTROL_REPEAT:
    case SB3_BLOCK_KIND_CONTROL_FOREVER:
        return 1;

    case SB3_BLOCK_KIND_MOTION_TURN_RIGHT:
        if (!block->has_motion_turn_degrees)
            return 0;

        thread->target->direction += block->motion_turn_degrees;
        while (thread->target->direction > 180.0)
            thread->target->direction -= 360.0;
        while (thread->target->direction <= -180.0)
            thread->target->direction += 360.0;
        return 1;

    case SB3_BLOCK_KIND_MOTION_TURN_LEFT:
        if (!block->has_motion_turn_degrees)
            return 0;

        thread->target->direction -= block->motion_turn_degrees;
        while (thread->target->direction > 180.0)
            thread->target->direction -= 360.0;
        while (thread->target->direction <= -180.0)
            thread->target->direction += 360.0;
        return 1;

    case SB3_BLOCK_KIND_LOOKS_SAY:
        printf("(say) %s\n", block->message ? block->message : "");
        return 1;

    case SB3_BLOCK_KIND_MOTION_GOTOXY:
        if (!block->has_motion_gotoxy)
            return 0;

        thread->target->x = block->motion_x;
        thread->target->y = block->motion_y;
        return 1;

    case SB3_BLOCK_KIND_MOTION_SETX:
        if (!block->has_motion_setx)
            return 0;

        thread->target->x = block->motion_x;
        return 1;

    case SB3_BLOCK_KIND_MOTION_SETY:
        if (!block->has_motion_sety)
            return 0;

        thread->target->y = block->motion_y;
        return 1;

    case SB3_BLOCK_KIND_CONTROL_WAIT:
        if (!block->has_wait_seconds)
            return 0;

        if (block->wait_seconds <= 0.0)
            return 1;

        thread->is_waiting = 1;
        thread->wait_until =
            sb3_now_seconds() + block->wait_seconds;
        return 1;

    case SB3_BLOCK_KIND_MOTION_POINTINDIRECTION:
        if (!block->has_motion_pointindirection)
            return 0;

        thread->target->direction = block->motion_direction;

        while (thread->target->direction > 180.0)
            thread->target->direction -= 360.0;
        while (thread->target->direction <= -180.0)
            thread->target->direction += 360.0;

        return 1;

    case SB3_BLOCK_KIND_MOTION_CHANGEXBY:
        if (!block->has_motion_setx)
            return 0;

        thread->target->x += block->motion_x;
        return 1;

    case SB3_BLOCK_KIND_MOTION_CHANGEYBY:
        if (!block->has_motion_sety)
            return 0;

        thread->target->y += block->motion_y;
        return 1;

    case SB3_BLOCK_KIND_LOOKS_SHOW:
        thread->target->visible = 1;
        return 1;

    case SB3_BLOCK_KIND_LOOKS_HIDE:
        thread->target->visible = 0;
        return 1;

    case SB3_BLOCK_KIND_LOOKS_SETSIZETO:
        if (!block->has_size_value)
            return 0;

        thread->target->size = block->size_value;
        return 1;

    case SB3_BLOCK_KIND_LOOKS_CHANGESIZEBY:
        if (!block->has_size_value)
            return 0;

        thread->target->size += block->size_value;
        return 1;

    case SB3_BLOCK_KIND_LOOKS_THINK:
        printf("(think) %s\n", block->message ? block->message : "");
        return 1;

    case SB3_BLOCK_KIND_UNKNOWN:
    default:
        return 0;
    }
}

static void sb3_thread_finish_body(SB3Thread *thread,
                                   const SB3Block *next)
{
    while (next == NULL && thread->repeat_depth > 0) {
        SB3RepeatFrame *frame =
            &thread->repeat_stack[thread->repeat_depth - 1];

        if (frame->forever) {
            thread->current_block = frame->body;
            return;
        }

        frame->remaining--;

        if (frame->remaining > 0) {
            thread->current_block = frame->body;
            return;
        }

        const SB3Block *repeat_block = frame->repeat_block;
        thread->repeat_depth--;

        if (repeat_block != NULL && repeat_block->next != NULL) {
            next = sb3_target_find_block(
                thread->target, repeat_block->next);
        }
    }

    thread->current_block = next;

    if (next == NULL)
        thread->running = 0;
}

static int sb3_thread_enter_repeat(SB3Thread *thread,
                                   const SB3Block *block)
{
    if (thread == NULL || block == NULL)
        return 0;

    if (block->kind == SB3_BLOCK_KIND_CONTROL_REPEAT &&
        !block->has_repeat_times)
        return 0;

    if (thread->repeat_depth >= SB3_RUNTIME_MAX_REPEAT_DEPTH)
        return 0;

    int forever = block->kind == SB3_BLOCK_KIND_CONTROL_FOREVER;
    int iterations = forever ? 0 : (int)block->repeat_times;

    if (block->kind == SB3_BLOCK_KIND_CONTROL_REPEAT &&
        iterations <= 0) {
        const SB3Block *next = NULL;

        if (block->next != NULL)
            next = sb3_target_find_block(
                thread->target, block->next);

        sb3_thread_finish_body(thread, next);
        return 1;
    }

    if (block->substack == NULL) {
        if (forever)
            return 0;

        const SB3Block *next = NULL;

        if (block->next != NULL)
            next = sb3_target_find_block(
                thread->target, block->next);

        sb3_thread_finish_body(thread, next);
        return 1;
    }

    const SB3Block *body =
        sb3_target_find_block(thread->target, block->substack);

    if (body == NULL)
        return 0;

    SB3RepeatFrame *frame =
        &thread->repeat_stack[thread->repeat_depth];

    frame->repeat_block = block;
    frame->body = body;
    frame->forever = forever;
    frame->remaining = iterations;

    thread->repeat_depth++;
    thread->current_block = body;

    return 1;
}

int sb3_thread_step(SB3Thread *thread)
{
    if (thread == NULL ||
        !thread->running ||
        thread->target == NULL ||
        thread->current_block == NULL)
        return 0;

    if (thread->is_waiting) {
        if (sb3_now_seconds() < thread->wait_until)
            return 1;

        thread->is_waiting = 0;
        thread->wait_until = 0.0;

        const SB3Block *block = thread->current_block;
        const SB3Block *next = NULL;

        if (block->next != NULL)
            next = sb3_target_find_block(
                thread->target, block->next);

        sb3_thread_finish_body(thread, next);
        return thread->running;
    }

    const SB3Block *block = thread->current_block;

    if (block->kind == SB3_BLOCK_KIND_CONTROL_REPEAT ||
        block->kind == SB3_BLOCK_KIND_CONTROL_FOREVER) {
        if (!sb3_thread_enter_repeat(thread, block)) {
            thread->running = 0;
            return 0;
        }

        return thread->running;
    }

    if (!execute_block(thread, block)) {
        thread->running = 0;
        return 0;
    }

    if (thread->is_waiting)
        return 1;

    const SB3Block *next = NULL;

    if (block->next != NULL)
        next = sb3_target_find_block(thread->target, block->next);

    if (next != NULL) {
        thread->current_block = next;
    } else {
        sb3_thread_finish_body(thread, NULL);
    }

    return thread->running;
}