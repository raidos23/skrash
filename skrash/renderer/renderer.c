#include "renderer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define NANOSVG_IMPLEMENTATION
#include "nanosvg.h"
#define NANOSVGRAST_IMPLEMENTATION
#include "nanosvgrast.h"
#include "../vm/_native/runtime/runtime.h"


// convertisseur SVG 
static SB3Texture *load_texture_from_svg(SDL_Renderer *sdl,
                                          const unsigned char *svg_data,
                                          size_t svg_size,
                                          float scale)
{
    /* nanosvg veut une string terminee par \0, on duplique */
    char *copy = malloc(svg_size + 1);
    if (copy == NULL) return NULL;
    memcpy(copy, svg_data, svg_size);
    copy[svg_size] = '\0';

    NSVGimage *img = nsvgParse(copy, "px", 96.0f);
    free(copy);
    if (img == NULL) return NULL;

    int w = (int)(img->width * scale);
    int h = (int)(img->height * scale);
    if (w <= 0 || h <= 0) { nsvgDelete(img); return NULL; }

    NSVGrasterizer *rast = nsvgCreateRasterizer();
    if (rast == NULL) { nsvgDelete(img); return NULL; }

    unsigned char *pixels = malloc((size_t)w * h * 4);
    if (pixels == NULL) {
        nsvgDeleteRasterizer(rast);
        nsvgDelete(img);
        return NULL;
    }

    nsvgRasterize(rast, img, 0, 0, scale, pixels, w, h, w * 4);

    nsvgDeleteRasterizer(rast);
    nsvgDelete(img);

    /* nanosvg produit du RGBA pre-multiplie, on convertit en non-premultiplie */
    for (int i = 0; i < w * h; ++i) {
        unsigned char a = pixels[i * 4 + 3];
        if (a != 0 && a != 255) {
            pixels[i*4+0] = (unsigned char)((pixels[i*4+0] * 255) / a);
            pixels[i*4+1] = (unsigned char)((pixels[i*4+1] * 255) / a);
            pixels[i*4+2] = (unsigned char)((pixels[i*4+2] * 255) / a);
        }
    }

    SDL_Texture *tex = SDL_CreateTexture(sdl,
        SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, w, h);
    if (tex == NULL) { free(pixels); return NULL; }

    SDL_UpdateTexture(tex, NULL, pixels, w * 4);
    SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);

    free(pixels);

    SB3Texture *t = malloc(sizeof(*t));
    if (t == NULL) { SDL_DestroyTexture(tex); return NULL; }
    t->tex = tex;
    t->w = w;
    t->h = h;
    return t;
}

/* ------------------------------------------------------------------ */
/* textures                                                            */

static SB3Texture *load_texture_from_memory(SDL_Renderer *sdl,
                                            const unsigned char *data,
                                            size_t size)
{
    int w, h, n;
    unsigned char *pixels = stbi_load_from_memory(data, (int)size,
                                                   &w, &h, &n, 4);
    if (pixels == NULL) return NULL;

    SDL_Texture *tex = SDL_CreateTexture(sdl,
        SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, w, h);
    if (tex == NULL) { stbi_image_free(pixels); return NULL; }

    SDL_UpdateTexture(tex, NULL, pixels, w * 4);
    SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);

    stbi_image_free(pixels);

    SB3Texture *t = malloc(sizeof(*t));
    if (t == NULL) { SDL_DestroyTexture(tex); return NULL; }
    t->tex = tex;
    t->w = w;
    t->h = h;
    return t;
}

SB3Texture *sb3_renderer_get_texture(SB3Renderer *r,
                                     SB3Project *project,
                                     const char *asset_id)
{
    if (r == NULL || project == NULL || asset_id == NULL) return NULL;

    /* cache */
    for (size_t i = 0; i < r->cache_count; ++i) {
        if (strcmp(r->cache[i].asset_id, asset_id) == 0)
            return r->cache[i].tex;
    }

    const SB3Asset *asset = sb3_asset_find(project, asset_id);
    if (asset == NULL || asset->data == NULL || asset->data_size == 0)
        return NULL;

    SB3Texture *tex = NULL;

    if (asset->data_format != NULL &&
        strcmp(asset->data_format, "svg") == 0) {
        tex = load_texture_from_svg(r->sdl,
                                     (const unsigned char *)asset->data,
                                     asset->data_size, 1.0f);
    } else if (asset->data_format != NULL &&
               (strcmp(asset->data_format, "png") == 0 ||
                strcmp(asset->data_format, "jpg") == 0 ||
                strcmp(asset->data_format, "jpeg") == 0 ||
                strcmp(asset->data_format, "bmp") == 0)) {
        tex = load_texture_from_memory(r->sdl,
                                        (const unsigned char *)asset->data,
                                        asset->data_size);
    }

    if (tex == NULL) return NULL;

    /* cache */
    if (r->cache_count == r->cache_cap) {
        size_t new_cap = r->cache_cap ? r->cache_cap * 2 : 16;
        void *new_cache = realloc(r->cache, new_cap * sizeof(*r->cache));
        if (new_cache == NULL) return tex;
        r->cache = new_cache;
        r->cache_cap = new_cap;
    }
    r->cache[r->cache_count].asset_id = strdup(asset_id);
    r->cache[r->cache_count].tex = tex;
    r->cache_count++;

    return tex;
}

