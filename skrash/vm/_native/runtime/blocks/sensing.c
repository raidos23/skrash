#include "blocks.h"
#include "../../project/sb3.h"
#include "../runtime.h"

int sb3_sensing_evaluate(SB3Thread *thread, const SB3Block *block, double *out)
{
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
            /* Scratch utilise la position de la souris par rapport au centre (0,0) */
            *out = (double)(thread->runtime->mouse_x - 240); 
            return 1;
            
        case SB3_BLOCK_KIND_SENSING_MOUSEY:
            *out = (double)(180 - thread->runtime->mouse_y);
            return 1;

        default:
            return 0;
    }
}