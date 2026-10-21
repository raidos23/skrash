#include "blocks.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../../project/sb3.h"
#include "../thread.h"
#define PI 3.14159265358979323846

static double scratch_round(double x)
{
    return (x >= 0.0) ? floor(x + 0.5) : ceil(x - 0.5);
}

int sb3_operator_evaluate(SB3Thread *thread,
                          const SB3Block *block,
                          double *out)
{
    if (thread == NULL || block == NULL || out == NULL)
        return 0;

    switch (block->kind) {

    case SB3_BLOCK_KIND_OPERATOR_ADD: {
        double a = 0.0, b = 0.0;
        if (!sb3_input_evaluate(thread, &block->input_num1, &a)) return 0;
        if (!sb3_input_evaluate(thread, &block->input_num2, &b)) return 0;
        *out = a + b;
        return 1;
    }

    case SB3_BLOCK_KIND_OPERATOR_SUBTRACT: {
        double a = 0.0, b = 0.0;
        if (!sb3_input_evaluate(thread, &block->input_num1, &a)) return 0;
        if (!sb3_input_evaluate(thread, &block->input_num2, &b)) return 0;
        *out = a - b;
        return 1;
    }

    case SB3_BLOCK_KIND_OPERATOR_MULTIPLY: {
        double a = 0.0, b = 0.0;
        if (!sb3_input_evaluate(thread, &block->input_num1, &a)) return 0;
        if (!sb3_input_evaluate(thread, &block->input_num2, &b)) return 0;
        *out = a * b;
        return 1;
    }

    case SB3_BLOCK_KIND_OPERATOR_DIVIDE: {
        double a = 0.0, b = 0.0;
        if (!sb3_input_evaluate(thread, &block->input_num1, &a)) return 0;
        if (!sb3_input_evaluate(thread, &block->input_num2, &b)) return 0;
        *out = (b == 0.0) ? 0.0 : a / b;   /* Scratch renvoie 0 sur /0 */
        return 1;
    }

    case SB3_BLOCK_KIND_OPERATOR_MOD: {
        double a = 0.0, b = 0.0;
        if (!sb3_input_evaluate(thread, &block->input_num1, &a)) return 0;
        if (!sb3_input_evaluate(thread, &block->input_num2, &b)) return 0;
        if (b == 0.0) { *out = 0.0; return 1; }
        {
            double r = fmod(a, b);
            if (r != 0.0 && ((r < 0.0) != (b < 0.0))) r += b;
            *out = r;
        }
        return 1;
    }

    case SB3_BLOCK_KIND_OPERATOR_ROUND: {
        double a = 0.0;
        if (!sb3_input_evaluate(thread, &block->input_num1, &a)) return 0;
        *out = scratch_round(a);
        return 1;
    }

    case SB3_BLOCK_KIND_OPERATOR_RANDOM: {
        double a = 0.0, b = 0.0;
        if (!sb3_input_evaluate(thread, &block->input_num1, &a)) return 0;
        if (!sb3_input_evaluate(thread, &block->input_num2, &b)) return 0;

        if (floor(a) != a || floor(b) != b) {
            double lo = (a < b) ? a : b;
            double hi = (a < b) ? b : a;
            *out = lo + (hi - lo) * ((double)rand() / (double)RAND_MAX);
        } else {
            int lo = (int)((a < b) ? a : b);
            int hi = (int)((a < b) ? b : a);
            int span = hi - lo + 1;
            *out = (double)(lo + (span > 0 ? rand() % span : 0));
        }
        return 1;
    }

    case SB3_BLOCK_KIND_OPERATOR_LT: {
        double a = 0.0, b = 0.0;
        if (!sb3_input_evaluate(thread, &block->input_num1, &a)) return 0;
        if (!sb3_input_evaluate(thread, &block->input_num2, &b)) return 0;
        *out = (a < b) ? 1.0 : 0.0;
        return 1;
    }

    case SB3_BLOCK_KIND_OPERATOR_GT: {
        double a = 0.0, b = 0.0;
        if (!sb3_input_evaluate(thread, &block->input_num1, &a)) return 0;
        if (!sb3_input_evaluate(thread, &block->input_num2, &b)) return 0;
        *out = (a > b) ? 1.0 : 0.0;
        return 1;
    }

    case SB3_BLOCK_KIND_OPERATOR_EQUALS: {
        double a = 0.0, b = 0.0;
        if (!sb3_input_evaluate(thread, &block->input_num1, &a)) return 0;
        if (!sb3_input_evaluate(thread, &block->input_num2, &b)) return 0;
        /* TODO : comparaison chaine a ajouter plus tard */
        *out = (a == b) ? 1.0 : 0.0;
        return 1;
    }

    case SB3_BLOCK_KIND_OPERATOR_AND: {
        double a = 0.0, b = 0.0;
        if (!sb3_input_evaluate(thread, &block->input_num1, &a)) return 0;
        if (!sb3_input_evaluate(thread, &block->input_num2, &b)) return 0;
        *out = (a != 0.0 && b != 0.0) ? 1.0 : 0.0;
        return 1;
    }

    case SB3_BLOCK_KIND_OPERATOR_OR: {
        double a = 0.0, b = 0.0;
        if (!sb3_input_evaluate(thread, &block->input_num1, &a)) return 0;
        if (!sb3_input_evaluate(thread, &block->input_num2, &b)) return 0;
        *out = (a != 0.0 || b != 0.0) ? 1.0 : 0.0;
        return 1;
    }

    case SB3_BLOCK_KIND_OPERATOR_NOT: {
        double a = 0.0;
        if (!sb3_input_evaluate(thread, &block->input_num1, &a)) return 0;
        *out = (a == 0.0) ? 1.0 : 0.0;
        return 1;
    }

    case SB3_BLOCK_KIND_OPERATOR_MATHOP: {
    double a = 0.0;
    if (!sb3_input_evaluate(thread, &block->input_num1, &a))
        return 0;

    const char *op = block->message ? block->message : "abs";
    double r = 0.0;

    if      (strcmp(op, "abs")     == 0) r = fabs(a);
    else if (strcmp(op, "floor")   == 0) r = floor(a);
    else if (strcmp(op, "ceiling") == 0) r = ceil(a);
    else if (strcmp(op, "sqrt")    == 0) r = sqrt(a);
    else if (strcmp(op, "sin")     == 0) r = sin(a * PI / 180.0);
    else if (strcmp(op, "cos")     == 0) r = cos(a * PI / 180.0);
    else if (strcmp(op, "tan")     == 0) r = tan(a * PI / 180.0);
    else if (strcmp(op, "asin")    == 0) r = asin(a) * 180.0 / PI;
    else if (strcmp(op, "acos")    == 0) r = acos(a) * 180.0 / PI;
    else if (strcmp(op, "atan")    == 0) r = atan(a) * 180.0 / PI;
    else if (strcmp(op, "ln")      == 0) r = log(a);
    else if (strcmp(op, "log")     == 0) r = log10(a);
    else if (strcmp(op, "e ^")     == 0) r = exp(a);
    else if (strcmp(op, "10 ^")    == 0) r = pow(10.0, a);

    *out = r;
    return 1; 
    }

    default:
        return 0;
    }
}

