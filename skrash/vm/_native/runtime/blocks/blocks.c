#include "blocks.h"
#include "../../project/sb3.h"
#include <stdio.h>
#include <string.h>
#include "../../runtime/runtime.h"


// evaluation d'une entrée 
int sb3_input_evaluate(SB3Thread *thread, const SB3Input *in, double *out)
{
    if (thread == NULL || in == NULL || out == NULL)
        return 0;

    switch (in->kind) {

    case SB3_INPUT_KIND_LITERAL: {
    /* Cas variable directe : [12, "nom", "id"] stockee comme LITERAL
     * avec block_id non-NULL */
    if (in->block_id != NULL && in->literal_string != NULL) {
        SB3Variable *v = sb3_project_find_variable(
            thread->runtime->project, thread->target,
            in->literal_string);
        if (v != NULL) {
            *out = v->value;
            return 1;
        }
        *out = 0.0;
        return 1;
    }
    *out = in->literal_number;
    return 1;
    }

    case SB3_INPUT_KIND_BLOCK: {
        const SB3Block *inner =
            sb3_target_find_block(thread->target, in->block_id);
        if (inner != NULL && sb3_block_evaluate(thread, inner, out))
            return 1;
        *out = in->literal_number;   /* fallback shadow */
        return 1;
    }

    case SB3_INPUT_KIND_BLOCK_ONLY: {
        const SB3Block *inner =
            sb3_target_find_block(thread->target, in->block_id);
        if (inner == NULL) return 0;
        return sb3_block_evaluate(thread, inner, out);
    }

    default:
        return 0;
    }
}

int sb3_input_evaluate_string(SB3Thread *thread, const SB3Input *in,
                              char *out, size_t out_size)
{
    if (thread == NULL || in == NULL || out == NULL || out_size == 0)
        return 0;

    out[0] = '\0';

    switch (in->kind) {

    case SB3_INPUT_KIND_LITERAL:
    if (in->block_id != NULL && in->literal_string != NULL) {
        SB3Variable *v = sb3_project_find_variable(
            thread->runtime->project, thread->target,
            in->literal_string);
        if (v != NULL) {
            snprintf(out, out_size, "%g", v->value);
            return 1;
        }
        out[0] = '\0';
        return 1;
    }
    if (in->literal_string != NULL)
        snprintf(out, out_size, "%s", in->literal_string);
    else
        snprintf(out, out_size, "%g", in->literal_number);
    return 1;

    case SB3_INPUT_KIND_BLOCK:
    case SB3_INPUT_KIND_BLOCK_ONLY: {
        const SB3Block *inner =
            sb3_target_find_block(thread->target, in->block_id);
        if (inner == NULL) {
            if (in->literal_string != NULL)
                snprintf(out, out_size, "%s", in->literal_string);
            else
                snprintf(out, out_size, "%g", in->literal_number);
            return 1;
        }
        return sb3_block_evaluate_string(thread, inner, out, out_size);
    }

    default:
        return 0;
    }
}

// dispatch d'evaluation  
int sb3_block_evaluate(SB3Thread *thread, const SB3Block *block, double *out)
{
    if (thread == NULL || block == NULL || out == NULL)
        return 0;

    switch (block->kind) {

    case SB3_BLOCK_KIND_OPERATOR_ADD:
    case SB3_BLOCK_KIND_OPERATOR_SUBTRACT:
    case SB3_BLOCK_KIND_OPERATOR_MULTIPLY:
    case SB3_BLOCK_KIND_OPERATOR_DIVIDE:
    case SB3_BLOCK_KIND_OPERATOR_MOD:
    case SB3_BLOCK_KIND_OPERATOR_ROUND:
    case SB3_BLOCK_KIND_OPERATOR_RANDOM:
    case SB3_BLOCK_KIND_OPERATOR_LT:
    case SB3_BLOCK_KIND_OPERATOR_GT:
    case SB3_BLOCK_KIND_OPERATOR_EQUALS:
    case SB3_BLOCK_KIND_OPERATOR_AND:
    case SB3_BLOCK_KIND_OPERATOR_OR:
    case SB3_BLOCK_KIND_OPERATOR_NOT:
        return sb3_operator_evaluate(thread, block, out);

    case SB3_BLOCK_KIND_MOTION_XPOSITION:
    case SB3_BLOCK_KIND_MOTION_YPOSITION:
    case SB3_BLOCK_KIND_MOTION_DIRECTION:
    return sb3_motion_evaluate(thread, block, out);
    case SB3_BLOCK_KIND_OPERATOR_JOIN:
    case SB3_BLOCK_KIND_OPERATOR_LENGTH:
    case SB3_BLOCK_KIND_OPERATOR_CONTAINS:
    case SB3_BLOCK_KIND_OPERATOR_LETTER_OF:
    // pas numerique 
    return 0;
    case SB3_BLOCK_KIND_OPERATOR_MATHOP:
    return sb3_operator_evaluate(thread, block, out);
    case SB3_BLOCK_KIND_DATA_VARIABLE:
    case SB3_BLOCK_KIND_DATA_LENGTHOFLIST:
    case SB3_BLOCK_KIND_DATA_ITEMNUMOFLIST:
    case SB3_BLOCK_KIND_DATA_LISTCONTAINSITEM:     
    return sb3_data_evaluate(thread, block, out);

    case SB3_BLOCK_KIND_SENSING_KEYPRESSED:
    case SB3_BLOCK_KIND_SENSING_MOUSEDOWN:
    case SB3_BLOCK_KIND_SENSING_MOUSEX:
    case SB3_BLOCK_KIND_SENSING_MOUSEY:
    case SB3_BLOCK_KIND_SENSING_TOUCHINGOBJECT:
    return sb3_sensing_evaluate(thread, block, out);

   
    default:
        return 0;
    }
}

