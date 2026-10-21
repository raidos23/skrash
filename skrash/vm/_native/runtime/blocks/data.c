#include "blocks.h"
#include "../../project/sb3.h"
#include "../../runtime/runtime.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* -------------------------------------
 Variables         
 -------------------------------------*/

int sb3_data_execute(SB3Thread *thread, const SB3Block *block)
{
    if (thread == NULL || block == NULL || thread->runtime == NULL)
        return 0;

    switch (block->kind) {

    case SB3_BLOCK_KIND_DATA_SETVARIABLETO: {
        double v = 0.0;
        if (!sb3_input_evaluate(thread, &block->input_num1, &v))
            return 0;
        SB3Variable *var = sb3_project_find_variable(
            thread->runtime->project, thread->target, block->variable_name);
        if (var != NULL) var->value = v;
        return 1;
    }

    case SB3_BLOCK_KIND_DATA_CHANGEVARIABLEBY: {
        double v = 0.0;
        if (!sb3_input_evaluate(thread, &block->input_num1, &v))
            return 0;
        SB3Variable *var = sb3_project_find_variable(
            thread->runtime->project, thread->target, block->variable_name);
        if (var != NULL) var->value += v;
        return 1;
    }

    case SB3_BLOCK_KIND_DATA_SHOWVARIABLE:
    case SB3_BLOCK_KIND_DATA_HIDEVARIABLE:
        /* no-op sans renderer */
        return 1;

    /* -------------------------------------------------------------- */
    /* Listes                                                          */

    case SB3_BLOCK_KIND_DATA_ADDTOLIST: {
        SB3List *l = sb3_project_find_list(
            thread->runtime->project, thread->target, block->list_name);
        if (l == NULL) return 1;

        char buf[512] = {0};
        if (block->input_num1.kind == SB3_INPUT_KIND_LITERAL &&
            block->input_num1.literal_string != NULL)
            snprintf(buf, sizeof(buf), "%s", block->input_num1.literal_string);
        else {
            double v = 0.0;
            if (!sb3_input_evaluate(thread, &block->input_num1, &v)) return 0;
            snprintf(buf, sizeof(buf), "%g", v);
        }

        char **new_items = realloc(l->items,
                                    (l->item_count + 1) * sizeof(char *));
        if (new_items == NULL) return 1;
        l->items = new_items;
        l->items[l->item_count++] = strdup(buf);
        return 1;
    }

    case SB3_BLOCK_KIND_DATA_DELETEOFLIST: {
        SB3List *l = sb3_project_find_list(
            thread->runtime->project, thread->target, block->list_name);
        if (l == NULL) return 1;

        double idx_d = 0.0;
        if (!sb3_input_evaluate(thread, &block->input_num1, &idx_d)) return 0;
        int idx = (int)idx_d;
        if (idx < 1 || idx > (int)l->item_count) return 1;

        free(l->items[idx - 1]);
        for (size_t i = (size_t)idx; i < l->item_count; ++i)
            l->items[i - 1] = l->items[i];
        l->item_count--;
        return 1;
    }

    case SB3_BLOCK_KIND_DATA_DELETEALLOFLIST: {
        SB3List *l = sb3_project_find_list(
            thread->runtime->project, thread->target, block->list_name);
        if (l == NULL) return 1;
        for (size_t i = 0; i < l->item_count; ++i)
            free(l->items[i]);
        free(l->items);
        l->items = NULL;
        l->item_count = 0;
        return 1;
    }

    case SB3_BLOCK_KIND_DATA_INSERTATLIST: {
        SB3List *l = sb3_project_find_list(
            thread->runtime->project, thread->target, block->list_name);
        if (l == NULL) return 1;

        char buf[512] = {0};
        if (block->input_num1.kind == SB3_INPUT_KIND_LITERAL &&
            block->input_num1.literal_string != NULL)
            snprintf(buf, sizeof(buf), "%s", block->input_num1.literal_string);
        else {
            double v = 0.0;
            if (!sb3_input_evaluate(thread, &block->input_num1, &v)) return 0;
            snprintf(buf, sizeof(buf), "%g", v);
        }

        double idx_d = 0.0;
        if (!sb3_input_evaluate(thread, &block->input_num2, &idx_d)) return 0;
        int idx = (int)idx_d;
        if (idx < 1 || idx > (int)l->item_count + 1) return 1;

        char **new_items = realloc(l->items,
                                    (l->item_count + 1) * sizeof(char *));
        if (new_items == NULL) return 1;
        l->items = new_items;

        for (size_t i = l->item_count; i >= (size_t)idx; --i)
            l->items[i] = l->items[i - 1];
        l->items[idx - 1] = strdup(buf);
        l->item_count++;
        return 1;
    }

    case SB3_BLOCK_KIND_DATA_REPLACEITEMOFLIST: {
        SB3List *l = sb3_project_find_list(
            thread->runtime->project, thread->target, block->list_name);
        if (l == NULL) return 1;

        double idx_d = 0.0;
        if (!sb3_input_evaluate(thread, &block->input_num1, &idx_d)) return 0;
        int idx = (int)idx_d;
        if (idx < 1 || idx > (int)l->item_count) return 1;

        char buf[512] = {0};
        if (block->input_num2.kind == SB3_INPUT_KIND_LITERAL &&
            block->input_num2.literal_string != NULL)
            snprintf(buf, sizeof(buf), "%s", block->input_num2.literal_string);
        else {
            double v = 0.0;
            if (!sb3_input_evaluate(thread, &block->input_num2, &v)) return 0;
            snprintf(buf, sizeof(buf), "%g", v);
        }

        free(l->items[idx - 1]);
        l->items[idx - 1] = strdup(buf);
        return 1;
    }

    case SB3_BLOCK_KIND_DATA_SHOWLIST:
    case SB3_BLOCK_KIND_DATA_HIDELIST:
        return 1;

    default:
        return 0;
    }
}

