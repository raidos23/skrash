#include <stdio.h>
#include "../skrash/vm/_native/project/sb3.h"
#include "../skrash/vm/_native/runtime/runtime.h"

int main(void)
{
    SB3Project *project = sb3_load("./tests/checks/test_repeat.sb3");

    if (project == NULL) {
        fprintf(stderr, "Erreur : chargement du projet impossible.\n");
        return 1;
    }

    SB3Runtime runtime;

    if (!sb3_runtime_init(&runtime, project)) {
        fprintf(stderr, "Erreur : initialisation du runtime impossible.\n");
        sb3_project_free(project);
        return 1;
    }

    if (!sb3_runtime_start(&runtime)) {
        fprintf(stderr, "Erreur : démarrage du runtime impossible.\n");
        sb3_runtime_free(&runtime);
        sb3_project_free(project);
        return 1;
    }
    printf(
    "Initial : x=%.2f y=%.2f direction=%.2f\n",
    runtime.target->x,
    runtime.target->y,
    runtime.target->direction
);
    int steps = 0;

    while (runtime.running && steps < 100) {
        if (!sb3_runtime_step(&runtime)) {
            fprintf(stderr, "Erreur pendant l'exécution.\n");
            break;
        }

        steps++;

        printf(
            "Step %d | x=%.2f y=%.2f direction=%.2f\n",
            steps,
            runtime.target->x,
            runtime.target->y,
            runtime.target->direction
        );
    }

    printf("\n--- Résultat final ---\n");
    printf("x = %.2f\n", runtime.target->x);
    printf("y = %.2f\n", runtime.target->y);
    printf("direction = %.2f\n", runtime.target->direction);
    printf("steps = %d\n", steps);

    if (runtime.running)
        fprintf(stderr, "Attention : limite de 100 étapes atteinte.\n");

    sb3_runtime_free(&runtime);
    sb3_project_free(project);

    return 0;
}