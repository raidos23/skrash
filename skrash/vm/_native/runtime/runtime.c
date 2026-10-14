#include "runtime.h"

#include <stdlib.h>
#include <string.h>

static void sb3_runtime_reset(SB3Runtime *runtime)
{
    if (runtime == NULL)
        return;

    runtime->project = NULL;
    runtime->threads = NULL;
    runtime->thread_count = 0;
}

int sb3_runtime_init(SB3Runtime *runtime, SB3Project *project)
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

    free(runtime->threads);
    sb3_runtime_reset(runtime);
}

int sb3_runtime_start(SB3Runtime *runtime)
{
    if (runtime == NULL || runtime->project == NULL)
        return 0;

    free(runtime->threads);
    runtime->threads = NULL;
    runtime->thread_count = 0;

    size_t count = 0;

    for (size_t i = 0;
         i < runtime->project->target_count;
         ++i) {
        SB3Target *target = &runtime->project->targets[i];

        if (sb3_target_find_block_by_kind(
                target,
                SB3_BLOCK_KIND_EVENT_WHEN_FLAG_CLICKED) != NULL)
            ++count;
    }

    if (count == 0)
        return 0;

    runtime->threads = calloc(
        count, sizeof(*runtime->threads));

    if (runtime->threads == NULL)
        return 0;

    for (size_t i = 0;
         i < runtime->project->target_count;
         ++i) {
        SB3Target *target = &runtime->project->targets[i];

        const SB3Block *hat = sb3_target_find_block_by_kind(
            target,
            SB3_BLOCK_KIND_EVENT_WHEN_FLAG_CLICKED);

        if (hat == NULL || hat->next == NULL)
            continue;

        const SB3Block *first =
            sb3_target_find_block(target, hat->next);

        if (first == NULL)
            continue;

        SB3Thread *thread =
            &runtime->threads[runtime->thread_count++];

        memset(thread, 0, sizeof(*thread));

        thread->target = target;
        thread->current_block = first;
        thread->running = 1;
    }

    return runtime->thread_count > 0;
}

int sb3_runtime_step(SB3Runtime *runtime)
{
    if (runtime == NULL || runtime->threads == NULL)
        return 0;

    int any_running = 0;

    for (size_t i = 0; i < runtime->thread_count; ++i) {
        SB3Thread *thread = &runtime->threads[i];

        if (!thread->running)
            continue;

        sb3_thread_step(thread);

        if (thread->running)
            any_running = 1;
    }

    return any_running;
}