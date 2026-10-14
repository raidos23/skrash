#include <stdio.h>
#include <stdlib.h>

#include "../skrash/vm/_native/project/sb3.h"
#include "../skrash/vm/_native/runtime/runtime.h"

#define MAX_RUNTIME_STEPS 100000

int main(void)
{
    SB3Project *project = sb3_load("./tests/checks/multithreads.sb3");

    if (project == NULL) {
        fprintf(stderr, "Failed to load project: tests/checks/multithreads.sb3\n");
        return EXIT_FAILURE;
    }

    SB3Runtime runtime = {0};

    if (!sb3_runtime_init(&runtime, project)) {
        fprintf(stderr, "Failed to initialize runtime\n");
        sb3_destroy(project);
        return EXIT_FAILURE;
    }

    if (!sb3_runtime_start(&runtime)) {
        fprintf(stderr, "No executable green-flag scripts found\n");
        sb3_runtime_free(&runtime);
        sb3_destroy(project);
        return EXIT_FAILURE;
    }

    printf("Executing %zu threads...\n", runtime.thread_count);

    size_t steps = 0;

    while (steps < MAX_RUNTIME_STEPS) {
        int any_running = sb3_runtime_step(&runtime);
        ++steps;

        if (!any_running)
            break;
    }

    if (steps == MAX_RUNTIME_STEPS)
        fprintf(stderr, "Execution limit reached\n");

    for (size_t i = 0; i < runtime.thread_count; ++i) {
        const SB3Thread *thread = &runtime.threads[i];

        printf(
            "Thread %zu final: x=%.2f, y=%.2f, direction=%.2f, running=%d\n",
            i + 1,
            thread->target->x,
            thread->target->y,
            thread->target->direction,
            thread->running
        );
    }

    printf("Execution finished after %zu scheduler steps.\n", steps);

    sb3_runtime_free(&runtime);
    sb3_destroy(project);

    return EXIT_SUCCESS;
}