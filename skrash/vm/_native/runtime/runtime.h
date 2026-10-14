#ifndef SKRASH_RUNTIME_H
#define SKRASH_RUNTIME_H

#include <stddef.h>

#include "../project/sb3.h"
#include "thread.h"

typedef struct SB3Runtime {
    SB3Project *project;
    SB3Thread *threads;
    size_t thread_count;
} SB3Runtime;

int sb3_runtime_init(SB3Runtime *runtime, SB3Project *project);
void sb3_runtime_free(SB3Runtime *runtime);

int sb3_runtime_start(SB3Runtime *runtime);
int sb3_runtime_step(SB3Runtime *runtime);

#endif /* SKRASH_RUNTIME_H */