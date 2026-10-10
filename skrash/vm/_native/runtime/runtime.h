#ifndef SKRASH_RUNTIME_H
#define SKRASH_RUNTIME_H
#define SB3_RUNTIME_MAX_REPEAT_DEPTH 64

#include "../project/sb3.h"

typedef struct SB3RepeatFrame {
    const SB3Block *repeat_block;
    const SB3Block *body;
    int remaining;
} SB3RepeatFrame;

typedef struct SB3Runtime {
    SB3Project *project;
    SB3Target *target;
    const SB3Block *current_block;

    SB3RepeatFrame repeat_stack[SB3_RUNTIME_MAX_REPEAT_DEPTH];
    size_t repeat_depth;

    int running;
} SB3Runtime;

int sb3_runtime_init(
    SB3Runtime *runtime,
    SB3Project *project
);

void sb3_runtime_free(
    SB3Runtime *runtime
);

int sb3_runtime_start(
    SB3Runtime *runtime
);

int sb3_runtime_step(
    SB3Runtime *runtime
);

#endif