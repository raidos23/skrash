#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../skrash/vm/_native/project/sb3.h"

typedef struct {
    const char *opcode;
    size_t count;
} OpcodeCount;
static int opcode_count_add(
    OpcodeCount **items,
    size_t *count,
    const char *opcode
)
{
    size_t i;
    OpcodeCount *new_items;
    for (i = 0; i < *count; ++i) {
        if (strcmp((*items)[i].opcode, opcode) == 0) {
            (*items)[i].count++;
            return 1;
        }
    }

    new_items = realloc(*items, (*count + 1) * sizeof(**items));

    if (new_items == NULL)
        return 0;

    *items = new_items;

    (*items)[*count].opcode = opcode;
    (*items)[*count].count = 1;
    (*count)++;

    return 1;
}

int main(void)
{
    SB3Project *project;
    OpcodeCount *known = NULL;
    OpcodeCount *unknown = NULL;
    size_t known_count = 0;
    size_t unknown_count = 0;
    size_t target_index;
    size_t block_index;
    size_t total = 0;

    project = sb3_load("tests/checks/blocks.sb3");

    if (project == NULL) {
        fprintf(stderr, "Erreur: impossible de charger blocks.sb3\n");
        return 1;
    }
    for (target_index = 0;
         target_index < project->target_count;
         ++target_index) {
        const SB3Target *target = &project->targets[target_index];

        for (block_index = 0;
             block_index < target->block_count;
             ++block_index) {

            const SB3Block *block = &target->blocks[block_index];

            if (block->opcode == NULL)
                continue;
            total++;

            if (block->kind == SB3_BLOCK_KIND_UNKNOWN) {
                if (!opcode_count_add(
                        &unknown,
                        &unknown_count,
                        block->opcode)) {
                    fprintf(stderr, "Erreur mémoire.\n");
                    sb3_project_free(project);
                    free(known);
                    free(unknown);
                    return 1;
                }
            } else {
                if (!opcode_count_add(
                        &known,
                        &known_count,
                        block->opcode)) {
                    fprintf(stderr, "Erreur mémoire.\n");
                    sb3_project_free(project);
                    free(known);
                    free(unknown);
                    return 1;
                }
            }
        }
    }

    printf("=== Opcode detection ===\n\n");

    for (size_t i = 0; i < known_count; ++i)
        printf("%-30s : %zu\n", known[i].opcode, known[i].count);

    if (unknown_count > 0) {
        printf("\n=== Unknown opcodes ===\n\n");

        for (size_t i = 0; i < unknown_count; ++i)
            printf("%-30s : %zu\n",
                   unknown[i].opcode,
                   unknown[i].count);
    }
    printf("\n=== Summary ===\n");
    printf("Total   : %zu\n", total);
    {
        size_t known_blocks = 0;
        size_t unknown_blocks = 0;

        for (size_t i = 0; i < known_count; ++i)
            known_blocks += known[i].count;
        for (size_t i = 0; i < unknown_count; ++i)
            unknown_blocks += unknown[i].count;
        printf("Known   : %zu\n", known_blocks);
        printf("Unknown : %zu\n", unknown_blocks);
    }
    sb3_project_free(project);
    free (known) ; 
    free(unknown);
    return 0;
}