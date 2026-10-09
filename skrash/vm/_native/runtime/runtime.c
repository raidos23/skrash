#include "runtime.h"

#include <stddef.h>

static void sb3_runtime_reset(SB3Runtime *runtime)
{
    if (runtime == NULL)
        return;

    runtime->project = NULL;
    runtime->target = NULL;
    runtime->current_block = NULL;
    runtime->running = 0;
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
         * V1 :
         * on simplifie le déplacement
         * en modifiant directement X.
         */
        runtime->target->x +=
            block->motion_steps;

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

    while (runtime->target->direction < 180.0)
        runtime->target->direction += 360.0;

    while (runtime->target->direction >= 180.0)
        runtime->target->direction -= 360.0;

    return 1;


    case SB3_BLOCK_KIND_UNKNOWN:
    default:
        return 0;
    }
}

int sb3_runtime_step(SB3Runtime *runtime)
{
    const SB3Block *next;

    if (runtime == NULL ||
        !runtime->running ||
        runtime->current_block == NULL)
        return 0;

    if (!execute_block(
            runtime,
            runtime->current_block))
        return 0;

    next = NULL;

    if (runtime->current_block->next != NULL) {
        next = sb3_target_find_block(
            runtime->target,
            runtime->current_block->next
        );
    }

    runtime->current_block = next;

    if (next == NULL)
        runtime->running = 0;

    return 1;
}