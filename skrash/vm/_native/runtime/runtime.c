#include "runtime.h"
#include "time_util.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static void spawn_hat_thread(SB3Runtime *runtime, SB3Target *target, const SB3Block *hat);

static void sb3_runtime_reset(SB3Runtime *runtime) {
    if (runtime == NULL)
        return;

    runtime->project = NULL;
    runtime->threads = NULL;
    runtime->thread_count = 0;

    /* Initialisation du clavier et de la souris */
    memset(runtime->keys, 0, sizeof(runtime->keys));
    runtime->mouse_down = 0;
    runtime->mouse_x = 0;
    runtime->mouse_y = 0;
    runtime->timer_start = 0;
    runtime->clones = NULL;
    runtime->clone_count = 0;
}

int sb3_runtime_init(SB3Runtime *runtime, SB3Project *project) {
    if (runtime == NULL || project == NULL)
        return 0;

    sb3_runtime_reset(runtime);
    runtime->project = project;
    runtime->timer_start = sb3_now_seconds();

    /* Allocation UNIQUE des deux tableaux fixes */
    runtime->threads = calloc(SB3_MAX_THREADS, sizeof(SB3Thread));
    runtime->clones  = calloc(SB3_MAX_CLONES,  sizeof(SB3Target));

    if (runtime->threads == NULL || runtime->clones == NULL) {
        free(runtime->threads);
        free(runtime->clones);
        return 0;
    }

    return 1;
}
void sb3_runtime_free(SB3Runtime *runtime) {
    if (runtime == NULL) return;

    for (size_t i = 0; i < runtime->clone_count; ++i) {
        SB3Target *c = &runtime->clones[i];

        free(c->name);

        for (size_t k = 0; k < c->variable_count; ++k) {
            free(c->variables[k].id);
            free(c->variables[k].name);
        }
        free(c->variables);

        for (size_t k = 0; k < c->list_count; ++k) {
            free(c->lists[k].id);
            free(c->lists[k].name);
            for (size_t m = 0; m < c->lists[k].item_count; ++m)
                free(c->lists[k].items[m]);
            free(c->lists[k].items);
        }
        free(c->lists);

        /* NE PAS free blocks ni costumes : appartiennent au projet */
    }

    free(runtime->clones);
    free(runtime->threads);
    sb3_runtime_reset(runtime);
}

int sb3_runtime_start(SB3Runtime *runtime) {
    if (runtime == NULL || runtime->project == NULL)
        return 0;

    /* Le tableau est alloue une fois pour toutes dans sb3_runtime_init.
       On le vide simplement. */
    runtime->thread_count = 0;

    /* 1. Compter le nombre total de chapeaux */
    size_t count = 0;
    for (size_t i = 0; i < runtime->project->target_count; ++i) {
        SB3Target *target = &runtime->project->targets[i];
        for (size_t j = 0; j < target->block_count; ++j) {
            SB3Block *b = &target->blocks[j];
            if (b->kind == SB3_BLOCK_KIND_EVENT_WHEN_FLAG_CLICKED && b->next != NULL) {
                ++count;
            }
        }
    }

    if (count == 0)
        return 0;

    if (count > SB3_MAX_THREADS)
        return 0; /* Au cas ou */

    /* 2. Créer un thread pour CHAQUE chapeau trouvé */
    for (size_t i = 0; i < runtime->project->target_count; ++i) {
        SB3Target *target = &runtime->project->targets[i];
        for (size_t j = 0; j < target->block_count; ++j) {
            SB3Block *b = &target->blocks[j];

            if (b->kind == SB3_BLOCK_KIND_EVENT_WHEN_FLAG_CLICKED && b->next != NULL) {
                const SB3Block *first = sb3_target_find_block(target, b->next);
                if (first == NULL) continue;

                SB3Thread *thread = &runtime->threads[runtime->thread_count++];
                memset(thread, 0, sizeof(*thread));
                thread->runtime = runtime;
                thread->target = target;
                thread->current_block = first;
                fprintf(stderr, "[START] thread for %s chapeau=%s\n",
                        target->name, b->opcode);
                thread->running = 1;
            }
        }
    }

    return runtime->thread_count > 0;
}

