#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "project/sb3.h"

static char *dup_string(const char *s)
{
    if (s == NULL) return NULL;
    size_t n = strlen(s) + 1;
    char *c = malloc(n);
    if (c) memcpy(c, s, n);
    return c;

}

int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "usage: %s <project.sb3>\n", argv[0]);
        return 1;
    }

    SB3Project *project = sb3_load(argv[1]);
    if (project == NULL) {
        fprintf(stderr, "error: failed to load %s\n", argv[1]);
        return 1;
    }

    printf("=== Analyse du projet : %s ===\n\n", argv[1]);

    /* On garde une trace des opcodes inconnus pour ne pas les repeter */
    char *unknown_opcodes[256];
    int unknown_counts[256];
    int unknown_total = 0;

    int known_total = 0;

    for (size_t i = 0; i < project->target_count; ++i) {
        SB3Target *t = &project->targets[i];
        for (size_t j = 0; j < t->block_count; ++j) {
            SB3Block *b = &t->blocks[j];
            
            /* On ignore les blocs qui n'ont pas d'opcode (variables, listes, etc) */
            if (b->opcode == NULL) continue;

            if (b->kind == SB3_BLOCK_KIND_UNKNOWN) {
                /* Cherche si on a deja vu cet opcode */
                int found = -1;
                for (int k = 0; k < unknown_total; ++k) {
                    if (strcmp(unknown_opcodes[k], b->opcode) == 0) {
                        found = k;
                        break;
                    }
                }
                
                if (found >= 0) {
                    unknown_counts[found]++;
                } else if (unknown_total < 256) {
                    unknown_opcodes[unknown_total] = dup_string(b->opcode);
                    unknown_counts[unknown_total] = 1;
                    unknown_total++;
                }
            } else {
                known_total++;
            }
        }
    }

    printf("Blocs reconnus : %d\n", known_total);
    printf("Blocs inconnus : %d\n\n", unknown_total);

    if (unknown_total == 0) {
        printf("✓ Tous les blocs de ce projet sont supportes !\n");
    } else {
        printf("✗ Blocs manquants :\n");
        for (int k = 0; k < unknown_total; ++k) {
            printf("  - %-40s (x%d)\n", unknown_opcodes[k], unknown_counts[k]);
            free(unknown_opcodes[k]);
        }
    }

    sb3_project_free(project);
    return 0;
}