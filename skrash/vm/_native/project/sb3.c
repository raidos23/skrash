#include "sb3.h"

#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <zip.h>

#include "asset.h"

typedef enum JsonType {
    JSON_NULL,
    JSON_BOOL,
    JSON_NUMBER,
    JSON_STRING,
    JSON_ARRAY,
    JSON_OBJECT
} JsonType;

typedef struct JsonValue JsonValue;

typedef struct JsonPair {
    char *key;
    JsonValue *value;
} JsonPair;

struct JsonValue {
    JsonType type;
    union {
        int boolean;
        double number;
        char *string;
        struct {
            JsonValue **items;
            size_t count;
        } array;
        struct {
            JsonPair *pairs;
            size_t count;
        } object;
    } as;
};

static char *sb3_strdup_local(const char *text)
{
    size_t length;
    char *copy;

    if (text == NULL)
        return NULL;

    length = strlen(text);
    copy = malloc(length + 1);
    if (copy == NULL)
        return NULL;

    memcpy(copy, text, length + 1);
    return copy;
}

static void *sb3_calloc(size_t count, size_t size)
{
    if (count == 0 || size == 0)
        return calloc(count, size);

    if (size != 0 && count > (SIZE_MAX / size))
        return NULL;

    return calloc(count, size);
}

static void sb3_block_init(SB3Block *block)
{
    if (block == NULL)
        return;

    block->block_id = NULL;
    block->opcode = NULL;
    block->next = NULL;
    block->parent = NULL;
    block->shadow = 0;
    block->top_level = 0;
    block->x = 0.0;
    block->y = 0.0;
    block->kind = SB3_BLOCK_KIND_UNKNOWN;
    block->motion_steps = 0.0;
    block->has_motion_steps = 0;
    block->motion_turn_degrees = 0.0;
    block->has_motion_turn_degrees = 0;
}

static void sb3_target_init(SB3Target *target)
{
    if (target == NULL)
        return;

    target->name = NULL;
    target->is_stage = 0;
    target->blocks = NULL;
    target->block_count = 0;
    target->x = 0.0;
    target->y = 0.0;
    target->direction = 90.0;
    target->size = 100.0;
    target->visible = 1;
    
}

static void sb3_project_init(SB3Project *project)
{
    if (project == NULL)
        return;

    project->project_json = NULL;
    project->project_json_size = 0;
    project->assets = NULL;
    project->asset_count = 0;
    project->targets = NULL;
    project->target_count = 0;
}

static JsonValue *json_new(JsonType type)
{
    JsonValue *value = calloc(1, sizeof(*value));

    if (value != NULL)
        value->type = type;

    return value;
}

static const char *json_skip_ws(const char *cursor, const char *end)
{
    while (cursor < end && isspace((unsigned char)*cursor))
        ++cursor;

    return cursor;
}

static int json_append_char(char **buffer, size_t *length, size_t *capacity, char ch)
{
    char *next;

    if (*length + 1 >= *capacity) {
        size_t new_capacity = (*capacity == 0) ? 32 : (*capacity * 2);

        next = realloc(*buffer, new_capacity);
        if (next == NULL)
            return 0;

        *buffer = next;
        *capacity = new_capacity;
    }

    (*buffer)[(*length)++] = ch;
    (*buffer)[*length] = '\0';
    return 1;
}

static int json_hex_value(char ch)
{
    if (ch >= '0' && ch <= '9')
        return ch - '0';
    if (ch >= 'a' && ch <= 'f')
        return 10 + (ch - 'a');
    if (ch >= 'A' && ch <= 'F')
        return 10 + (ch - 'A');
    return -1;
}

