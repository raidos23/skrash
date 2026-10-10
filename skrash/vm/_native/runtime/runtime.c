#include "runtime.h"
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#define PI 3.14159265358979323846
static void sb3_runtime_reset(SB3Runtime *runtime)
{
    if (runtime == NULL)
        return;

    runtime->project = NULL;
    runtime->target = NULL;
    runtime->current_block = NULL;
    runtime->running = 0;
    runtime ->repeat_depth = 0;
}

int sb3_runtime_init(
    SB3Runtime *runtime,
    SB3Project *project)
{
    if (runtime == NULL || project == NULL)
        return 0;

    sb3_runtime_reset(runtime);

    runtime->project = project;

    return 1;
}

void sb3_runtime_free(SB3Runtime *runtime)
{
    if (runtime == NULL)
        return;

    /*
     * Le runtime ne possède pas le projet.
     */
    sb3_runtime_reset(runtime);
}

static SB3Target *find_target(SB3Runtime *runtime)
{
    size_t i;

    if (runtime == NULL || runtime->project == NULL)
        return NULL;

    for (i = 0; i < runtime->project->target_count; ++i) {
        SB3Target *target =
            &runtime->project->targets[i];

        if (!target->is_stage)
            return target;
    }

    return NULL;
}

int sb3_runtime_start(SB3Runtime *runtime)
{
    SB3Target *target;
    const SB3Block *event;

    if (runtime == NULL || runtime->project == NULL)
        return 0;

    target = find_target(runtime);

    if (target == NULL)
        return 0;

    event = sb3_target_find_block_by_kind(
        target,
        SB3_BLOCK_KIND_EVENT_WHEN_FLAG_CLICKED
    );

    if (event == NULL)
        return 0;

    runtime->target = target;

    if (event->next != NULL) {
        runtime->current_block =
            sb3_target_find_block(
                target,
                event->next
            );
    } else {
        runtime->current_block = NULL;
    }

    runtime->running =
        runtime->current_block != NULL;

    return 1;
}

static int execute_block(
    SB3Runtime *runtime,
    const SB3Block *block)
{
    if (runtime == NULL ||
        runtime->target == NULL ||
        block == NULL)
        return 0;

    switch (block->kind) {
    
    case SB3_BLOCK_KIND_MOTION_MOVE_STEPS:
    if (!block->has_motion_steps)
        return 0;
    /*
    La direction du sprite est exprimée en degrés dans Scratch,
    tandis que sin() et cos() attendent un angle en radians
    on convertit donc la direction avant de calculer le déplacement
    */
    double radians =
        runtime->target->direction
        * PI / 180.0;
    /*
         * V1 :
         * on simplifie le déplacement
         * en modifiant directement X.
         */
    runtime->target->x +=
        block->motion_steps * sin(radians);

    runtime->target->y +=
        block->motion_steps * cos(radians);
        
        return 1;

    case SB3_BLOCK_KIND_EVENT_WHEN_FLAG_CLICKED:
        return 1;
    /*Scratch normalise la direction du sprite dans l'intervalle ]-180°, 180°] 
    Une rotation à droite augmente la direction avant normalisation */
    case SB3_BLOCK_KIND_MOTION_TURN_RIGHT:
    if (!block->has_motion_turn_degrees)
        return 0;

    runtime->target->direction +=
        block->motion_turn_degrees;

    while (runtime->target->direction > 180.0)
        runtime->target->direction -= 360.0;

    while (runtime->target->direction <= -180.0)
        runtime->target->direction += 360.0;

    return 1;

    case SB3_BLOCK_KIND_MOTION_TURN_LEFT:
    if (!block->has_motion_turn_degrees)
        return 0;

    runtime->target->direction -=
        block->motion_turn_degrees;

    while (runtime->target->direction > 180.0)
        runtime->target->direction -= 360.0;

    while (runtime->target->direction <= -180.0)
        runtime->target->direction += 360.0;

    return 1;

    case SB3_BLOCK_KIND_CONTROL_REPEAT:
    return 1;

    case SB3_BLOCK_KIND_LOOKS_SAY:
        printf("%s\n", block->message ? block->message : "");
    return 1;

    case SB3_BLOCK_KIND_CONTROL_FOREVER:
    return 1;

    case SB3_BLOCK_KIND_UNKNOWN:
    default:
        return 0;
    }
}

