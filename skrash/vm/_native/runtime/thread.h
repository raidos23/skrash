#ifndef SKRASH_THREAD_H
#define SKRASH_THREAD_H

#include <stddef.h>
#include "../project/sb3.h"

#define SB3_RUNTIME_MAX_REPEAT_DEPTH 64

typedef struct SB3Runtime SB3Runtime;
typedef struct SB3RepeatFrame {
    const SB3Block *repeat_block;
    const SB3Block *body;
    int remaining;
    int forever;
} SB3RepeatFrame;

typedef struct SB3Thread {
    SB3Runtime *runtime;
    SB3Target *target;
    const SB3Block *current_block;

    SB3RepeatFrame repeat_stack[SB3_RUNTIME_MAX_REPEAT_DEPTH];
    size_t repeat_depth;

    int running;
    int is_waiting;
    double wait_until;
    int waiting_for_broadcast;
    int broadcast_threads_started;
    size_t broadcast_start_count; // Pour savoir combien de threads ont ete crees
} SB3Thread;

int sb3_thread_step(SB3Thread *thread);

#endif /* SKRASH_THREAD_H */