int sb3_runtime_step(SB3Runtime *runtime) {
    if (runtime == NULL || runtime->threads == NULL)
        return 0;
    if (runtime->project == NULL)
        return 0;

    /* 1. Verification du chrono (front montant uniquement) */
    double elapsed = sb3_now_seconds() - runtime->timer_start;

    for (size_t i = 0; i < runtime->project->target_count; ++i) {
        SB3Target *target = &runtime->project->targets[i];

        for (size_t j = 0; j < target->block_count; ++j) {
            SB3Block *b = &target->blocks[j];

            if (b->kind != SB3_BLOCK_KIND_EVENT_WHENGREATERTHAN)
                continue;

            double val = b->input_num1.literal_number;
            int over = (elapsed > val);

            /* Ne spawn qu'au passage faux -> vrai.
             * Le flag est stocke sur le bloc : safe car le bloc
             * n'est jamais partage entre plusieurs runtimes vivants. */
            if (over && !b->greater_than_latched) {
                b->greater_than_latched = 1;
                spawn_hat_thread(runtime, target, b);
            } else if (!over && b->greater_than_latched) {
                b->greater_than_latched = 0;
            }
        }
    }

    /* 2. Avancement normal des threads.
     *
     * On fige le nombre au debut, mais on doit aussi garantir que
     * les threads ajoutes pendant cette etape (spawn_hat_thread via
     * broadcast, click, etc.) tournent dans la MEME frame si possible.
     *
     * Strategie : boucle tant qu'il y a des threads non encore
     * "vus" a l'index courant. Chaque thread tourne au plus UNE fois
     * par step (via le compteur next_to_run).
     */
    int any_running = 0;
    size_t next_to_run = 0;

    while (next_to_run < runtime->thread_count) {
        size_t current = next_to_run++;
        SB3Thread *th = &runtime->threads[current];

        if (!th->running)
            continue;

        sb3_thread_step(th);

        if (th->running)
            any_running = 1;
    }

    return any_running;
}

static int scratch_key_to_sdl(const char *name) {
    if (!name) return -1;
    if (strcmp(name, "space") == 0) return 44;
    if (strcmp(name, "right arrow") == 0) return 79;
    if (strcmp(name, "left arrow") == 0) return 80;
    if (strcmp(name, "up arrow") == 0) return 82;
    if (strcmp(name, "down arrow") == 0) return 81;
    if (name[0] >= 'a' && name[0] <= 'z' && name[1] == '\0') return 4 + (name[0] - 'a');
    return -1;
}

void sb3_runtime_set_key(SB3Runtime *runtime, int scancode, int is_down) {
    if (runtime && scancode >= 0 && scancode < 512)
        runtime->keys[scancode] = is_down;
}

void sb3_runtime_set_mouse(SB3Runtime *runtime, int x, int y, int down) {
    if (runtime) {
        runtime->mouse_x = x;
        runtime->mouse_y = y;
        runtime->mouse_down = down;
    }
}

int sb3_runtime_is_key_pressed(SB3Runtime *runtime, const char *key_name) {
    if (!runtime || !key_name) return 0;
    if (strcmp(key_name, "any") == 0) {
        for (int i = 0; i < 512; i++) if (runtime->keys[i]) return 1;
        return 0;
    }
    int sc = scratch_key_to_sdl(key_name);
    return (sc >= 0) ? runtime->keys[sc] : 0;
}

/* Fonction utilitaire interne pour créer un thread à partir d'un chapeau */
static void spawn_hat_thread(SB3Runtime *runtime, SB3Target *target, const SB3Block *hat) {
    if (!hat->next) return;

    // Plus de realloc Le tableau est fixe
    if (runtime->thread_count >= SB3_MAX_THREADS) return;

    SB3Thread *th = &runtime->threads[runtime->thread_count++];
    memset(th, 0, sizeof(*th));
    th->runtime = runtime;
    th->target = target;
    th->current_block = sb3_target_find_block(target, hat->next);
    th->running = (th->current_block != NULL);
}

void sb3_runtime_trigger_key_hats(SB3Runtime *runtime, int scancode) {
    if (!runtime || !runtime->project) return;

    for (size_t i = 0; i < runtime->project->target_count; ++i) {
        SB3Target *target = &runtime->project->targets[i];
        for (size_t j = 0; j < target->block_count; ++j) {
            SB3Block *b = &target->blocks[j];

            if (b->kind == SB3_BLOCK_KIND_EVENT_WHENKEYPRESSED && b->event_option) {
                int hat_sc = scratch_key_to_sdl(b->event_option);
                if (hat_sc == scancode || strcmp(b->event_option, "any") == 0) {
                    spawn_hat_thread(runtime, target, b);
                }
            }
        }
    }
}

void sb3_runtime_broadcast(SB3Runtime *runtime, const char *message) {
    if (!runtime || !runtime->project || !message) return;

    for (size_t i = 0; i < runtime->project->target_count; ++i) {
        SB3Target *target = &runtime->project->targets[i];
        for (size_t j = 0; j < target->block_count; ++j) {
            SB3Block *b = &target->blocks[j];
            if (b->kind == SB3_BLOCK_KIND_EVENT_WHENBROADCASTRECEIVED && b->broadcast_name) {
                if (strcmp(b->broadcast_name, message) == 0) {
                    spawn_hat_thread(runtime, target, b);
                }
            }
        }
    }
}

