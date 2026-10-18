#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "project/sb3.h"
#include "runtime/runtime.h"

int main(void)
{
    SB3Project *project;
    SB3Runtime runtime;

    project = sb3_load("./tests/checks/motion_reporters.sb3");
    if (project == NULL) {
        fprintf(stderr, "error: failed to load motion_reporters.sb3\n");
        return 1;
    }

    if (!sb3_runtime_init(&runtime, project)) {
        sb3_project_free(project);
        return 1;
    }

    if (!sb3_runtime_start(&runtime)) {
        sb3_runtime_free(&runtime);
        sb3_project_free(project);
        return 1;
    }

    while (sb3_runtime_step(&runtime))
        ;

    printf("runtime finished\n");

    sb3_runtime_free(&runtime);
    sb3_project_free(project);

    return 0;
}