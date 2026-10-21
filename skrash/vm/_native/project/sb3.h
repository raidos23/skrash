#ifndef SKRASH_SB3_H
#define SKRASH_SB3_H

#include <stddef.h>
#include "asset.h"

typedef enum SB3BlockKind {
    SB3_BLOCK_KIND_UNKNOWN = 0,
    SB3_BLOCK_KIND_EVENT_WHEN_FLAG_CLICKED,
    SB3_BLOCK_KIND_MOTION_MOVE_STEPS,
    SB3_BLOCK_KIND_MOTION_TURN_RIGHT,
    SB3_BLOCK_KIND_MOTION_TURN_LEFT,
    SB3_BLOCK_KIND_CONTROL_REPEAT,
    SB3_BLOCK_KIND_LOOKS_SAY,
    SB3_BLOCK_KIND_CONTROL_FOREVER,
    SB3_BLOCK_KIND_MOTION_GOTOXY,
    SB3_BLOCK_KIND_MOTION_SETX,
    SB3_BLOCK_KIND_MOTION_SETY,
    SB3_BLOCK_KIND_CONTROL_WAIT,
    SB3_BLOCK_KIND_MOTION_POINTINDIRECTION,
    SB3_BLOCK_KIND_MOTION_CHANGEXBY,  
    SB3_BLOCK_KIND_MOTION_CHANGEYBY,
    SB3_BLOCK_KIND_LOOKS_SHOW,  
    SB3_BLOCK_KIND_LOOKS_HIDE,  
    SB3_BLOCK_KIND_LOOKS_SETSIZETO,  
    SB3_BLOCK_KIND_LOOKS_CHANGESIZEBY,  
    SB3_BLOCK_KIND_LOOKS_THINK,
    SB3_BLOCK_KIND_OPERATOR_ADD,
    SB3_BLOCK_KIND_OPERATOR_SUBTRACT,
    SB3_BLOCK_KIND_OPERATOR_MULTIPLY,
    SB3_BLOCK_KIND_OPERATOR_DIVIDE,
    SB3_BLOCK_KIND_OPERATOR_MOD,
    SB3_BLOCK_KIND_OPERATOR_ROUND,
    SB3_BLOCK_KIND_OPERATOR_RANDOM,
    SB3_BLOCK_KIND_OPERATOR_LT,
    SB3_BLOCK_KIND_OPERATOR_GT,
    SB3_BLOCK_KIND_OPERATOR_EQUALS,
    SB3_BLOCK_KIND_OPERATOR_AND,
    SB3_BLOCK_KIND_OPERATOR_OR,
    SB3_BLOCK_KIND_OPERATOR_NOT,
    SB3_BLOCK_KIND_CONTROL_IF,
    SB3_BLOCK_KIND_CONTROL_IF_ELSE,
    SB3_BLOCK_KIND_MOTION_XPOSITION,
    SB3_BLOCK_KIND_MOTION_YPOSITION,
    SB3_BLOCK_KIND_MOTION_DIRECTION,
    SB3_BLOCK_KIND_OPERATOR_JOIN,
    SB3_BLOCK_KIND_OPERATOR_LENGTH,
    SB3_BLOCK_KIND_OPERATOR_CONTAINS,
    SB3_BLOCK_KIND_OPERATOR_LETTER_OF,
    SB3_BLOCK_KIND_OPERATOR_MATHOP,
} SB3BlockKind;

typedef enum SB3InputKind {
    SB3_INPUT_KIND_NONE = 0,
    SB3_INPUT_KIND_LITERAL,    /* [1, [4, "5"]]            */
    SB3_INPUT_KIND_BLOCK,      /* [3, "blockid", [4,"3"]]  */
    SB3_INPUT_KIND_BLOCK_ONLY  /* [2, "blockid"]           */
} SB3InputKind;

typedef struct SB3Input {
    SB3InputKind kind;
    double literal_number;
    char  *literal_string;
    char  *block_id;
} SB3Input;

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
    double repeat_times;
    int has_repeat_times;
    char *substack;
    char *substack_else;
    char *message;
    double motion_x;
    double motion_y;
    double motion_direction;
    int has_motion_gotoxy;
    int has_motion_setx;
    int has_motion_sety;
    int has_motion_pointindirection;
    double wait_seconds;
    int has_wait_seconds;
    double size_value;
    int has_size_value;
    /* entrees generiques (reporters) — posees a l'etape 1,
 * utilisees a partir de l'etape 3 */
    SB3Input input_steps;
    SB3Input input_degrees;
    SB3Input input_times;
    SB3Input input_duration;
    SB3Input input_x;
    SB3Input input_y;
    SB3Input input_direction;
    SB3Input input_size;
    SB3Input input_change;
    SB3Input input_message;
    SB3Input input_num1;
    SB3Input input_num2;
    SB3Input input_condition;
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