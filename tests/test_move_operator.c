#include <stdio.h>

#include "project/sb3.h"
#include "runtime/runtime.h"

int main(void)
{
    SB3Project *project;
    SB3Runtime runtime;

    project = sb3_load("./tests/checks/move_operator.sb3");
    if (project == NULL) {
        fprintf(stderr, "error: failed to load move_operator.sb3\n");
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

    /* Etat initial */
    SB3Target *sprite = runtime.threads[0].target;
    printf("before: x=%.4f y=%.4f direction=%.4f\n",
           sprite->x, sprite->y, sprite->direction);

    while (sb3_runtime_step(&runtime))
        ;

    printf("after:  x=%.4f y=%.4f direction=%.4f\n",
           sprite->x, sprite->y, sprite->direction);

    int ok_x = (sprite->x > 29.999 && sprite->x < 30.001);
    int ok_y = (sprite->y > -0.001 && sprite->y < 0.001);

    printf("x == 30 ? %s\n", ok_x ? "OK" : "FAIL");
    printf("y == 0 ? %s\n", ok_y ? "OK" : "FAIL");

    sb3_runtime_free(&runtime);
    sb3_project_free(project);

    return (ok_x && ok_y) ? 0 : 1;
}