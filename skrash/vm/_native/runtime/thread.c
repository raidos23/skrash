#include "thread.h"
#include "blocks/blocks.h"
#include <stddef.h>
#include <time.h>
#include "time_util.h"
#include <stdio.h>
#include "../project/sb3.h"
#include "runtime.h"
#include <math.h>

static void sb3_thread_finish_body(SB3Thread *thread, const SB3Block *next) {
    while (next == NULL && thread->repeat_depth > 0) {
        SB3RepeatFrame *frame =
            &thread->repeat_stack[thread->repeat_depth - 1];

        if (frame->forever) {
            thread->current_block = frame->body;
            return;
        }

        if (frame->remaining == -1) {
            thread->current_block = frame->repeat_block;
            return;
        }

        frame->remaining--;

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
                                   const SB3Block *block) {
    if (thread == NULL || block == NULL)
        return 0;

    if (thread->repeat_depth >= SB3_RUNTIME_MAX_REPEAT_DEPTH)
        return 0;

    int forever = (block->kind == SB3_BLOCK_KIND_CONTROL_FOREVER);
    int is_repeat_until = (block->kind == SB3_BLOCK_KIND_CONTROL_REPEATUNTIL);
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
    } else if (is_repeat_until) {
        double cond = 0.0;
        if (!sb3_input_evaluate(thread, &block->input_condition, &cond))
            return 0;

        if (cond != 0.0) {
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

    const SB3Block *body = sb3_target_find_block(thread->target, block->substack);
    if (body == NULL)
        return 0;

    SB3RepeatFrame *frame = &thread->repeat_stack[thread->repeat_depth];
    frame->repeat_block = block;
    frame->body = body;
    frame->forever = forever;
    frame->remaining = is_repeat_until ? -1 : iterations;

    thread->repeat_depth++;
    thread->current_block = body;

    return 1;
}

int sb3_thread_step(SB3Thread *thread) {

    if (thread->yield_next) {
        thread->yield_next = 0;
        return 1; // On rend la main, le scheduler nous rappellera
    }


    if (thread == NULL ||
            !thread->running ||
            thread->target == NULL ||
            thread->current_block == NULL)
        return 0;

    if (thread->waiting_for_broadcast) {
        int all_done = 1;
        for (size_t i = thread->broadcast_start_count; i < thread->runtime->thread_count; ++i) {
            if (thread->runtime->threads[i].running) {
                all_done = 0;
                break;
            }
        }
        if (all_done) {
            thread->waiting_for_broadcast = 0;
            // On avance au bloc suivant
            if (thread->current_block->next) {
                thread->current_block = sb3_target_find_block(thread->target, thread->current_block->next);
            } else {
                sb3_thread_finish_body(thread, NULL);
            }
        }

        return 1;
    }

    if (thread->is_waiting) {
        if (sb3_now_seconds() < thread->wait_until) return 1; // on attend encore

        thread->is_waiting = 0;

        // On est à la fin du bloc wait, on avance au suivant
        const SB3Block *block = thread->current_block;
        const SB3Block *next = NULL;
        if (block && block->next) next = sb3_target_find_block(thread->target, block->next);

        if (next) thread->current_block = next;
        else sb3_thread_finish_body(thread, NULL);

        return 1;
    }

    const SB3Block *block = thread->current_block;

    if (block->kind == SB3_BLOCK_KIND_CONTROL_REPEAT ||
            block->kind == SB3_BLOCK_KIND_CONTROL_FOREVER ||
            block->kind == SB3_BLOCK_KIND_CONTROL_REPEATUNTIL) {
        if (!sb3_thread_enter_repeat(thread, block)) {
            thread->running = 0;
            return 0;
        }

        return thread->running;
    }
    if (block->kind == SB3_BLOCK_KIND_CONTROL_IF || block->kind == SB3_BLOCK_KIND_CONTROL_IF_ELSE) {
        double cond = 0.0;
        if (!sb3_input_evaluate(thread, &block->input_condition, &cond)) {
            thread->running = 0;
            return 0;
        }
        const char *branch_id = NULL;
        if (cond != 0.0) {
            branch_id = block->substack;
        } else if (block->kind == SB3_BLOCK_KIND_CONTROL_IF_ELSE) {
            branch_id = block->substack_else;
        }

        const SB3Block *branch = NULL;
        if (branch_id != NULL)
            branch = sb3_target_find_block(thread->target, branch_id);

        if (branch != NULL) {
            if (thread->repeat_depth >= SB3_RUNTIME_MAX_REPEAT_DEPTH) {
                thread->running = 0;
                return 0;
            }

            SB3RepeatFrame *frame =
                &thread->repeat_stack[thread->repeat_depth];
            frame->repeat_block = block;
            frame->body         = branch;
            frame->forever      = 0;
            frame->remaining    = 1;

            thread->repeat_depth++;
            thread->current_block = branch;
            return 1;
        }

        // pas de branche : on saute au next du if
        const SB3Block *next = NULL;
        if (block->next != NULL)
            next = sb3_target_find_block(thread->target, block->next);

        sb3_thread_finish_body(thread, next);
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