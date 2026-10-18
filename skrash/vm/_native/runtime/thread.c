#include "thread.h"
#include "blocks/blocks.h"
#include <stddef.h>
#include <time.h>

static double sb3_now_seconds(void)
{
    return (double)time(NULL);
}

static void sb3_thread_finish_body(SB3Thread *thread, const SB3Block *next)
{
    while (next == NULL && thread->repeat_depth > 0) {
        SB3RepeatFrame *frame =
            &thread->repeat_stack[thread->repeat_depth - 1];

        if (frame->forever) {                 /* D'ABORD */
            thread->current_block = frame->body;
            return;
        }

        frame->remaining--;                   /* ENSUITE */

        if (frame->remaining > 0) {
            thread->current_block = frame->body;
            return;
        }

        const SB3Block *repeat_block = frame->repeat_block;
        thread->repeat_depth--;

        if (repeat_block->next != NULL)
            next = sb3_target_find_block(thread->target, repeat_block->next);
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

    if (thread->repeat_depth >= SB3_RUNTIME_MAX_REPEAT_DEPTH)
        return 0;

    int forever = (block->kind == SB3_BLOCK_KIND_CONTROL_FOREVER);
    int iterations = 0;

    if (block->kind == SB3_BLOCK_KIND_CONTROL_REPEAT) {
        double times = 0.0;

        if (!sb3_input_evaluate(thread, &block->input_times, &times))
            return 0;

        iterations = (int)times;

        if (iterations <= 0) {
            const SB3Block *next = NULL;

            if (block->next != NULL)
                next = sb3_target_find_block(thread->target, block->next);

            sb3_thread_finish_body(thread, next);
            return 1;
        }
    }

    if (block->substack == NULL) {
        const SB3Block *next = NULL;

        if (block->next != NULL)
            next = sb3_target_find_block(thread->target, block->next);

        sb3_thread_finish_body(thread, next);
        return 1;
    }

    const SB3Block *body =
        sb3_target_find_block(thread->target, block->substack);
    if (body == NULL)
        return 0;

    SB3RepeatFrame *frame = &thread->repeat_stack[thread->repeat_depth];
    frame->repeat_block = block;
    frame->body         = body;
    frame->forever      = forever;
    frame->remaining    = iterations;

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

        if (block->next != NULL) {
            next = sb3_target_find_block(
                thread->target,
                block->next
            );
        }

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

    if (!sb3_block_execute(thread, block)) {
        thread->running = 0;
        return 0;
    }

    if (thread->is_waiting)
        return 1;

    const SB3Block *next = NULL;

    if (block->next != NULL) {
        next = sb3_target_find_block(
            thread->target,
            block->next
        );
    }

    if (next != NULL) {
        thread->current_block = next;
    } else {
        sb3_thread_finish_body(thread, NULL);
    }

    return thread->running;
}