static void sb3_runtime_finish_body( SB3Runtime *runtime, const SB3Block *next)
{
    while (next == NULL && runtime->repeat_depth > 0) {
        SB3RepeatFrame *frame =
            &runtime->repeat_stack[runtime->repeat_depth - 1];

        if (frame->forever) {
            runtime->current_block = frame->body;
            return;
        }
        frame->remaining--;
        
        if (frame->remaining > 0) {
            runtime->current_block = frame->body;
            return;
        }

        const SB3Block *repeat_block = frame->repeat_block;

        runtime->repeat_depth--;

        next = NULL;

        if (repeat_block->next != NULL) {
            next = sb3_target_find_block(
                runtime->target,
                repeat_block->next
            );
        }
    }

    runtime->current_block = next;

    if (next == NULL)
        runtime->running = 0;
}

static int sb3_runtime_enter_repeat( SB3Runtime *runtime, const SB3Block *block)
{
    const SB3Block *body;
    int iterations;

    if (block == NULL)
        return 0;

    if (block->kind == SB3_BLOCK_KIND_CONTROL_REPEAT && !block->has_repeat_times)
    return 0; 

    if (runtime->repeat_depth >= SB3_RUNTIME_MAX_REPEAT_DEPTH)
        return 0;

    iterations = block->kind == SB3_BLOCK_KIND_CONTROL_FOREVER ? 0 : (int)block->repeat_times;

    if ((block->kind == SB3_BLOCK_KIND_CONTROL_REPEAT && iterations <= 0) || block->substack == NULL) {
        const SB3Block *next = NULL;

        if (block->next != NULL) {
            next = sb3_target_find_block(
                runtime->target,
                block->next
            );
        }

        sb3_runtime_finish_body(runtime, next);
        return 1;
    }

    body = sb3_target_find_block(
        runtime->target,
        block->substack
    );

    if (body == NULL)
        return 0;

    SB3RepeatFrame *frame =
        &runtime->repeat_stack[runtime->repeat_depth];

    frame->repeat_block = block;
    frame->body = body;
    frame->forever = block->kind == SB3_BLOCK_KIND_CONTROL_FOREVER;
    frame->remaining = frame->forever ? 0 : iterations;


    runtime->repeat_depth++;
    runtime->current_block = body;

    return 1;
}

int sb3_runtime_step(SB3Runtime *runtime)
{
    const SB3Block *block;
    const SB3Block *next = NULL;

    if (runtime == NULL ||
        !runtime->running ||
        runtime->current_block == NULL)
        return 0;

    block = runtime->current_block;

    if (block->kind == SB3_BLOCK_KIND_CONTROL_REPEAT) {
        if (!sb3_runtime_enter_repeat(runtime, block)) {
            runtime->running = 0;
            return 0;
        }
        return 1;
    }
    if (block->kind == SB3_BLOCK_KIND_CONTROL_FOREVER) {
    if (!sb3_runtime_enter_repeat(runtime, block)) {
        runtime->running = 0;
        return 0;
    }
    return 1;
}

    if (!execute_block(runtime, block)) {
        runtime->running = 0;
        return 0; 
    }

    if (block->next != NULL) {
        next = sb3_target_find_block(
            runtime->target,
            block->next
        );
    }

    if (next != NULL) {
        runtime->current_block = next;
    } else {
        sb3_runtime_finish_body(runtime, NULL);
    }

    return 1;
}