void sb3_runtime_trigger_clicks(SB3Runtime *runtime, int x, int y, SB3Target *clicked_target) {
    (void)x;
    (void)y;
    if (!runtime || !clicked_target) return;

    for (size_t j = 0; j < clicked_target->block_count; ++j) {
        SB3Block *b = &clicked_target->blocks[j];

        /* Si c'est le Stage, on cherche when_stage_clicked */
        if (clicked_target->is_stage && b->kind == SB3_BLOCK_KIND_EVENT_WHENSTAGECLICKED) {
            spawn_hat_thread(runtime, clicked_target, b);
        }
        /* Si c'est un sprite, on cherche when_this_sprite_clicked */
        else if (!clicked_target->is_stage && b->kind == SB3_BLOCK_KIND_EVENT_WHENTHISSPRITECLICKED) {
            spawn_hat_thread(runtime, clicked_target, b);
        }
    }
}

void sb3_runtime_broadcast_and_wait(SB3Runtime *runtime, SB3Thread *calling_thread, const char *message) {
    // 1. On lance les threads
    size_t start_count = runtime->thread_count;
    sb3_runtime_broadcast(runtime, message);

    // 2. On dit au thread appelant d'attendre
    calling_thread->waiting_for_broadcast = 1;
    calling_thread->broadcast_start_count = start_count;
}

void sb3_runtime_create_clone(SB3Runtime *runtime, SB3Target *original) {
    if (!runtime || !original) return;
    if (runtime->clone_count >= SB3_MAX_CLONES) return;

    SB3Target *clone = &runtime->clones[runtime->clone_count++];
    *clone = *original;

    clone->is_clone = 1;

    /* Nom : duplication profonde (le nom est possede par le clone) */
    clone->name = original->name ? sb3_strdup_local(original->name) : NULL;

    /* Variables : copie par valeur (shallow copy des champs, deep copy
     * des allocations id/name). Les valeurs sont independantes. */
    if (original->variable_count > 0) {
        clone->variables = calloc(original->variable_count, sizeof(SB3Variable));
        if (clone->variables == NULL) {
            /* rollback : on rend le slot */
            runtime->clone_count--;
            free(clone->name);
            return;
        }
        for (size_t i = 0; i < original->variable_count; ++i) {
            clone->variables[i].id       = original->variables[i].id
                                           ? sb3_strdup_local(original->variables[i].id) : NULL;
            clone->variables[i].name     = original->variables[i].name
                                           ? sb3_strdup_local(original->variables[i].name) : NULL;
            clone->variables[i].value    = original->variables[i].value;
            clone->variables[i].is_cloud = original->variables[i].is_cloud;
        }
        clone->variable_count = original->variable_count;
    }

    /* Listes : meme chose, deep copy des items */
    if (original->list_count > 0) {
        clone->lists = calloc(original->list_count, sizeof(SB3List));
        if (clone->lists == NULL) {
            /* rollback simplifie : on laisse les variables, elles seront
             * liberees par sb3_runtime_free */
            runtime->clone_count--;
            free(clone->name);
            return;
        }
        for (size_t i = 0; i < original->list_count; ++i) {
            SB3List *src = &original->lists[i];
            SB3List *dst = &clone->lists[i];

            dst->id   = src->id   ? sb3_strdup_local(src->id)   : NULL;
            dst->name = src->name ? sb3_strdup_local(src->name) : NULL;
            dst->item_count = src->item_count;

            if (src->item_count > 0) {
                dst->items = calloc(src->item_count, sizeof(char *));
                if (dst->items == NULL) {
                    dst->item_count = 0;
                    continue;
                }
                for (size_t k = 0; k < src->item_count; ++k) {
                    dst->items[k] = src->items[k] ? sb3_strdup_local(src->items[k]) : NULL;
                }
            }
        }
        clone->list_count = original->list_count;
    }

    /* Les blocs et costumes restent PARTAGES : ils sont immutables. */

    /* Spawn du thread "when I start as a clone" */
    for (size_t j = 0; j < original->block_count; ++j) {
        SB3Block *b = &original->blocks[j];
        if (b->kind == SB3_BLOCK_KIND_CONTROL_STARTASCLONE && b->next != NULL) {
            if (runtime->thread_count >= SB3_MAX_THREADS) return;

            SB3Thread *th = &runtime->threads[runtime->thread_count++];
            memset(th, 0, sizeof(*th));
            th->runtime       = runtime;
            th->target        = clone;
            th->current_block = sb3_target_find_block(clone, b->next);
            th->running       = (th->current_block != NULL);
            break;
        }
    }
}