static const char *json_parse_string_raw(const char *cursor, const char *end, char **out)
{
    char *buffer = NULL;
    size_t length = 0;
    size_t capacity = 0;

    if (cursor >= end || *cursor != '"')
        return NULL;

    ++cursor;
    while (cursor < end) {
        char ch = *cursor++;

        if (ch == '"') {
            if (!json_append_char(&buffer, &length, &capacity, '\0')) {
                free(buffer);
                return NULL;
            }

            *out = buffer;
            return cursor;
        }

        if (ch == '\\') {
            int value;

            if (cursor >= end) {
                free(buffer);
                return NULL;
            }

            ch = *cursor++;
            switch (ch) {
            case '"':
            case '\\':
            case '/':
                break;
            case 'b':
                ch = '\b';
                break;
            case 'f':
                ch = '\f';
                break;
            case 'n':
                ch = '\n';
                break;
            case 'r':
                ch = '\r';
                break;
            case 't':
                ch = '\t';
                break;
            case 'u':
                if (cursor + 4 > end) {
                    free(buffer);
                    return NULL;
                }

                value = json_hex_value(cursor[0]);
                value = (value < 0) ? -1 : (value << 4) | json_hex_value(cursor[1]);
                value = (value < 0) ? -1 : (value << 4) | json_hex_value(cursor[2]);
                value = (value < 0) ? -1 : (value << 4) | json_hex_value(cursor[3]);
                cursor += 4;
                ch = (value < 0) ? '?' : (char)value;
                break;
            default:
                free(buffer);
                return NULL;
            }
        }

        if (!json_append_char(&buffer, &length, &capacity, ch)) {
            free(buffer);
            return NULL;
        }
    }

    free(buffer);
    return NULL;
}

static JsonValue *json_parse_value(const char **cursor, const char *end);
static void json_value_free(JsonValue *value);

static JsonValue *json_parse_array(const char **cursor, const char *end)
{
    JsonValue *value = json_new(JSON_ARRAY);
    const char *p;

    if (value == NULL)
        return NULL;

    p = *cursor + 1;
    p = json_skip_ws(p, end);

    if (p < end && *p == ']') {
        *cursor = p + 1;
        return value;
    }

    while (p < end) {
        JsonValue *item = json_parse_value(&p, end);
        JsonValue **items;

        if (item == NULL) {
            json_value_free(value);
            return NULL;
        }

        items = realloc(value->as.array.items, (value->as.array.count + 1) * sizeof(*items));
        if (items == NULL) {
            json_value_free(item);
            json_value_free(value);
            return NULL;
        }

        value->as.array.items = items;
        value->as.array.items[value->as.array.count++] = item;

        p = json_skip_ws(p, end);
        if (p >= end) {
            json_value_free(value);
            return NULL;
        }

        if (*p == ',') {
            p = json_skip_ws(p + 1, end);
            continue;
        }

        if (*p == ']') {
            *cursor = p + 1;
            return value;
        }

        json_value_free(value);
        return NULL;
    }

    json_value_free(value);
    return NULL;
}

static JsonValue *json_parse_object(const char **cursor, const char *end)
{
    JsonValue *value = json_new(JSON_OBJECT);
    const char *p;

    if (value == NULL)
        return NULL;

    p = *cursor + 1;
    p = json_skip_ws(p, end);

    if (p < end && *p == '}') {
        *cursor = p + 1;
        return value;
    }

    while (p < end) {
        char *key = NULL;
        JsonValue *member;
        JsonPair *pairs;

        p = json_parse_string_raw(p, end, &key);
        if (p == NULL) {
            json_value_free(value);
            return NULL;
        }

        p = json_skip_ws(p, end);
        if (p >= end || *p != ':') {
            free(key);
            json_value_free(value);
            return NULL;
        }

        p = json_skip_ws(p + 1, end);
        member = json_parse_value(&p, end);
        if (member == NULL) {
            free(key);
            json_value_free(value);
            return NULL;
        }

        pairs = realloc(value->as.object.pairs, (value->as.object.count + 1) * sizeof(*pairs));
        if (pairs == NULL) {
            free(key);
            json_value_free(member);
            json_value_free(value);
            return NULL;
        }

        value->as.object.pairs = pairs;
        value->as.object.pairs[value->as.object.count].key = key;
        value->as.object.pairs[value->as.object.count].value = member;
        value->as.object.count++;

        p = json_skip_ws(p, end);
        if (p >= end) {
            json_value_free(value);
            return NULL;
        }

        if (*p == ',') {
            p = json_skip_ws(p + 1, end);
            continue;
        }

        if (*p == '}') {
            *cursor = p + 1;
            return value;
        }

        json_value_free(value);
        return NULL;
    }

    json_value_free(value);
    return NULL;
}

