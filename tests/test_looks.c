#include <stdio.h>

#include "../skrash/vm/_native/project/sb3.h"
#include "../skrash/vm/_native/runtime/runtime.h"

int main(void)
{
    SB3Project *project;
    SB3Runtime runtime;

    project = sb3_load("./tests/checks/looks.sb3");
    if (project == NULL) {
        fprintf(stderr, "error: failed to load one.sb3\n");
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

    while (runtime.threads[0].running) {
        const SB3Block *block = runtime.threads[0].current_block;

        if (!sb3_runtime_step(&runtime)) {
            fprintf(stderr, "Error during execution.\n");
            break;
        }

        if (block != NULL) {
            printf(
                "executed: %s | x=%.2f y=%.2f direction=%.2f\n size=%.2f",
                block->opcode,
                runtime.threads[0].target->x,
                runtime.threads[0].target->y,
                runtime.threads[0].target->direction,
                runtime.threads[0].target->size
            );
        }
    }

    printf("runtime finished\n");

    sb3_runtime_free(&runtime);
    sb3_project_free(project);

    return 0;
}