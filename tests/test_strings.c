#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "project/sb3.h"
#include "runtime/runtime.h"

static void run_and_capture(SB3Runtime *runtime, char *buffer, size_t size)
{
    fflush(stdout);
    FILE *saved = stdout;
    FILE *tmp = tmpfile();
    if (!tmp) return;
    stdout = tmp;
    while (sb3_runtime_step(runtime))
        ;
    fflush(stdout);
    stdout = saved;
    rewind(tmp);
    size_t n = fread(buffer, 1, size - 1, tmp);
    buffer[n] = '\0';
    fclose(tmp);
}

int main(void)
{
    SB3Project *project = sb3_load("./tests/checks/strings.sb3");
    if (project == NULL) {
        fprintf(stderr, "error: failed to load strings.sb3\n");
        return 1;
    }

    SB3Runtime runtime;
    if (!sb3_runtime_init(&runtime, project) ||
        !sb3_runtime_start(&runtime)) {
        sb3_project_free(project);
        return 1;
    }

    char output[4096] = {0};
    run_and_capture(&runtime, output, sizeof(output));

    const char *expected =
        "(say) bonjour\n"
        "(say) 7\n"
        "(say) 1\n"
        "(say) 0\n"
        "(say) Score:5\n";

    int ok = (strcmp(output, expected) == 0);

    if (!ok) {
        fprintf(stderr, "=== FAIL ===\n");
        fprintf(stderr, "--- attendu ---\n%s", expected);
        fprintf(stderr, "--- obtenu ---\n%s", output);
    } else {
        printf("OK: strings conformes\n%s", output);
    }

    sb3_runtime_free(&runtime);
    sb3_project_free(project);
    return ok ? 0 : 1;
}