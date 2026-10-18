#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../skrash/vm/_native/project/sb3.h"
#include "../skrash/vm/_native/runtime/runtime.h"

int main(void)
{
    SB3Project *project;
    SB3Runtime runtime;

    project = sb3_load("./tests/checks/if.sb3");
    if (project == NULL) {
        fprintf(stderr, "error: failed to load if.sb3\n");
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

    int steps = 0;
    while (sb3_runtime_step(&runtime))
        ++steps;

    printf("runtime finished in %d steps\n", steps);

    sb3_runtime_free(&runtime);
    sb3_project_free(project);

    return 0;
}