#include "runtime.h"
#include "time_util.h"
#include <stdlib.h>
#include <string.h>

static void spawn_hat_thread(SB3Runtime *runtime, SB3Target *target, const SB3Block *hat);

static void sb3_runtime_reset(SB3Runtime *runtime)
{
    if (runtime == NULL)
        return;

    runtime->project = NULL;
    runtime->threads = NULL;
    runtime->thread_count = 0;
    
    /* Initialisation du clavier et de la souris */
    memset(runtime->keys, 0, sizeof(runtime->keys));
    runtime->mouse_down = 0;
    runtime->mouse_x = 240;  /* Centre de l'écran Scratch (480/2) */
    runtime->mouse_y = 180;  /* Centre de l'écran Scratch (360/2) */
    runtime->timer_start = 0;
}

int sb3_runtime_init(SB3Runtime *runtime, SB3Project *project)
{
    if (runtime == NULL || project == NULL)
        return 0;

    sb3_runtime_reset(runtime);
    runtime->project = project;
    runtime->timer_start = sb3_now_seconds();


    return 1;
}

void sb3_runtime_free(SB3Runtime *runtime)
{
    if (runtime == NULL)
        return;

    free(runtime->threads);
    sb3_runtime_reset(runtime);
}

int sb3_runtime_start(SB3Runtime *runtime)
{
    if (runtime == NULL || runtime->project == NULL)
        return 0;

    free(runtime->threads);
    runtime->threads = NULL;
    runtime->thread_count = 0;

    /* 1. Compter le nombre total de chapeaux (flag + key + etc.) */
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

    runtime->threads = calloc(count, sizeof(*runtime->threads));
    if (runtime->threads == NULL)
        return 0;

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
                thread->running = 1;
            }
        }
    }

    return runtime->thread_count > 0;
}

int sb3_runtime_step(SB3Runtime *runtime) {
    if (runtime == NULL || runtime->threads == NULL) return 0;

    /* 1. Verification du chrono */
    double elapsed = sb3_now_seconds() - runtime->timer_start;
    for (size_t i = 0; i < runtime->project->target_count; ++i) {
        SB3Target *target = &runtime->project->targets[i];
        for (size_t j = 0; j < target->block_count; ++j) {
            SB3Block *b = &target->blocks[j];
            if (b->kind == SB3_BLOCK_KIND_EVENT_WHENGREATERTHAN) {
                double val = b->input_num1.literal_number;
                if (elapsed > val) {
                    spawn_hat_thread(runtime, target, b);
                }
            }
        }
    }

    /* 2. Avancement normal des threads (on fige la taille au debut) */
    int any_running = 0;
    size_t count_at_start = runtime->thread_count; 
    
    for (size_t i = 0; i < count_at_start; ++i) {
        SB3Thread *th = &runtime->threads[i];
        if (!th->running) continue;
        sb3_thread_step(th);
        if (th->running) any_running = 1;
    }
    
    /* Si de nouveaux threads ont ete crees pendant l'execution, on continue */
    if (runtime->thread_count > count_at_start) {
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
    SB3Thread *new_threads = realloc(runtime->threads, (runtime->thread_count + 1) * sizeof(SB3Thread));
    if (!new_threads) return;
    runtime->threads = new_threads;
    
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
    (void)x; (void)y;
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