static JsonValue *json_parse_value(const char **cursor, const char *end)
{
    const char *p = json_skip_ws(*cursor, end);
    JsonValue *value = NULL;

    if (p >= end)
        return NULL;

    switch (*p) {
    case '"':
        {
            char *string = NULL;

            p = json_parse_string_raw(p, end, &string);
            if (p == NULL)
                return NULL;

            value = json_new(JSON_STRING);
            if (value == NULL) {
                free(string);
                return NULL;
            }

            value->as.string = string;
            *cursor = p;
            return value;
        }
    case '{':
        value = json_parse_object(&p, end);
        if (value != NULL)
            *cursor = p;
        return value;
    case '[':
        value = json_parse_array(&p, end);
        if (value != NULL)
            *cursor = p;
        return value;
    case 't':
        if (end - p >= 4 && memcmp(p, "true", 4) == 0) {
            value = json_new(JSON_BOOL);
            if (value == NULL)
                return NULL;
            value->as.boolean = 1;
            *cursor = p + 4;
            return value;
        }
        return NULL;
    case 'f':
        if (end - p >= 5 && memcmp(p, "false", 5) == 0) {
            value = json_new(JSON_BOOL);
            if (value == NULL)
                return NULL;
            value->as.boolean = 0;
            *cursor = p + 5;
            return value;
        }
        return NULL;
    case 'n':
        if (end - p >= 4 && memcmp(p, "null", 4) == 0) {
            value = json_new(JSON_NULL);
            if (value == NULL)
                return NULL;
            *cursor = p + 4;
            return value;
        }
        return NULL;
    default:
        {
            char *tail = NULL;
            double number;

            errno = 0;
            number = strtod(p, &tail);
            if (tail == p || errno == ERANGE)
                return NULL;

            value = json_new(JSON_NUMBER);
            if (value == NULL)
                return NULL;

            value->as.number = number;
            *cursor = tail;
            return value;
        }
    }
}

static void json_value_free(JsonValue *value)
{
    size_t index;

    if (value == NULL)
        return;

    switch (value->type) {
    case JSON_STRING:
        free(value->as.string);
        break;
    case JSON_ARRAY:
        for (index = 0; index < value->as.array.count; ++index)
            json_value_free(value->as.array.items[index]);
        free(value->as.array.items);
        break;
    case JSON_OBJECT:
        for (index = 0; index < value->as.object.count; ++index) {
            free(value->as.object.pairs[index].key);
            json_value_free(value->as.object.pairs[index].value);
        }
        free(value->as.object.pairs);
        break;
    case JSON_NULL:
    case JSON_BOOL:
    case JSON_NUMBER:
        break;
    }

    free(value);
}

static const JsonValue *json_object_get(const JsonValue *object, const char *key)
{
    size_t index;

    if (object == NULL || object->type != JSON_OBJECT || key == NULL)
        return NULL;

    for (index = 0; index < object->as.object.count; ++index) {
        if (strcmp(object->as.object.pairs[index].key, key) == 0)
            return object->as.object.pairs[index].value;
    }

    return NULL;
}

static int json_get_bool(const JsonValue *object, const char *key, int default_value)
{
    const JsonValue *value = json_object_get(object, key);

    if (value == NULL)
        return default_value;

    if (value->type == JSON_BOOL)
        return value->as.boolean;

    return default_value;
}

static double json_get_number(const JsonValue *object, const char *key, double default_value)
{
    const JsonValue *value = json_object_get(object, key);

    if (value == NULL)
        return default_value;

    if (value->type == JSON_NUMBER)
        return value->as.number;

    return default_value;
}

static char *json_get_string_dup(const JsonValue *object, const char *key)
{
    const JsonValue *value = json_object_get(object, key);

    if (value == NULL || value->type != JSON_STRING)
        return NULL;

    return sb3_strdup_local(value->as.string);
}

static void sb3_block_free(SB3Block *block)
{
    if (block == NULL)
        return;

    free(block->block_id);
    free(block->opcode);
    free(block->next);
    free(block->parent);
    sb3_block_init(block);
}

static void sb3_target_clear(SB3Target *target)
{
    size_t index;

    if (target == NULL)
        return;

    free(target->name);

    for (index = 0; index < target->block_count; ++index)
        sb3_block_free(&target->blocks[index]);

    free(target->blocks);
    sb3_target_init(target);
}

static void sb3_project_clear_assets(SB3Project *project)
{
    size_t index;

    if (project == NULL)
        return;

    for (index = 0; index < project->asset_count; ++index)
        sb3_asset_free(&project->assets[index]);

    free(project->assets);
    project->assets = NULL;
    project->asset_count = 0;
}

static void sb3_project_clear_targets(SB3Project *project)
{
    size_t index;

    if (project == NULL)
        return;

    for (index = 0; index < project->target_count; ++index)
        sb3_target_clear(&project->targets[index]);

    free(project->targets);
    project->targets = NULL;
    project->target_count = 0;
}