/* ------------------------------------------------------------------ */
/* init / free                                                         */

int sb3_renderer_init(SB3Renderer *r, int width, int height, const char *title)
{
    if (r == NULL) return 0;
    memset(r, 0, sizeof(*r));
    r->width = width;
    r->height = height;
    r->running = 1;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 0;
    }

    r->window = SDL_CreateWindow(title,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        width, height, SDL_WINDOW_SHOWN);
    if (r->window == NULL) {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 0;
    }

    r->sdl = SDL_CreateRenderer(r->window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (r->sdl == NULL) {
        fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(r->window);
        SDL_Quit();
        return 0;
    }

    /* mise a l'echelle 1:1, pas d'interpolation */
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
    SDL_RenderSetLogicalSize(r->sdl, width, height);

    return 1;
}

void sb3_renderer_free(SB3Renderer *r)
{
    if (r == NULL) return;
    for (size_t i = 0; i < r->cache_count; ++i) {
        free(r->cache[i].asset_id);
        if (r->cache[i].tex) {
            SDL_DestroyTexture(r->cache[i].tex->tex);
            free(r->cache[i].tex);
        }
    }
    free(r->cache);
    if (r->sdl)    SDL_DestroyRenderer(r->sdl);
    if (r->window) SDL_DestroyWindow(r->window);
    SDL_Quit();
}

/* ------------------------------------------------------------------ */
/* events                                                              */

int sb3_renderer_poll(SB3Renderer *r)
{
    if (r == NULL) return 0;
    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
        switch (ev.type) {
        case SDL_QUIT:
            r->running = 0;
            break;
    
        case SDL_MOUSEMOTION:
            r->mouse_x = ev.motion.x;
            r->mouse_y = ev.motion.y;
            if (r->runtime_ptr) 
                sb3_runtime_set_mouse((SB3Runtime*)r->runtime_ptr, r->mouse_x, r->mouse_y, r->mouse_down);
            break;
        case SDL_MOUSEBUTTONDOWN:
            r->mouse_down = 1;
            if (r->runtime_ptr) 
                sb3_runtime_set_mouse((SB3Runtime*)r->runtime_ptr, r->mouse_x, r->mouse_y, 1);
            break;
        case SDL_MOUSEBUTTONUP:
            r->mouse_down = 0;
            if (r->runtime_ptr) 
                sb3_runtime_set_mouse((SB3Runtime*)r->runtime_ptr, r->mouse_x, r->mouse_y, 0);
            break;
        case SDL_KEYDOWN:
            if (ev.key.keysym.sym == SDLK_ESCAPE) r->running = 0;
            else if (r->runtime_ptr) 
                sb3_runtime_set_key((SB3Runtime*)r->runtime_ptr, ev.key.keysym.scancode, 1);
            break;
        case SDL_KEYUP:
            if (r->runtime_ptr) 
                sb3_runtime_set_key((SB3Runtime*)r->runtime_ptr, ev.key.keysym.scancode, 0);
            break;
    } 
}
    return r->running;
}

/* ------------------------------------------------------------------ */
/* dessin                                                              */

void sb3_renderer_clear(SB3Renderer *r, Uint8 rr, Uint8 gg, Uint8 bb)
{
    if (r == NULL || r->sdl == NULL) return;
    SDL_SetRenderDrawColor(r->sdl, rr, gg, bb, 255);
    SDL_RenderClear(r->sdl);
}

void sb3_renderer_draw_stage(SB3Renderer *r, SB3Project *project)
{
    (void)r; (void)project;
    /* Stage backdrop : a implementer dans une session ulterieure.
     * Necessite le parsing des costumes (SB3Costume). */
}