int sb3_operator_evaluate_string(SB3Thread *thread,
                                 const SB3Block *block,
                                 char *out, size_t out_size)
{
    if (thread == NULL || block == NULL || out == NULL || out_size == 0)
        return 0;

    out[0] = '\0';

    switch (block->kind) {

    case SB3_BLOCK_KIND_OPERATOR_JOIN: {
        char a[256], b[256];
        if (!sb3_input_evaluate_string(thread, &block->input_num1, a, sizeof(a)))
            return 0;
        if (!sb3_input_evaluate_string(thread, &block->input_num2, b, sizeof(b)))
            return 0;
        snprintf(out, out_size, "%s%s", a, b);
        return 1;
    }

    case SB3_BLOCK_KIND_OPERATOR_LENGTH: {
        char s[1024];
        if (!sb3_input_evaluate_string(thread, &block->input_num1, s, sizeof(s)))
            return 0;
        snprintf(out, out_size, "%d", (int)strlen(s));
        return 1;
    }

    case SB3_BLOCK_KIND_OPERATOR_CONTAINS: {
        char a[512], b[512];
        if (!sb3_input_evaluate_string(thread, &block->input_num1, a, sizeof(a)))
            return 0;
        if (!sb3_input_evaluate_string(thread, &block->input_num2, b, sizeof(b)))
            return 0;
        snprintf(out, out_size, "%d", strstr(a, b) != NULL ? 1 : 0);
        return 1;
    }
    case SB3_BLOCK_KIND_OPERATOR_LETTER_OF: {
    double idx_d = 0.0;
    char s[256];
    if (!sb3_input_evaluate(thread, &block->input_num1, &idx_d))
        return 0;
    if (!sb3_input_evaluate_string(thread, &block->input_num2, s, sizeof(s)))
        return 0;

    int idx = (int)idx_d;
    if (idx < 1 || idx > (int)strlen(s)) {
        out[0] = '\0';
        return 1;
    }
    out[0] = s[idx - 1];
    out[1] = '\0';
    return 1;
    }


    default:
        return 0;
    }
}