static void sb3_project_clear(SB3Project *project)
{
    if (project == NULL)
        return;

    sb3_project_clear_assets(project);
    sb3_project_clear_targets(project);

    free(project->project_json);
    project->project_json = NULL;
    project->project_json_size = 0;
}

static int sb3_load_project_json(zip_t *archive, SB3Project *project)
{
    zip_stat_t stat;
    zip_file_t *file;
    zip_int64_t bytes_read;
    char *json;

    if (zip_stat(archive, "project.json", 0, &stat) < 0)
        return 0;

    json = malloc((size_t)stat.size + 1);
    if (json == NULL)
        return 0;

    file = zip_fopen(archive, "project.json", 0);
    if (file == NULL) {
        free(json);
        return 0;
    }

    bytes_read = zip_fread(file, json, (zip_uint64_t)stat.size);
    zip_fclose(file);

    if (bytes_read != (zip_int64_t)stat.size) {
        free(json);
        return 0;
    }

    json[stat.size] = '\0';
    project->project_json = json;
    project->project_json_size = (size_t)stat.size;
    return 1;
}

static char *sb3_dup_asset_id_from_name(const char *file_name)
{
    const char *dot;
    size_t length;
    char *asset_id;

    if (file_name == NULL)
        return NULL;

    dot = strrchr(file_name, '.');
    if (dot == NULL || dot == file_name)
        return sb3_strdup_local(file_name);

    length = (size_t)(dot - file_name);
    asset_id = malloc(length + 1);
    if (asset_id == NULL)
        return NULL;

    memcpy(asset_id, file_name, length);
    asset_id[length] = '\0';
    return asset_id;
}

static int sb3_load_assets(zip_t *archive, SB3Project *project)
{
    zip_int64_t entry_count;
    zip_uint64_t index;
    SB3Asset *assets;
    size_t asset_count = 0;

    entry_count = zip_get_num_entries(archive, 0);
    if (entry_count < 0)
        return 0;

    if (entry_count == 0)
        return 1;

    assets = calloc((size_t)entry_count, sizeof(*assets));
    if (assets == NULL)
        return 0;

    for (index = 0; index < (zip_uint64_t)entry_count; ++index) {
        zip_stat_t stat;
        zip_file_t *file;
        zip_int64_t bytes_read;
        char *name_copy;
        char *slash;
        char *asset_id;
        char *data_format;

        if (zip_stat_index(archive, index, 0, &stat) < 0)
            continue;

        if (stat.name == NULL || strcmp(stat.name, "project.json") == 0)
            continue;

        slash = strrchr(stat.name, '/');
        if (slash != NULL && slash[1] == '\0')
            continue;

        file = zip_fopen_index(archive, index, 0);
        if (file == NULL)
            continue;

        sb3_asset_init(&assets[asset_count]);
        if (stat.size > 0) {
            assets[asset_count].data = malloc((size_t)stat.size);
            if (assets[asset_count].data == NULL) {
                zip_fclose(file);
                continue;
            }
        }

        bytes_read = zip_fread(file, assets[asset_count].data, (zip_uint64_t)stat.size);
        zip_fclose(file);
        if (bytes_read != (zip_int64_t)stat.size) {
            free(assets[asset_count].data);
            assets[asset_count].data = NULL;
            continue;
        }

        name_copy = sb3_strdup_local(stat.name);
        if (name_copy == NULL) {
            free(assets[asset_count].data);
            assets[asset_count].data = NULL;
            continue;
        }

        asset_id = sb3_dup_asset_id_from_name(stat.name);
        data_format = NULL;
        if (stat.name != NULL) {
            const char *dot = strrchr(stat.name, '.');
            if (dot != NULL && dot[1] != '\0')
                data_format = sb3_strdup_local(dot + 1);
        }

        assets[asset_count].file_name = name_copy;
        assets[asset_count].md5ext = sb3_strdup_local(stat.name);
        assets[asset_count].asset_id = asset_id;
        assets[asset_count].data_format = data_format;
        assets[asset_count].data_size = (size_t)stat.size;
        ++asset_count;
    }

    project->assets = assets;
    project->asset_count = asset_count;
    return 1;
}