/* Couleur deterministe par nom de sprite. */
static void sprite_color(const char *name, Uint8 *r, Uint8 *g, Uint8 *b)
{
    unsigned h = 5381;
    if (name)
        for (const char *p = name; *p; ++p)
            h = ((h << 5) + h) + (unsigned char)*p;
    *r = (Uint8)((h >> 16) & 0x7F) + 64;
    *g = (Uint8)((h >>  8) & 0x7F) + 64;
    *b = (Uint8)((h      ) & 0x7F) + 64;
}

/* Dessine un rectangle colore, centre sur (cx, cy), tourne de angle_deg.
 * angle Scratch : 0 = haut, 90 = droite. */
static void draw_rotated_rect(SB3Renderer *r,
                               float cx, float cy,
                               float w, float h,
                               float angle_deg,
                               Uint8 rr, Uint8 gg, Uint8 bb)
{
    SDL_Renderer *sdl = r->sdl;
    float a = (angle_deg - 90.0f) * 3.14159265f / 180.0f;
    float ca = cosf(a), sa = sinf(a);

    float hw = w / 2.0f, hh = h / 2.0f;
    float pts[4][2] = { { -hw, -hh }, { hw, -hh }, { hw, hh }, { -hw, hh } };
    SDL_Vertex v[4];
    for (int i = 0; i < 4; ++i) {
        float x = pts[i][0] * ca - pts[i][1] * sa + cx;
        float y = pts[i][0] * sa + pts[i][1] * ca + cy;
        v[i].position.x = x;
        v[i].position.y = y;
        v[i].color.r = rr / 255.0f;
        v[i].color.g = gg / 255.0f;
        v[i].color.b = bb / 255.0f;
        v[i].color.a = 1.0f;
        v[i].tex_coord.x = 0;
        v[i].tex_coord.y = 0;
    }
    int idx[6] = { 0, 1, 2, 0, 2, 3 };
    SDL_RenderGeometry(sdl, NULL, v, 4, idx, 6);
}

/* Convertit une position Scratch (centre = 0,0, y vers le haut)
 * en coordonnee ecran (origine = coin haut-gauche, y vers le bas).
 * Suppose que la fenetre fait exactement SB3_STAGE_WIDTH x SB3_STAGE_HEIGHT. */
static void scratch_to_screen(float sx_scratch, float sy_scratch,
                               float *sx_screen, float *sy_screen)
{
    *sx_screen = (SB3_STAGE_WIDTH  / 2.0f) + sx_scratch;
    *sy_screen = (SB3_STAGE_HEIGHT / 2.0f) - sy_scratch;
}

void sb3_renderer_draw_targets(SB3Renderer *r, SB3Project *project)
{
    if (r == NULL || r->sdl == NULL || project == NULL) return;

    for (size_t i = 0; i < project->target_count; ++i) {
        SB3Target *t = &project->targets[i];
        if (t->is_stage) continue;
        if (!t->visible) continue;

        float cx, cy;
        scratch_to_screen((float)t->x, (float)t->y, &cx, &cy);

        /* essaie d'afficher le costume courant */
        SB3Texture *tex = NULL;
        if (t->costume_count > 0 && t->current_costume < t->costume_count) {
            SB3Costume *c = &t->costumes[t->current_costume];
            tex = sb3_renderer_get_texture(r, project, c->asset_id);
        }

        if (tex != NULL) {
            float scale = (float)t->size / 100.0f;
            float w = (float)tex->w * scale;
            float h = (float)tex->h * scale;

            SDL_Rect dst = {
                (int)(cx - w / 2),
                (int)(cy - h / 2),
                (int)w,
                (int)h
            };

            /* Scratch : direction 0 = haut. SDL : 0 = droite.
             * On compense : angle = direction - 90. */
            double angle = (double)t->direction - 90.0;
            SDL_Point center = { dst.w / 2, dst.h / 2 };

            SDL_RenderCopyEx(r->sdl, tex->tex, NULL, &dst,
                             angle, &center, SDL_FLIP_NONE);
        } else {
            // fallback : rectangle
            Uint8 rr, gg, bb;
            sprite_color(t->name, &rr, &gg, &bb);
            float w = 100.0f * (float)t->size / 100.0f;
            float h = w;
            draw_rotated_rect(r, cx, cy, w, h,
                              (float)t->direction, rr, gg, bb);
        }
    }
}

void sb3_renderer_present(SB3Renderer *r)
{
    if (r == NULL || r->sdl == NULL) return;
    SDL_RenderPresent(r->sdl);
}