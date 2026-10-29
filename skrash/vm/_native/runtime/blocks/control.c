#include "blocks.h"
#include "../../project/sb3.h"
#include "../runtime.h"
#include <math.h>
#include <string.h>
#include <stdio.h>
#include "../time_util.h"

int sb3_control_execute(SB3Thread *thread, const SB3Block *block)
{
    if (thread == NULL || block == NULL)
        return 0;

    switch (block->kind) {

    case SB3_BLOCK_KIND_CONTROL_WAIT: {
        double seconds;

        if (!sb3_input_evaluate(thread, &block->input_duration, &seconds))
            return 0;

        if (seconds <= 0.0)
            return 1;

        thread->is_waiting = 1;
        thread->wait_until = sb3_now_seconds() + seconds;
        return 1;
    }

    case SB3_BLOCK_KIND_CONTROL_CREATECLONEOF: {
    SB3Target *to_clone = NULL;
    if (block->message && strcmp(block->message, "_myself_") == 0) {
        to_clone = thread->target;
    } else if (block->message) {
        /* Chercher le sprite par son nom */
        for (size_t i = 0; i < thread->runtime->project->target_count; ++i) {
            SB3Target *t = &thread->runtime->project->targets[i];
            if (t->name && strcmp(t->name, block->message) == 0) {
                to_clone = t;
                break;
            }
        }
    }
    
    if (to_clone) {
        sb3_runtime_create_clone(thread->runtime, to_clone);
    }
    return 1;
}

    case SB3_BLOCK_KIND_CONTROL_DELETETHISCLONE: {
    if (thread->target->is_clone) {
        thread->target->visible = 0; /* On le cache du renderer */
        thread->running = 0;         /* On tue le thread */
    }
    return 1;
    }

    case SB3_BLOCK_KIND_CONTROL_STOP: {
    if (block->stop_option) {
        if (strcmp(block->stop_option, "all") == 0) {
            for (size_t i = 0; i < thread->runtime->thread_count; ++i) {
                thread->runtime->threads[i].running = 0;
            }
        } else if (strcmp(block->stop_option, "this script") == 0) {
            thread->running = 0;
        } else if (strcmp(block->stop_option, "other scripts in sprite") == 0) {
            for (size_t i = 0; i < thread->runtime->thread_count; ++i) {
                if (&thread->runtime->threads[i] != thread &&
                    thread->runtime->threads[i].target == thread->target) {
                    thread->runtime->threads[i].running = 0;
                }
            }
        }
    }
    return 1;
    }

    default:
        return 0;
    }
}