#include <stdio.h>

#include "../skrash/vm/_native/project/sb3.h"

int main(void)
{
    SB3Project *project = sb3_load("tests/checks/non_object_blocks.sb3");

    if (project == NULL) {
        fprintf(stderr, "Erreur: projet avec une entree non-objet rejete.\n");
        return 1;
    }
    if (project->target_count != 1 || project->targets[0].block_count != 1) {
        fprintf(stderr, "Erreur: cibles ou blocs inattendus.\n");
        sb3_project_free(project);
        return 1;
    }

    printf("Entree non-objet ignoree, bloc conserve.\n");
    sb3_project_free(project);
    return 0;
}
