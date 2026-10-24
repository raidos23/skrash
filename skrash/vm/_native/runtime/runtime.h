#ifndef SKRASH_RUNTIME_H
#define SKRASH_RUNTIME_H

#include <stddef.h>

#include "../project/sb3.h"
#include "thread.h"

typedef struct SB3Runtime {
    SB3Project *project;
    SB3Thread *threads;
    size_t thread_count;

    int keys[512];
    int mouse_down;
    int mouse_x;
    int mouse_y;
} SB3Runtime;

int sb3_runtime_init(SB3Runtime *runtime, SB3Project *project);
void sb3_runtime_free(SB3Runtime *runtime);

int sb3_runtime_start(SB3Runtime *runtime);
int sb3_runtime_step(SB3Runtime *runtime);

void sb3_runtime_set_key(SB3Runtime *runtime, int scancode, int is_down);
void sb3_runtime_set_mouse(SB3Runtime *runtime, int x, int y, int down);
int sb3_runtime_is_key_pressed(SB3Runtime *runtime, const char *key_name);

#endif /* SKRASH_RUNTIME_H */