int sb3_block_execute(SB3Thread *thread, const SB3Block *block)
{
    if (thread == NULL || thread->target == NULL || block == NULL)
        return 0;

    switch (block->kind) {
    case SB3_BLOCK_KIND_MOTION_MOVE_STEPS:
    case SB3_BLOCK_KIND_MOTION_TURN_RIGHT:
    case SB3_BLOCK_KIND_MOTION_TURN_LEFT:
    case SB3_BLOCK_KIND_MOTION_GOTOXY:
    case SB3_BLOCK_KIND_MOTION_SETX:
    case SB3_BLOCK_KIND_MOTION_SETY:
    case SB3_BLOCK_KIND_MOTION_POINTINDIRECTION:
    case SB3_BLOCK_KIND_MOTION_CHANGEXBY:
    case SB3_BLOCK_KIND_MOTION_CHANGEYBY:
        return sb3_motion_execute(thread, block);

    case SB3_BLOCK_KIND_LOOKS_SAY:
    case SB3_BLOCK_KIND_LOOKS_THINK:
    case SB3_BLOCK_KIND_LOOKS_SHOW:
    case SB3_BLOCK_KIND_LOOKS_HIDE:
    case SB3_BLOCK_KIND_LOOKS_SETSIZETO:
    case SB3_BLOCK_KIND_LOOKS_CHANGESIZEBY:
    case SB3_BLOCK_KIND_LOOKS_NEXTCOSTUME:
    case SB3_BLOCK_KIND_LOOKS_SWITCHCOSTUMETO:
    case SB3_BLOCK_KIND_LOOKS_COSTUME:
        return sb3_looks_execute(thread, block);

    case SB3_BLOCK_KIND_CONTROL_FOREVER:
    case SB3_BLOCK_KIND_CONTROL_REPEAT:
    case SB3_BLOCK_KIND_CONTROL_WAIT:
    case SB3_BLOCK_KIND_CONTROL_CREATECLONEOF:
    case SB3_BLOCK_KIND_CONTROL_DELETETHISCLONE:
    case SB3_BLOCK_KIND_CONTROL_STOP:
    return sb3_control_execute(thread, block);

    case SB3_BLOCK_KIND_EVENT_WHEN_FLAG_CLICKED:
    case SB3_BLOCK_KIND_EVENT_WHENKEYPRESSED:
    case SB3_BLOCK_KIND_EVENT_BROADCAST:
    case SB3_BLOCK_KIND_EVENT_BROADCASTANDWAIT:
    case SB3_BLOCK_KIND_EVENT_WHENBROADCASTRECEIVED:
    case SB3_BLOCK_KIND_EVENT_WHENTHISSPRITECLICKED:
    case SB3_BLOCK_KIND_EVENT_WHENSTAGECLICKED:
    case SB3_BLOCK_KIND_EVENT_WHENGREATERTHAN:
    return sb3_events_execute(thread, block);

    case SB3_BLOCK_KIND_DATA_SETVARIABLETO:
    case SB3_BLOCK_KIND_DATA_CHANGEVARIABLEBY:
    case SB3_BLOCK_KIND_DATA_SHOWVARIABLE:
    case SB3_BLOCK_KIND_DATA_HIDEVARIABLE:
    case SB3_BLOCK_KIND_DATA_ADDTOLIST:
    case SB3_BLOCK_KIND_DATA_DELETEOFLIST:
    case SB3_BLOCK_KIND_DATA_DELETEALLOFLIST:
    case SB3_BLOCK_KIND_DATA_INSERTATLIST:
    case SB3_BLOCK_KIND_DATA_REPLACEITEMOFLIST:
    case SB3_BLOCK_KIND_DATA_SHOWLIST:
    case SB3_BLOCK_KIND_DATA_HIDELIST:
        return sb3_data_execute(thread, block);

    default:
        return 0;
    }
}

int sb3_block_evaluate_string(SB3Thread *thread,
                              const SB3Block *block,
                              char *out, size_t out_size)
{
    if (thread == NULL || block == NULL || out == NULL || out_size == 0)
        return 0;

    out[0] = '\0';

    switch (block->kind) {

    case SB3_BLOCK_KIND_OPERATOR_JOIN:
    case SB3_BLOCK_KIND_OPERATOR_LENGTH:
    case SB3_BLOCK_KIND_OPERATOR_CONTAINS:
    case SB3_BLOCK_KIND_OPERATOR_LETTER_OF:
        return sb3_operator_evaluate_string(thread, block, out, out_size);
    case SB3_BLOCK_KIND_DATA_ITEMOFLIST:
    case SB3_BLOCK_KIND_DATA_LISTCONTENTS:
    case SB3_BLOCK_KIND_DATA_LISTCONTAINSITEM:
        return sb3_data_evaluate_string(thread, block, out, out_size);

  

    default: {
        double v = 0.0;
        if (!sb3_block_evaluate(thread, block, &v))
            return 0;
        snprintf(out, out_size, "%g", v);
        return 1;
    }
    }
}