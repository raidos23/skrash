#ifndef SKRASH_RENDERER_H
#define SKRASH_RENDERER_H

#include <SDL2/SDL.h>
#include "../vm/_native/project/sb3.h"

/* Taille native du stage Scratch */
#define SB3_STAGE_WIDTH  480
#define SB3_STAGE_HEIGHT 360

typedef struct SB3Texture {
    SDL_Texture *tex;
    int          w;
    int          h;
} SB3Texture;

typedef struct SB3Renderer {
    SDL_Window   *window;
    SDL_Renderer *sdl;
    int           width;
    int           height;
    int           running;

    /* cache textures : un par asset_id */
    struct {
        char        *asset_id;
        SB3Texture  *tex;
    } *cache;
    size_t cache_count;
    size_t cache_cap;

    /* etat clavier/souris */
    int  key_down[SDL_NUM_SCANCODES];
    int  mouse_down;
    int  mouse_x;
    int  mouse_y;
} SB3Renderer;

int  sb3_renderer_init(SB3Renderer *r, int width, int height, const char *title);
void sb3_renderer_free(SB3Renderer *r);
int  sb3_renderer_poll(SB3Renderer *r);
void sb3_renderer_clear(SB3Renderer *r, Uint8 rr, Uint8 gg, Uint8 bb);
void sb3_renderer_draw_stage(SB3Renderer *r, SB3Project *project);
void sb3_renderer_draw_targets(SB3Renderer *r, SB3Project *project);
void sb3_renderer_present(SB3Renderer *r);

SB3Texture *sb3_renderer_get_texture(SB3Renderer *r,
                                     SB3Project *project,
                                     const char *asset_id);

#endif