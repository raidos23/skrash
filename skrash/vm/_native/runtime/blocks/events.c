#include "blocks.h"
#include "../../project/sb3.h"
#include "../runtime.h"

int sb3_events_execute(SB3Thread *thread, const SB3Block *block)
{
    if (!thread || !block) return 0;

    switch (block->kind) {
        case SB3_BLOCK_KIND_EVENT_BROADCAST:
        case SB3_BLOCK_KIND_EVENT_BROADCASTANDWAIT:
            if (block->broadcast_name && thread->runtime) {
                sb3_runtime_broadcast(thread->runtime, block->broadcast_name);
            }
            return 1;

        case SB3_BLOCK_KIND_EVENT_WHEN_FLAG_CLICKED:
        case SB3_BLOCK_KIND_EVENT_WHENBROADCASTRECEIVED:
        case SB3_BLOCK_KIND_EVENT_WHENKEYPRESSED:
        case SB3_BLOCK_KIND_EVENT_WHENTHISSPRITECLICKED:
        case SB3_BLOCK_KIND_EVENT_WHENSTAGECLICKED:
        case SB3_BLOCK_KIND_EVENT_WHENGREATERTHAN:
            return 1; /* Les chapeaux ne s'executent pas */

        default:
            return 0;
    }
}