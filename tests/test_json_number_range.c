#include <stdio.h>

#include "../skrash/vm/_native/project/sb3.h"

int main(void)
{
    SB3Project *project;

    project = sb3_load("tests/checks/number_range.sb3");

    if (project == NULL) {
        fprintf(stderr, "Erreur: un nombre sous-normal (-1e-323) ou hors plage (1e400) rejette le projet.\n");
        return 1;
    }

    printf("Nombre sous-normal et nombre hors plage acceptes.\n");
    printf("project.json: %zu octets\n", project->project_json_size);
    printf("targets: %zu\n", project->target_count);

    sb3_project_free(project);

    return 0;
}
