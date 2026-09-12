#ifndef SKRASH_SB3_H
#define SKRASH_SB3_H

#include <stddef.h>

struct SB3Asset;

typedef struct SB3Asset SB3Asset;

typedef enum SB3BlockKind {
    SB3_BLOCK_KIND_UNKNOWN = 0,
    SB3_BLOCK_KIND_EVENT_WHEN_FLAG_CLICKED,
    SB3_BLOCK_KIND_MOTION_MOVE_STEPS
} SB3BlockKind;

typedef struct SB3Block {
    char *block_id;
    char *opcode;
    char *next;
    char *parent;
    int shadow;
    int top_level;
    double x;
    double y;
    SB3BlockKind kind;
    double motion_steps;
    int has_motion_steps;
} SB3Block;

typedef struct SB3Target {
    char *name;
    int is_stage;
    SB3Block *blocks;
    size_t block_count;
} SB3Target;

/*
 * Représente un projet Scratch 3 charge en memoire.
 */
typedef struct SB3Project {
    char *project_json;
    size_t project_json_size;
    SB3Asset *assets;
    size_t asset_count;
    SB3Target *targets;
    size_t target_count;
} SB3Project;


/*
 * Charge un fichier .sb3 et construit le modele interne.
 */
SB3Project *sb3_load(const char *path);


/*
 * Libere un projet charge par sb3_load().
 */
void sb3_project_free(SB3Project *project);

/*
 * Alias de compatibilite.
 */
void sb3_destroy(SB3Project *project);

/*
 * Recherche un asset par son identifiant Scratch.
 */
const SB3Asset *sb3_asset_find(const SB3Project *project, const char *asset_id);

/*
 * Retourne le type d'un bloc Scratch connu a partir de son opcode.
 */
SB3BlockKind sb3_block_kind_from_opcode(const char *opcode);

/*
 * Recherche un bloc dans un target par son identifiant.
 */
const SB3Block *sb3_target_find_block(const SB3Target *target, const char *block_id);

/*
 * Recherche le premier bloc d'un type donne dans un target.
 */
const SB3Block *sb3_target_find_block_by_kind(const SB3Target *target, SB3BlockKind kind);

#endif /* SKRASH_SB3_H */
