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
    SB3_BLOCK_KIND_DATA_SETVARIABLETO,
    SB3_BLOCK_KIND_DATA_CHANGEVARIABLEBY,
    SB3_BLOCK_KIND_DATA_SHOWVARIABLE,
    SB3_BLOCK_KIND_DATA_HIDEVARIABLE,
    SB3_BLOCK_KIND_DATA_VARIABLE,
    SB3_BLOCK_KIND_DATA_ADDTOLIST,
    SB3_BLOCK_KIND_DATA_DELETEOFLIST,
    SB3_BLOCK_KIND_DATA_DELETEALLOFLIST,
    SB3_BLOCK_KIND_DATA_INSERTATLIST,
    SB3_BLOCK_KIND_DATA_REPLACEITEMOFLIST,
    SB3_BLOCK_KIND_DATA_ITEMOFLIST,
    SB3_BLOCK_KIND_DATA_ITEMNUMOFLIST,
    SB3_BLOCK_KIND_DATA_LENGTHOFLIST,
    SB3_BLOCK_KIND_DATA_LISTCONTAINSITEM,
    SB3_BLOCK_KIND_DATA_SHOWLIST,
    SB3_BLOCK_KIND_DATA_HIDELIST,
    SB3_BLOCK_KIND_DATA_LISTCONTENTS,
    SB3_BLOCK_KIND_LOOKS_NEXTCOSTUME,
    SB3_BLOCK_KIND_SENSING_KEYPRESSED,
    SB3_BLOCK_KIND_EVENT_WHENKEYPRESSED,
    SB3_BLOCK_KIND_SENSING_KEYOPTIONS,
    SB3_BLOCK_KIND_SENSING_MOUSEDOWN,
    SB3_BLOCK_KIND_SENSING_MOUSEX,
    SB3_BLOCK_KIND_SENSING_MOUSEY,
    SB3_BLOCK_KIND_EVENT_BROADCAST,
    SB3_BLOCK_KIND_EVENT_BROADCASTANDWAIT,
    SB3_BLOCK_KIND_EVENT_WHENBROADCASTRECEIVED,
    SB3_BLOCK_KIND_EVENT_WHENTHISSPRITECLICKED,
    SB3_BLOCK_KIND_EVENT_WHENSTAGECLICKED,
    SB3_BLOCK_KIND_EVENT_WHENGREATERTHAN,
    SB3_BLOCK_KIND_SENSING_TOUCHINGOBJECT,
    SB3_BLOCK_KIND_SENSING_TOUCHINGOBJECTMENU,
    SB3_BLOCK_KIND_CONTROL_CREATECLONEOF,
    SB3_BLOCK_KIND_CONTROL_CREATECLONEOFMENU,
    SB3_BLOCK_KIND_CONTROL_DELETETHISCLONE,
    SB3_BLOCK_KIND_CONTROL_STARTASCLONE,
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
    char *variable_name;
    char *variable_id;
    char *list_name;
    char *list_id;
    char *broadcast_name;
    char *event_option;  /* pour KEY_OPTION, BROADCAST_OPTION, WHENGREATERTHANMENU */
} SB3Block;

typedef struct SB3Variable {
    char *id;
    char *name;
    double value;
    int is_cloud;
} SB3Variable;

typedef struct SB3List {
    char *id;
    char *name;
    /* On stocke les valeurs comme des chaines
    Chaque element peut etre un nombre ou un texte. */
    char **items;
    size_t item_count;
} SB3List;

typedef struct SB3Costume {
    char *name;
    char *asset_id;
    char *md5ext;
    char *data_format;
    int width; 
    int height;  
} SB3Costume;

typedef struct SB3Target {
    char *name;
    int is_stage;

    double x;
    double y;
    double direction;
    double size;
    int visible;
    int is_clone;

    SB3Block *blocks;
    size_t block_count;
    SB3Variable *variables;
    size_t variable_count;
    SB3List *lists;
    size_t list_count;
    SB3Costume *costumes;
    size_t costume_count;
    size_t current_costume;

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

SB3Variable *sb3_project_find_variable(SB3Project *project, SB3Target *context, const char *name);
SB3List *sb3_project_find_list(SB3Project *project, SB3Target *context, const char *name);
char *sb3_strdup_local(const char *text);

#endif /* SKRASH_SB3_H */