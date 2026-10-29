#include "blocks.h"
#include "../../project/sb3.h"
#include "../runtime.h"
#include <math.h>
#include <string.h>
#include <stdio.h>
int sb3_sensing_evaluate(SB3Thread *thread, const SB3Block *block, double *out) {
    if (!thread || !block || !out) return 0;

    switch (block->kind) {
    case SB3_BLOCK_KIND_SENSING_KEYPRESSED: {
        const char *key_name = block->message;

        /* Résolution du shadow si nécessaire */
        if (key_name == NULL && block->input_num1.kind == SB3_INPUT_KIND_BLOCK) {
            const SB3Block *inner = sb3_target_find_block(thread->target, block->input_num1.block_id);
            if (inner) key_name = inner->message;
        }

        *out = (key_name && sb3_runtime_is_key_pressed(thread->runtime, key_name)) ? 1.0 : 0.0;
        return 1;
    }

    case SB3_BLOCK_KIND_SENSING_MOUSEDOWN:
        *out = thread->runtime->mouse_down ? 1.0 : 0.0;
        return 1;

    case SB3_BLOCK_KIND_SENSING_MOUSEX:
        *out = (double)(thread->runtime->mouse_x);
        return 1;

    case SB3_BLOCK_KIND_SENSING_MOUSEY:
        *out = (double)thread->runtime->mouse_y;
        return 1;

    case SB3_BLOCK_KIND_SENSING_TOUCHINGOBJECT: {

        if (!block->message || !thread->target) {
            *out = 0.0;
            return 1;
        }

        /* 1. Dimensions du sprite appelant */
        double w1 = 50.0, h1 = 50.0;
        if (thread->target->costume_count > 0) {
            SB3Costume *c = &thread->target->costumes[thread->target->current_costume];
            if (c->width > 0) w1 = c->width;
            if (c->height > 0) h1 = c->height;
        }
        w1 *= (thread->target->size / 100.0);
        h1 *= (thread->target->size / 100.0);
        /* Cas 1 : souris */
        /* Cas 1 : souris */
        if (strcmp(block->message, "_mouse_") == 0) {
            double mx = (double)thread->runtime->mouse_x;
            double my = (double)thread->runtime->mouse_y;

            double dx = mx - thread->target->x;
            double dy = my - thread->target->y;
            *out = (fabs(dx) <= w1/2.0 && fabs(dy) <= h1/2.0) ? 1.0 : 0.0;
            return 1;
        }

        /* Cas 2 : bord de l'ecran */
        if (strcmp(block->message, "_edge_") == 0) {
            int hit = (thread->target->x - w1/2.0 <= -240.0 ||
                       thread->target->x + w1/2.0 >=  240.0 ||
                       thread->target->y - h1/2.0 <= -180.0 ||
                       thread->target->y + h1/2.0 >=  180.0);
            *out = hit ? 1.0 : 0.0;
            return 1;
        }
        /* Cas 3 : autre sprite */
        for (size_t i = 0; i < thread->runtime->project->target_count; ++i) {
            SB3Target *other = &thread->runtime->project->targets[i];
            if (other == thread->target || other->is_stage) continue;

            if (other->name && strcmp(other->name, block->message) == 0) {
                /* Dimensions de l'autre sprite */
                double w2 = 50.0, h2 = 50.0;
                if (other->costume_count > 0) {
                    SB3Costume *c2 = &other->costumes[other->current_costume];
                    if (c2->width > 0) w2 = c2->width;
                    if (c2->height > 0) h2 = c2->height;
                }
                w2 *= (other->size / 100.0);
                h2 *= (other->size / 100.0);

                double dx = thread->target->x - other->x;
                double dy = thread->target->y - other->y;

                int touching = (fabs(dx) <= (w1 + w2)/2.0 && fabs(dy) <= (h1 + h2)/2.0);
                *out = touching ? 1.0 : 0.0;
                return 1;
            }
        }

        *out = 0.0;
        return 1;
    }

    default:
        return 0;
    }
}