SB3BlockKind sb3_block_kind_from_opcode(const char *opcode)
{
    if (opcode == NULL)
        return SB3_BLOCK_KIND_UNKNOWN;

    if (strcmp(opcode, "event_whenflagclicked") == 0)
        return SB3_BLOCK_KIND_EVENT_WHEN_FLAG_CLICKED;

    if (strcmp(opcode, "motion_movesteps") == 0)
        return SB3_BLOCK_KIND_MOTION_MOVE_STEPS;

    if (strcmp(opcode, "motion_turnright") == 0)
        return SB3_BLOCK_KIND_MOTION_TURN_RIGHT;

    if (strcmp(opcode, "motion_turnleft") == 0)
        return SB3_BLOCK_KIND_MOTION_TURN_LEFT;

    return SB3_BLOCK_KIND_UNKNOWN;
}

static double sb3_read_steps_value(const JsonValue *block_value)
{
    const JsonValue *inputs;
    const JsonValue *steps;
    const JsonValue *literal;

    inputs = json_object_get(block_value, "inputs");
    if (inputs == NULL || inputs->type != JSON_OBJECT)
        return 0.0;

    steps = json_object_get(inputs, "STEPS");
    if (steps == NULL || steps->type != JSON_ARRAY || steps->as.array.count < 2)
        return 0.0;

    literal = steps->as.array.items[1];
    if (literal == NULL || literal->type != JSON_ARRAY || literal->as.array.count < 2)
        return 0.0;

    literal = literal->as.array.items[1];
    if (literal == NULL)
        return 0.0;

    if (literal->type == JSON_NUMBER)
        return literal->as.number;

    if (literal->type == JSON_STRING)
        return strtod(literal->as.string, NULL);

    return 0.0;
}
static double sb3_read_turn_degrees_value(const JsonValue *block_value)
{
    const JsonValue *inputs;
    const JsonValue *degrees;
    const JsonValue *literal;

    inputs = json_object_get(block_value, "inputs");

    if (inputs == NULL || inputs->type != JSON_OBJECT)
        return 0.0;

    degrees = json_object_get(inputs, "DEGREES");

    if (degrees == NULL ||
        degrees->type != JSON_ARRAY ||
        degrees->as.array.count < 2)
        return 0.0;

    literal = degrees->as.array.items[1];

    if (literal == NULL ||
        literal->type != JSON_ARRAY ||
        literal->as.array.count < 2)
        return 0.0;

    literal = literal->as.array.items[1];

    if (literal == NULL)
        return 0.0;

    if (literal->type == JSON_NUMBER)
        return literal->as.number;

    if (literal->type == JSON_STRING)
        return strtod(literal->as.string, NULL);

    return 0.0;
}

static int sb3_block_from_json(const char *block_id, const JsonValue *block_value, SB3Block *block)
{
    const char *opcode;

    if (block_id == NULL || block_value == NULL || block_value->type != JSON_OBJECT || block == NULL)
        return 0;

    sb3_block_init(block);
    block->block_id = sb3_strdup_local(block_id);
    block->opcode = json_get_string_dup(block_value, "opcode");
    block->next = json_get_string_dup(block_value, "next");
    block->parent = json_get_string_dup(block_value, "parent");
    block->shadow = json_get_bool(block_value, "shadow", 0);
    block->top_level = json_get_bool(block_value, "topLevel", 0);
    block->x = json_get_number(block_value, "x", 0.0);
    block->y = json_get_number(block_value, "y", 0.0);

    if (block->block_id == NULL)
        return 0;

    opcode = block->opcode;
    block->kind = sb3_block_kind_from_opcode(opcode);
    switch (block->kind) {
        case SB3_BLOCK_KIND_MOTION_MOVE_STEPS:
        block->motion_steps = sb3_read_steps_value(block_value);
        block->has_motion_steps = 1;
    break;
    case SB3_BLOCK_KIND_MOTION_TURN_RIGHT:
    block->motion_turn_degrees = 
    sb3_read_turn_degrees_value(block_value);
    block->has_motion_turn_degrees = 1;
    break;
    case SB3_BLOCK_KIND_MOTION_TURN_LEFT:
    block->motion_turn_degrees = 
    sb3_read_turn_degrees_value(block_value);
    block->has_motion_turn_degrees = -1;
    break;
    case SB3_BLOCK_KIND_UNKNOWN:
    case SB3_BLOCK_KIND_EVENT_WHEN_FLAG_CLICKED:
    default:
        break;
}
    return 1;
}

