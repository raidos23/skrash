#include <stdio.h>
#include <stdlib.h>

#include "renderer.h"
#include "../vm/_native/project/sb3.h"
#include "../vm/_native/runtime/runtime.h"

int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "usage: %s <project.sb3>\n", argv[0]);
        return 1;
    }

    SB3Project *project = sb3_load(argv[1]);
    if (project == NULL) {
        fprintf(stderr, "error: failed to load %s\n", argv[1]);
        return 1;
    }

    SB3Runtime runtime;
    if (!sb3_runtime_init(&runtime, project) ||
        !sb3_runtime_start(&runtime)) {
        fprintf(stderr, "error: failed to start runtime\n");
        sb3_project_free(project);
        return 1;
    }

    printf("project loaded: %zu targets, %zu threads\n",
           project->target_count, runtime.thread_count);

    /* Taille native du stage Scratch */
    SB3Renderer renderer;
    if (!sb3_renderer_init(&renderer,
                           SB3_STAGE_WIDTH, SB3_STAGE_HEIGHT, "skrash")) {
        sb3_runtime_free(&runtime);
        sb3_project_free(project);
        return 1;
    }

    int steps = 0;
    renderer.runtime_ptr = &runtime;
    while (sb3_renderer_poll(&renderer)) {
        int active = sb3_runtime_step(&runtime);
        if (active) ++steps;

        sb3_renderer_clear(&renderer, 255, 255, 255);
        sb3_renderer_draw_stage(&renderer, project);
        sb3_renderer_draw_targets(&renderer, project);
        sb3_renderer_present(&renderer);

        SDL_Delay(16);
    }

    printf("runtime finished in %d steps\n", steps);

    sb3_renderer_free(&renderer);
    sb3_runtime_free(&runtime);
    sb3_project_free(project);
    return 0;
}