/* ------------------------------------------------------------------ */
/* Reporters numeriques                                               */

int sb3_data_evaluate(SB3Thread *thread, const SB3Block *block, double *out)
{
    fprintf(stderr, "[eval] kind=%d list='%s' num1.kind=%d num1.str='%s'\n",
        block->kind,
        block->list_name ? block->list_name : "(null)",
        block->input_num1.kind,
        block->input_num1.literal_string ? block->input_num1.literal_string : "(null)");
    // dans data.c, avant le switch
    fprintf(stderr, "[eval] kind=%d LISTCONTAINS=%d\n",
        block->kind,
        SB3_BLOCK_KIND_DATA_LISTCONTAINSITEM);

    if (thread == NULL || block == NULL || out == NULL ||
        thread->runtime == NULL)
        return 0;

    switch (block->kind) {

    case SB3_BLOCK_KIND_DATA_VARIABLE: {
        SB3Variable *v = sb3_project_find_variable(
            thread->runtime->project, thread->target, block->variable_name);
        if (v == NULL) return 0;
        *out = v->value;
        return 1;
    }

    case SB3_BLOCK_KIND_DATA_LENGTHOFLIST: {
        SB3List *l = sb3_project_find_list(
            thread->runtime->project, thread->target, block->list_name);
        if (l == NULL) return 0;
        *out = (double)l->item_count;
        return 1;
    }

    case SB3_BLOCK_KIND_DATA_ITEMNUMOFLIST: {
        SB3List *l = sb3_project_find_list(
            thread->runtime->project, thread->target, block->list_name);
        if (l == NULL) return 0;

        char needle[512] = {0};
        if (block->input_num1.kind == SB3_INPUT_KIND_LITERAL &&
            block->input_num1.literal_string != NULL)
            snprintf(needle, sizeof(needle), "%s",
                     block->input_num1.literal_string);
        else {
            double v = 0.0;
            if (!sb3_input_evaluate(thread, &block->input_num1, &v)) return 0;
            snprintf(needle, sizeof(needle), "%g", v);
        }

        for (size_t i = 0; i < l->item_count; ++i) {
            if (strcmp(l->items[i], needle) == 0) {
                *out = (double)(i + 1);
                return 1;
            }
        }
        *out = 0.0;
        return 1;
    }

    case SB3_BLOCK_KIND_DATA_LISTCONTAINSITEM: {
        SB3List *l = sb3_project_find_list(
            thread->runtime->project, thread->target, block->list_name);
        if (l == NULL) return 0;

        char needle[512] = {0};
        if (block->input_num1.kind == SB3_INPUT_KIND_LITERAL &&
            block->input_num1.literal_string != NULL)
            snprintf(needle, sizeof(needle), "%s",
                     block->input_num1.literal_string);
        else {
            double v = 0.0;
            if (!sb3_input_evaluate(thread, &block->input_num1, &v)) return 0;
            snprintf(needle, sizeof(needle), "%g", v);
        }

        for (size_t i = 0; i < l->item_count; ++i) {
            if (strcmp(l->items[i], needle) == 0) {
                *out = 1.0;
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

/* ------------------------------------------------------------------ */
/* Reporters string                                                   */

int sb3_data_evaluate_string(SB3Thread *thread,
                              const SB3Block *block,
                              char *out, size_t out_size)
{
    if (thread == NULL || block == NULL || out == NULL || out_size == 0 ||
        thread->runtime == NULL)
        return 0;

    out[0] = '\0';

    switch (block->kind) {

    case SB3_BLOCK_KIND_DATA_ITEMOFLIST: {
        SB3List *l = sb3_project_find_list(
            thread->runtime->project, thread->target, block->list_name);
        if (l == NULL) return 1;

        double idx_d = 0.0;
        if (!sb3_input_evaluate(thread, &block->input_num1, &idx_d)) return 0;
        int idx = (int)idx_d;
        if (idx < 1 || idx > (int)l->item_count) return 1;

        snprintf(out, out_size, "%s", l->items[idx - 1]);
        return 1;
    }

    case SB3_BLOCK_KIND_DATA_LISTCONTENTS: {
        SB3List *l = sb3_project_find_list(
            thread->runtime->project, thread->target, block->list_name);
        if (l == NULL) return 1;

        size_t used = 0;
        for (size_t i = 0; i < l->item_count && used < out_size - 1; ++i) {
            int n = snprintf(out + used, out_size - used,
                             "%s%s", i > 0 ? " " : "", l->items[i]);
            if (n < 0) break;
            used += (size_t)n;
        }
        return 1;
    }

    case SB3_BLOCK_KIND_DATA_LISTCONTAINSITEM: {
    double v = 0.0;
    if (!sb3_data_evaluate(thread, block, &v))
        return 0;
    snprintf(out, out_size, "%g", v);
    return 1;
}

    default:
        return 0;
    }
}