static int sb3_parse_target(const JsonValue *target_value, SB3Target *target)
{
    const JsonValue *blocks;

    if (target_value == NULL || target_value->type != JSON_OBJECT || target == NULL)
        return 0;

    sb3_target_init(target);
    target->name = json_get_string_dup(target_value, "name");
    target->is_stage = json_get_bool(target_value, "isStage", 0);
    target->x = json_get_number(target_value, "x", 0.0);
    target->y = json_get_number(target_value, "y", 0.0);
    target->direction = json_get_number(target_value, "direction", 90.0);
    target->size = json_get_number(target_value, "size", 100.0);
    target->visible =   json_get_bool(target_value, "visible", 1);

    blocks = json_object_get(target_value, "blocks");
    if (blocks != NULL && blocks->type == JSON_OBJECT && blocks->as.object.count > 0) {
        size_t index;
        SB3Block *items;

        items = sb3_calloc(blocks->as.object.count, sizeof(*items));
        if (items == NULL)
            return 0;

        for (index = 0; index < blocks->as.object.count; ++index) {
            const JsonPair *pair = &blocks->as.object.pairs[index];

            if (!sb3_block_from_json(pair->key, pair->value, &items[target->block_count])) {
                size_t rollback;

                for (rollback = 0; rollback < target->block_count; ++rollback)
                    sb3_block_free(&items[rollback]);

                free(items);
                return 0;
            }

            ++target->block_count;
        }

        target->blocks = items;
    }

    return 1;
}

static int sb3_convert_project_json(SB3Project *project, const char *json, size_t json_size)
{
    const JsonValue *root;
    const JsonValue *targets;
    const char *cursor;
    const char *end;
    size_t index;

    if (project == NULL || json == NULL || json_size == 0)
        return 0;

    cursor = json;
    end = json + json_size;
    root = json_parse_value(&cursor, end);
    if (root == NULL || root->type != JSON_OBJECT) {
        json_value_free((JsonValue *)root);
        return 0;
    }

    cursor = json_skip_ws(cursor, end);
    if (cursor != end) {
        json_value_free((JsonValue *)root);
        return 0;
    }

    sb3_project_clear_targets(project);

    targets = json_object_get(root, "targets");
    if (targets == NULL || targets->type != JSON_ARRAY) {
        json_value_free((JsonValue *)root);
        return 0;
    }

    if (targets->as.array.count > 0) {
        project->targets = sb3_calloc(targets->as.array.count, sizeof(*project->targets));
        if (project->targets == NULL) {
            json_value_free((JsonValue *)root);
            return 0;
        }
    }

    for (index = 0; index < targets->as.array.count; ++index) {
        if (!sb3_parse_target(targets->as.array.items[index], &project->targets[project->target_count])) {
            sb3_project_clear_targets(project);
            json_value_free((JsonValue *)root);
            return 0;
        }

        ++project->target_count;
    }

    json_value_free((JsonValue *)root);
    return 1;
}

SB3Project *sb3_load(const char *path)
{
    int error = 0;
    zip_t *archive;
    SB3Project *project;

    if (path == NULL)
        return NULL;

    archive = zip_open(path, ZIP_RDONLY, &error);
    if (archive == NULL)
        return NULL;

    project = malloc(sizeof(*project));
    if (project == NULL) {
        zip_close(archive);
        return NULL;
    }

    sb3_project_init(project);

    if (!sb3_load_project_json(archive, project) ||
        !sb3_load_assets(archive, project) ||
        !sb3_convert_project_json(project, project->project_json, project->project_json_size)) {
        sb3_project_clear(project);
        free(project);
        zip_close(archive);
        return NULL;
    }

    zip_close(archive);
    return project;
}

void sb3_project_free(SB3Project *project)
{
    if (project == NULL)
        return;

    sb3_project_clear(project);
    free(project);
}

void sb3_destroy(SB3Project *project)
{
    sb3_project_free(project);
}

const SB3Block *sb3_target_find_block(const SB3Target *target, const char *block_id)
{
    size_t index;

    if (target == NULL || block_id == NULL)
        return NULL;

    for (index = 0; index < target->block_count; ++index) {
        const SB3Block *block = &target->blocks[index];

        if (block->block_id != NULL && strcmp(block->block_id, block_id) == 0)
            return block;
    }

    return NULL;
}

const SB3Block *sb3_target_find_block_by_kind(const SB3Target *target, SB3BlockKind kind)
{
    size_t index;

    if (target == NULL)
        return NULL;

    for (index = 0; index < target->block_count; ++index) {
        const SB3Block *block = &target->blocks[index];

        if (block->kind == kind)
            return block;
    }

    return NULL;
}
