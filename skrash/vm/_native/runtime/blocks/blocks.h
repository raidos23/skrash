#ifndef SKRASH_BLOCKS_H
#define SKRASH_BLOCKS_H
#include "../../project/sb3.h"
#include "../thread.h"
#include <stddef.h>

/* dispatch principal */
int sb3_block_execute (SB3Thread *thread, const SB3Block *block);
int sb3_block_evaluate(SB3Thread *thread, const SB3Block *block, double *out);

/* evaluation d'une entree generique (litteral ou bloc) */
int sb3_input_evaluate(SB3Thread *thread, const SB3Input *in, double *out);

/* sous-modules */
int sb3_motion_execute    (SB3Thread *thread, const SB3Block *block);
int sb3_looks_execute     (SB3Thread *thread, const SB3Block *block);
int sb3_control_execute   (SB3Thread *thread, const SB3Block *block);
int sb3_events_execute    (SB3Thread *thread, const SB3Block *block);
int sb3_operator_evaluate (SB3Thread *thread, const SB3Block *block, double *out);
int sb3_motion_evaluate(SB3Thread *thread, const SB3Block *block, double *out);
int sb3_input_evaluate_string(SB3Thread *thread, const SB3Input *in, char *out, size_t out_size);
int sb3_block_evaluate_string(SB3Thread *thread, const SB3Block *block, char *out, size_t out_size);
int sb3_operator_evaluate_string(SB3Thread *thread, const SB3Block *block, char *out, size_t out_size);
int sb3_data_execute(SB3Thread *thread, const SB3Block *block);
int sb3_data_evaluate(SB3Thread *thread, const SB3Block *block, double *out);
int sb3_data_evaluate_string(SB3Thread *thread, const SB3Block *block, char *out, size_t out_size);
int sb3_sensing_evaluate(SB3Thread *thread, const SB3Block *block, double *out);
#endif