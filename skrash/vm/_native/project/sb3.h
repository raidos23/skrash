#ifndef SKRASH_SB3_H
#define SKRASH_SB3_H

#include <stddef.h>
#include "asset.h"

typedef enum SB3BlockKind {
    SB3_BLOCK_KIND_UNKNOWN = 0,
    SB3_BLOCK_KIND_EVENT_WHEN_FLAG_CLICKED,
    SB3_BLOCK_KIND_MOTION_MOVE_STEPS,
    SB3_BLOCK_KIND_MOTION_TURN_RIGHT
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
    double motion_turn_degrees;
    int has_motion_turn_degrees;
} SB3Block;

typedef struct SB3Target {
    char *name;
    int is_stage;

    double x;
    double y;
    double direction;
    double size;
    int visible;

    SB3Block *blocks;
    size_t block_count;
} SB3Target;

/*
 * Représente un projet Scratch 3 chargé en mémoire.
 */
typedef struct SB3Project {
    char *project_json;
    size_t project_json_size;
    SB3Asset *assets;
    size_t asset_count;
    SB3Target *targets;
    size_t target_count;
} SB3Project;

SB3Project *sb3_load(const char *path);

void sb3_project_free(SB3Project *project);

void sb3_destroy(SB3Project *project);

const SB3Asset *sb3_asset_find(
    const SB3Project *project,
    const char *asset_id
);

SB3BlockKind sb3_block_kind_from_opcode(const char *opcode);

const SB3Block *sb3_target_find_block(
    const SB3Target *target,
    const char *block_id
);

const SB3Block *sb3_target_find_block_by_kind(
    const SB3Target *target,
    SB3BlockKind kind
);

#endif /* SKRASH_SB3_H */