#include <stdio.h>
#include <stdlib.h>
#include "../skrash/vm/_native/project/sb3.h"
#include "../skrash/vm/_native/runtime/runtime.h"

#define MAX_TEST_STEPS 1000

int main(void)
{
    SB3Project *project;
    SB3Runtime runtime;

    project = sb3_load("./tests/checks/looks.sb3");
    if (project == NULL) {
        fprintf(stderr, "error: failed to load looks.sb3\n");
        return 1;
    }

    printf("project loaded: %zu targets\n", project->target_count);

    if (!sb3_runtime_init(&runtime, project)) {
        fprintf(stderr, "error: failed to initialize runtime\n");
        sb3_project_free(project);
        return 1;
    }

    if (!sb3_runtime_start(&runtime)) {
        fprintf(stderr, "error: no executable script found\n");
        sb3_runtime_free(&runtime);
        sb3_project_free(project);
        return 1;
    }

    /* Memoire du dernier bloc affiche par thread. */
    const SB3Block **last_seen = calloc(SB3_MAX_THREADS, sizeof(*last_seen));
    if (last_seen == NULL) {
        sb3_runtime_free(&runtime);
        sb3_project_free(project);
        return 1;
    }

    /*
     * Some Scratch projects intentionally contain forever loops.
     * A test must advance a bounded number of frames instead of waiting
     * for such a project to finish naturally.
     */
    size_t steps = 0;
    int runtime_active = 1;

    while (steps < MAX_TEST_STEPS && runtime_active) {
        runtime_active = sb3_runtime_step(&runtime);
        ++steps;

        for (size_t i = 0; i < runtime.thread_count; ++i) {
            SB3Thread *th = &runtime.threads[i];
            if (!th->running) continue;

            const SB3Block *block = th->current_block;
            if (block == NULL) continue;

            /* N'affiche que si le bloc change. */
            if (block == last_seen[i]) continue;
            last_seen[i] = block;

            printf("executed: %s | x=%.2f y=%.2f direction=%.2f\n",
                   block->opcode,
                   th->target->x,
                   th->target->y,
                   th->target->direction);
        }
    }

    if (runtime_active) {
        printf("runtime stopped after %zu steps (test limit reached)\n",
               steps);
    } else {
        printf("runtime finished after %zu steps\n", steps);
    }

    free(last_seen);
    sb3_runtime_free(&runtime);
    sb3_project_free(project);

    return 0;
}