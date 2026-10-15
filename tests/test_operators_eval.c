#include <stdio.h>
#include <string.h>

#include "../skrash/vm/_native/project/sb3.h"
#include "../skrash/vm/_native/runtime/thread.h"
#include "../skrash/vm/_native/runtime/blocks/blocks.h"

static int check(const char *label, double got, double want)
{
    int ok = (got == want);
    printf("%-30s got=%g want=%g  %s\n",
           label, got, want, ok ? "OK" : "FAIL");
    return ok;
}

int main(void)
{
    SB3Target target;
    memset(&target, 0, sizeof(target));
    target.direction = 90.0;

    SB3Thread th;
    memset(&th, 0, sizeof(th));
    th.target = &target;

    SB3Block b;
    memset(&b, 0, sizeof(b));

    double v;
    int pass = 0, total = 0;

    /* add 2 + 3 */
    b.kind = SB3_BLOCK_KIND_OPERATOR_ADD;
    b.input_num1.kind = SB3_INPUT_KIND_LITERAL;
    b.input_num1.literal_number = 2.0;
    b.input_num2.kind = SB3_INPUT_KIND_LITERAL;
    b.input_num2.literal_number = 3.0;
    sb3_block_evaluate(&th, &b, &v);
    total++; pass += check("2 + 3", v, 5.0);

    /* subtract 10 - 4 */
    b.kind = SB3_BLOCK_KIND_OPERATOR_SUBTRACT;
    b.input_num1.literal_number = 10.0;
    b.input_num2.literal_number = 4.0;
    sb3_block_evaluate(&th, &b, &v);
    total++; pass += check("10 - 4", v, 6.0);

    /* multiply 6 * 7 */
    b.kind = SB3_BLOCK_KIND_OPERATOR_MULTIPLY;
    b.input_num1.literal_number = 6.0;
    b.input_num2.literal_number = 7.0;
    sb3_block_evaluate(&th, &b, &v);
    total++; pass += check("6 * 7", v, 42.0);

    /* divide 2 / 4 */
    b.kind = SB3_BLOCK_KIND_OPERATOR_DIVIDE;
    b.input_num1.literal_number = 2.0;
    b.input_num2.literal_number = 4.0;
    sb3_block_evaluate(&th, &b, &v);
    total++; pass += check("2 / 4", v, 0.5);

    /* divide par zero */
    b.input_num2.literal_number = 0.0;
    sb3_block_evaluate(&th, &b, &v);
    total++; pass += check("2 / 0", v, 0.0);

    /* mod 17 mod 5 */
    b.kind = SB3_BLOCK_KIND_OPERATOR_MOD;
    b.input_num1.literal_number = 17.0;
    b.input_num2.literal_number = 5.0;
    sb3_block_evaluate(&th, &b, &v);
    total++; pass += check("17 mod 5", v, 2.0);

    /* round 3.7 */
    b.kind = SB3_BLOCK_KIND_OPERATOR_ROUND;
    b.input_num1.literal_number = 3.7;
    sb3_block_evaluate(&th, &b, &v);
    total++; pass += check("round 3.7", v, 4.0);

    /* round -3.7 */
    b.input_num1.literal_number = -3.7;
    sb3_block_evaluate(&th, &b, &v);
    total++; pass += check("round -3.7", v, -4.0);

    /* lt 3 < 5 */
    b.kind = SB3_BLOCK_KIND_OPERATOR_LT;
    b.input_num1.literal_number = 3.0;
    b.input_num2.literal_number = 5.0;
    sb3_block_evaluate(&th, &b, &v);
    total++; pass += check("3 < 5", v, 1.0);

    /* lt 5 < 3 */
    b.input_num1.literal_number = 5.0;
    b.input_num2.literal_number = 3.0;
    sb3_block_evaluate(&th, &b, &v);
    total++; pass += check("5 < 3", v, 0.0);

    /* gt 5 > 3 */
    b.kind = SB3_BLOCK_KIND_OPERATOR_GT;
    b.input_num1.literal_number = 5.0;
    b.input_num2.literal_number = 3.0;
    sb3_block_evaluate(&th, &b, &v);
    total++; pass += check("5 > 3", v, 1.0);

    /* equals 5 = 5 */
    b.kind = SB3_BLOCK_KIND_OPERATOR_EQUALS;
    b.input_num1.literal_number = 5.0;
    b.input_num2.literal_number = 5.0;
    sb3_block_evaluate(&th, &b, &v);
    total++; pass += check("5 = 5", v, 1.0);

    /* equals 5 = 4 */
    b.input_num2.literal_number = 4.0;
    sb3_block_evaluate(&th, &b, &v);
    total++; pass += check("5 = 4", v, 0.0);

    /* and 1 and 1 */
    b.kind = SB3_BLOCK_KIND_OPERATOR_AND;
    b.input_num1.literal_number = 1.0;
    b.input_num2.literal_number = 1.0;
    sb3_block_evaluate(&th, &b, &v);
    total++; pass += check("1 and 1", v, 1.0);

    /* and 1 and 0 */
    b.input_num2.literal_number = 0.0;
    sb3_block_evaluate(&th, &b, &v);
    total++; pass += check("1 and 0", v, 0.0);

    /* or 0 or 1 */
    b.kind = SB3_BLOCK_KIND_OPERATOR_OR;
    b.input_num1.literal_number = 0.0;
    b.input_num2.literal_number = 1.0;
    sb3_block_evaluate(&th, &b, &v);
    total++; pass += check("0 or 1", v, 1.0);

    /* not 0 */
    b.kind = SB3_BLOCK_KIND_OPERATOR_NOT;
    b.input_num1.literal_number = 0.0;
    sb3_block_evaluate(&th, &b, &v);
    total++; pass += check("not 0", v, 1.0);

    /* not 1 */
    b.input_num1.literal_number = 1.0;
    sb3_block_evaluate(&th, &b, &v);
    total++; pass += check("not 1", v, 0.0);

    /* random 1 to 1 */
    b.kind = SB3_BLOCK_KIND_OPERATOR_RANDOM;
    b.input_num1.literal_number = 1.0;
    b.input_num2.literal_number = 1.0;
    sb3_block_evaluate(&th, &b, &v);
    total++; pass += check("random 1 to 1", v, 1.0);

    printf("\n%d/%d tests OK\n", pass, total);
    return (pass == total) ? 0 : 1;
}