#include <stdio.h>

#include "../skrash/vm/_native/project/sb3.h"

int main(void)
{
    SB3Project *project;
    project = sb3_load("tests/checks/minimal.sb3");

    if (project == NULL) {
        fprintf(stderr, "Erreur: impossible de charger le projet.\n");
        return 1;
    }

    printf("SB3 charge avec succes.\n");
    printf("project.json: %zu octets\n", project->project_json_size);
    printf("assets: %zu\n", project->asset_count);

    printf("%.100s\n", project->project_json);

    sb3_project_free(project);

    return 0;
}
