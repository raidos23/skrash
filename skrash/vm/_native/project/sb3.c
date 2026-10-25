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

static void sb3_input_init(SB3Input *in)
{
    if (in == NULL)
        return;

    in->kind = SB3_INPUT_KIND_NONE;
    in->literal_number = 0.0;
    in->literal_string = NULL;
    in->block_id = NULL;
}

static void sb3_input_free(SB3Input *in)
{
    if (in == NULL)
        return;

    free(in->literal_string);
    free(in->block_id);
    sb3_input_init(in);
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
    block ->repeat_times = 0.0;
    block ->has_repeat_times = 0;
    block ->substack = NULL;
    block ->message = NULL;
    block->motion_x = 0.0;
    block->motion_y = 0.0;
    block->has_motion_gotoxy = 0;
    block->substack_else = NULL;
    block->variable_name = NULL;
    block->variable_id = NULL;
    block->list_name = NULL;
    block->list_id = NULL;
    block->broadcast_name = NULL;
    block->event_option = NULL;
    sb3_input_init(&block->input_steps);
    sb3_input_init(&block->input_degrees);
    sb3_input_init(&block->input_times);
    sb3_input_init(&block->input_duration);
    sb3_input_init(&block->input_x);
    sb3_input_init(&block->input_y);
    sb3_input_init(&block->input_direction);
    sb3_input_init(&block->input_size);
    sb3_input_init(&block->input_change);
    sb3_input_init(&block->input_message);
    sb3_input_init(&block->input_num1);
    sb3_input_init(&block->input_num2);
    sb3_input_init(&block->input_condition);

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
    target->variables = NULL;
    target->variable_count = 0;
    target->lists = NULL;
    target->list_count = 0;
    target->costumes = NULL;
    target->costume_count = 0;
    target->current_costume = 0;
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

static char *sb3_read_field_array0(const JsonValue *block_value, const char *name)
{
    const JsonValue *fields = json_object_get(block_value, "fields");
    const JsonValue *f = fields ? json_object_get(fields, name) : NULL;
    if (!f || f->type != JSON_ARRAY || f->as.array.count < 1) return NULL;
    const JsonValue *val = f->as.array.items[0];
    if (val && val->type == JSON_STRING) return sb3_strdup_local(val->as.string);
    return NULL;
}

static char *sb3_read_field_string(const JsonValue *block_value, const char *name)
{
    const JsonValue *fields = json_object_get(block_value, "fields");
    const JsonValue *field = fields ? json_object_get(fields, name) : NULL;
    const JsonValue *value;

    if (field == NULL || field->type != JSON_ARRAY || field->as.array.count < 1)
        return NULL;

    value = field->as.array.items[0];
    if (value != NULL && value->type == JSON_STRING)
        return sb3_strdup_local(value->as.string);

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
    free(block->substack);
    free(block->message);
    free(block->substack_else);
    free(block->variable_name);
    free(block->variable_id);
    free(block->list_name);
    free(block->list_id);
    free(block->broadcast_name);
    free(block->event_option);
    sb3_input_free(&block->input_steps);
    sb3_input_free(&block->input_degrees);
    sb3_input_free(&block->input_times);
    sb3_input_free(&block->input_duration);
    sb3_input_free(&block->input_x);
    sb3_input_free(&block->input_y);
    sb3_input_free(&block->input_direction);
    sb3_input_free(&block->input_size);
    sb3_input_free(&block->input_change);
    sb3_input_free(&block->input_message);
    sb3_input_free(&block->input_num1);
    sb3_input_free(&block->input_num2);
    sb3_input_free(&block->input_condition);
    sb3_block_init(block);
}

static void sb3_target_clear(SB3Target *target)
{
    size_t index;

    if (target == NULL)
        return;

    for (index = 0; index < target->variable_count; ++index) {
    free(target->variables[index].id);
    free(target->variables[index].name);
    }
    free(target->variables);
    for (index = 0; index < target->list_count; ++index) {
    free(target->lists[index].id);
    free(target->lists[index].name);
    for (size_t j = 0; j < target->lists[index].item_count; ++j)
        free(target->lists[index].items[j]);
    free(target->lists[index].items);  
    }
    free(target->lists);

    free(target->name);

    for (index = 0; index < target->block_count; ++index)
        sb3_block_free(&target->blocks[index]);

    for (index = 0; index < target->costume_count; ++index) {
    free(target->costumes[index].name);
    free(target->costumes[index].asset_id);
    free(target->costumes[index].md5ext);
    free(target->costumes[index].data_format);
    }
    free(target->costumes);

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

    if (strcmp(opcode, "control_repeat") == 0)
        return SB3_BLOCK_KIND_CONTROL_REPEAT;
    
    if (strcmp(opcode, "looks_say") == 0)
        return SB3_BLOCK_KIND_LOOKS_SAY;

    if (strcmp(opcode, "control_forever") == 0)
       return SB3_BLOCK_KIND_CONTROL_FOREVER;

    if (strcmp(opcode, "motion_gotoxy") == 0)
       return SB3_BLOCK_KIND_MOTION_GOTOXY;
    
    if (strcmp(opcode, "motion_setx") == 0)
       return SB3_BLOCK_KIND_MOTION_SETX;
    
    if (strcmp(opcode, "motion_sety") == 0)
       return SB3_BLOCK_KIND_MOTION_SETY;

    if (strcmp(opcode, "control_wait") == 0)
        return SB3_BLOCK_KIND_CONTROL_WAIT;

    if (strcmp(opcode, "motion_pointindirection") == 0)  
        return SB3_BLOCK_KIND_MOTION_POINTINDIRECTION;  
    
    if (strcmp(opcode, "motion_changexby") == 0)  
        return SB3_BLOCK_KIND_MOTION_CHANGEXBY; 

    if (strcmp(opcode, "motion_changeyby") == 0)  
        return SB3_BLOCK_KIND_MOTION_CHANGEYBY;  

    if (strcmp(opcode, "looks_show") == 0)  
    return SB3_BLOCK_KIND_LOOKS_SHOW;  
    
    if (strcmp(opcode, "looks_hide") == 0)  
    return SB3_BLOCK_KIND_LOOKS_HIDE;  
    
    if (strcmp(opcode, "looks_setsizeto") == 0)  
    return SB3_BLOCK_KIND_LOOKS_SETSIZETO;  

    if (strcmp(opcode, "looks_changesizeby") == 0)  
    return SB3_BLOCK_KIND_LOOKS_CHANGESIZEBY;  
    
    if (strcmp(opcode, "looks_think") == 0)  
    return SB3_BLOCK_KIND_LOOKS_THINK;  

    if (strcmp(opcode, "operator_add")      == 0) return SB3_BLOCK_KIND_OPERATOR_ADD;
    if (strcmp(opcode, "operator_subtract") == 0) return SB3_BLOCK_KIND_OPERATOR_SUBTRACT;
    if (strcmp(opcode, "operator_multiply") == 0) return SB3_BLOCK_KIND_OPERATOR_MULTIPLY;
    if (strcmp(opcode, "operator_divide")   == 0) return SB3_BLOCK_KIND_OPERATOR_DIVIDE;
    if (strcmp(opcode, "operator_mod")      == 0) return SB3_BLOCK_KIND_OPERATOR_MOD;
    if (strcmp(opcode, "operator_round")    == 0) return SB3_BLOCK_KIND_OPERATOR_ROUND;
    if (strcmp(opcode, "operator_random")   == 0) return SB3_BLOCK_KIND_OPERATOR_RANDOM;
    if (strcmp(opcode, "operator_lt")       == 0) return SB3_BLOCK_KIND_OPERATOR_LT;
    if (strcmp(opcode, "operator_gt")       == 0) return SB3_BLOCK_KIND_OPERATOR_GT;
    if (strcmp(opcode, "operator_equals")   == 0) return SB3_BLOCK_KIND_OPERATOR_EQUALS;
    if (strcmp(opcode, "operator_and")      == 0) return SB3_BLOCK_KIND_OPERATOR_AND;
    if (strcmp(opcode, "operator_or")       == 0) return SB3_BLOCK_KIND_OPERATOR_OR;
    if (strcmp(opcode, "operator_not")      == 0) return SB3_BLOCK_KIND_OPERATOR_NOT;
    if (strcmp(opcode, "control_if")      == 0) return SB3_BLOCK_KIND_CONTROL_IF;
    if (strcmp(opcode, "control_if_else") == 0) return SB3_BLOCK_KIND_CONTROL_IF_ELSE;
    if (strcmp(opcode, "motion_xposition") == 0) return SB3_BLOCK_KIND_MOTION_XPOSITION;
    if (strcmp(opcode, "motion_yposition") == 0) return SB3_BLOCK_KIND_MOTION_YPOSITION;
    if (strcmp(opcode, "motion_direction") == 0) return SB3_BLOCK_KIND_MOTION_DIRECTION;
    if (strcmp(opcode, "operator_join")     == 0) return SB3_BLOCK_KIND_OPERATOR_JOIN;
    if (strcmp(opcode, "operator_length")   == 0) return SB3_BLOCK_KIND_OPERATOR_LENGTH;
    if (strcmp(opcode, "operator_contains") == 0) return SB3_BLOCK_KIND_OPERATOR_CONTAINS;
    if (strcmp(opcode, "operator_letter_of") == 0) return SB3_BLOCK_KIND_OPERATOR_LETTER_OF;
    if (strcmp(opcode, "operator_mathop")    == 0) return SB3_BLOCK_KIND_OPERATOR_MATHOP;
    if (strcmp(opcode, "data_setvariableto")    == 0) return SB3_BLOCK_KIND_DATA_SETVARIABLETO;
    if (strcmp(opcode, "data_changevariableby") == 0) return SB3_BLOCK_KIND_DATA_CHANGEVARIABLEBY;
    if (strcmp(opcode, "data_showvariable")     == 0) return SB3_BLOCK_KIND_DATA_SHOWVARIABLE;
    if (strcmp(opcode, "data_hidevariable")     == 0) return SB3_BLOCK_KIND_DATA_HIDEVARIABLE;
    if (strcmp(opcode, "data_variable")         == 0) return SB3_BLOCK_KIND_DATA_VARIABLE;

    if (strcmp(opcode, "data_addtolist")          == 0) return SB3_BLOCK_KIND_DATA_ADDTOLIST;
    if (strcmp(opcode, "data_deleteoflist")       == 0) return SB3_BLOCK_KIND_DATA_DELETEOFLIST;
    if (strcmp(opcode, "data_deletealloflist")    == 0) return SB3_BLOCK_KIND_DATA_DELETEALLOFLIST;
    if (strcmp(opcode, "data_insertatlist")       == 0) return SB3_BLOCK_KIND_DATA_INSERTATLIST;
    if (strcmp(opcode, "data_replaceitemoflist")  == 0) return SB3_BLOCK_KIND_DATA_REPLACEITEMOFLIST;
    if (strcmp(opcode, "data_itemoflist")         == 0) return SB3_BLOCK_KIND_DATA_ITEMOFLIST;
    if (strcmp(opcode, "data_itemnumoflist")      == 0) return SB3_BLOCK_KIND_DATA_ITEMNUMOFLIST;
    if (strcmp(opcode, "data_lengthoflist")       == 0) return SB3_BLOCK_KIND_DATA_LENGTHOFLIST;
    if (strcmp(opcode, "data_listcontainsitem")   == 0) return SB3_BLOCK_KIND_DATA_LISTCONTAINSITEM;
    if (strcmp(opcode, "data_showlist")           == 0) return SB3_BLOCK_KIND_DATA_SHOWLIST;
    if (strcmp(opcode, "data_hidelist")           == 0) return SB3_BLOCK_KIND_DATA_HIDELIST;
    if (strcmp(opcode, "data_listcontents")       == 0) return SB3_BLOCK_KIND_DATA_LISTCONTENTS;

    if (strcmp(opcode, "looks_nextcostume")     == 0) return SB3_BLOCK_KIND_LOOKS_NEXTCOSTUME;
    if (strcmp(opcode, "sensing_keypressed") == 0) return SB3_BLOCK_KIND_SENSING_KEYPRESSED;
    if (strcmp(opcode, "sensing_keyoptions") == 0) return SB3_BLOCK_KIND_SENSING_KEYOPTIONS;
    if (strcmp(opcode, "sensing_mousedown")  == 0) return SB3_BLOCK_KIND_SENSING_MOUSEDOWN;
    if (strcmp(opcode, "sensing_mousex")     == 0) return SB3_BLOCK_KIND_SENSING_MOUSEX;
    if (strcmp(opcode, "sensing_mousey")     == 0) return SB3_BLOCK_KIND_SENSING_MOUSEY;
    if (strcmp(opcode, "event_whenkeypressed") == 0) return SB3_BLOCK_KIND_EVENT_WHENKEYPRESSED;
    if (strcmp(opcode, "event_broadcast") == 0) return SB3_BLOCK_KIND_EVENT_BROADCAST;
    if (strcmp(opcode, "event_whenbroadcastreceived") == 0) return SB3_BLOCK_KIND_EVENT_WHENBROADCASTRECEIVED;
    if (strcmp(opcode, "event_broadcastandwait")       == 0) return SB3_BLOCK_KIND_EVENT_BROADCASTANDWAIT;
    if (strcmp(opcode, "event_whenbroadcastreceived")  == 0) return SB3_BLOCK_KIND_EVENT_WHENBROADCASTRECEIVED;
    if (strcmp(opcode, "event_whenthisspriteclicked")  == 0) return SB3_BLOCK_KIND_EVENT_WHENTHISSPRITECLICKED;
    if (strcmp(opcode, "event_whenstageclicked")       == 0) return SB3_BLOCK_KIND_EVENT_WHENSTAGECLICKED;
    if (strcmp(opcode, "event_whengreaterthan")        == 0) return SB3_BLOCK_KIND_EVENT_WHENGREATERTHAN;
    return SB3_BLOCK_KIND_UNKNOWN;
}

static double sb3_shadow_number(const JsonValue *shadow)
{
    if (shadow == NULL ||
        shadow->type != JSON_ARRAY ||
        shadow->as.array.count < 2)
        return 0.0;

    const JsonValue *v = shadow->as.array.items[1];
    if (v == NULL) return 0.0;
    if (v->type == JSON_NUMBER) return v->as.number;
    if (v->type == JSON_STRING) return strtod(v->as.string, NULL);
    return 0.0;
}

static int sb3_parse_input(const JsonValue *input, SB3Input *out)
{
    if (input == NULL ||
        input->type != JSON_ARRAY ||
        input->as.array.count < 2 ||
        out == NULL)
        return 0;

    const JsonValue *tag  = input->as.array.items[0];
    const JsonValue *body = input->as.array.items[1];

    if (tag == NULL || tag->type != JSON_NUMBER) return 0;

    int kind = (int)tag->as.number;

    if (kind == 1) {
    out->kind = SB3_INPUT_KIND_LITERAL;
    out->literal_number = sb3_shadow_number(body);

    /* Cas [1, [10, "poire"]] : lire la string aussi */
    if (body != NULL && body->type == JSON_ARRAY &&
        body->as.array.count >= 2) {
        const JsonValue *inner = body->as.array.items[1];
        if (inner != NULL && inner->type == JSON_STRING)
            out->literal_string = sb3_strdup_local(inner->as.string);
    }
    return 1;
}

    if (kind == 2) {
        if (body == NULL || body->type != JSON_STRING) return 0;
        out->kind = SB3_INPUT_KIND_BLOCK_ONLY;
        out->block_id = sb3_strdup_local(body->as.string);
        return out->block_id != NULL;
    }
    
    if (kind == 3) {
    if (body == NULL) return 0;

    /* Cas 1 : body est une string = block_id (format classique) */
    if (body->type == JSON_STRING) {
        out->kind = SB3_INPUT_KIND_BLOCK;
        out->block_id = sb3_strdup_local(body->as.string);
        if (out->block_id == NULL) return 0;

        if (input->as.array.count >= 3) {
            const JsonValue *shadow = input->as.array.items[2];
            out->literal_number = sb3_shadow_number(shadow);
            if (shadow != NULL && shadow->type == JSON_ARRAY &&
                shadow->as.array.count >= 2) {
                const JsonValue *inner = shadow->as.array.items[1];
                if (inner != NULL && inner->type == JSON_STRING)
                    out->literal_string = sb3_strdup_local(inner->as.string);
            }
        }
        return 1;
    }

    /* Cas 2 : body est un tableau [12, "nom", "id"] = variable directe */
    if (body->type == JSON_ARRAY && body->as.array.count >= 3) {
        const JsonValue *tag = body->as.array.items[0];
        if (tag == NULL || tag->type != JSON_NUMBER ||
            (int)tag->as.number != 12)
            return 0;

        const JsonValue *name_j = body->as.array.items[1];
        const JsonValue *id_j   = body->as.array.items[2];
        if (name_j == NULL || name_j->type != JSON_STRING)
            return 0;

        out->kind = SB3_INPUT_KIND_LITERAL;
        out->literal_string = sb3_strdup_local(name_j->as.string);
        if (id_j != NULL && id_j->type == JSON_STRING)
            out->block_id = sb3_strdup_local(id_j->as.string);

        /* shadow de fallback */
        if (input->as.array.count >= 3) {
            const JsonValue *shadow = input->as.array.items[2];
            out->literal_number = sb3_shadow_number(shadow);
        }
        return 1;
    }
    }
    return 0;
}

static int sb3_parse_input_at(
    const JsonValue *block_value,
    const char *input_name,
    SB3Input *out)
{
    const JsonValue *inputs = json_object_get(block_value, "inputs");
    const JsonValue *in = inputs ? json_object_get(inputs, input_name) : NULL;
    return sb3_parse_input(in, out);
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

static double sb3_read_coordinate_value(
    const JsonValue *block_value,
    const char *input_name
)
{
    const JsonValue *inputs;
    const JsonValue *input;
    const JsonValue *literal;

    inputs = json_object_get(block_value, "inputs");

    if (inputs == NULL || inputs->type != JSON_OBJECT)
        return 0.0;

    input = json_object_get(inputs, input_name);

    if (input == NULL ||
        input->type != JSON_ARRAY ||
        input->as.array.count < 2)
        return 0.0;

    literal = input->as.array.items[1];

    if (literal == NULL)
        return 0.0;

    if (literal->type == JSON_ARRAY) {
        if (literal->as.array.count < 2)
            return 0.0;

        literal = literal->as.array.items[1];
    }

    if (literal == NULL)
        return 0.0;

    if (literal->type == JSON_NUMBER)
        return literal->as.number;

    if (literal->type == JSON_STRING) {
        char *end;
        double value;

        errno = 0;
        value = strtod(literal->as.string, &end);

        if (end == literal->as.string || *end != '\0' || errno == ERANGE)
            return 0.0;

        return value;
    }

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

static double sb3_read_repeat_times(const JsonValue *block_value)
{
    const JsonValue *inputs;
    const JsonValue *times;
    const JsonValue *value;

    inputs = json_object_get(block_value, "inputs");
    times = json_object_get(inputs, "TIMES");

    if (times == NULL ||
        times->type != JSON_ARRAY ||
        times->as.array.count < 2)
        return 0.0;

    value = times->as.array.items[1];

    if (value == NULL)
        return 0.0;

    if (value->type == JSON_NUMBER)
        return value->as.number;

    if (value->type == JSON_STRING)
        return strtod(value->as.string, NULL);

    if (value->type == JSON_ARRAY &&
        value->as.array.count >= 2) {
        value = value->as.array.items[1];

        if (value != NULL && value->type == JSON_NUMBER)
            return value->as.number;

        if (value != NULL && value->type == JSON_STRING)
            return strtod(value->as.string, NULL);
    }

    return 0.0;
}

static char *sb3_read_substack(const JsonValue *block_value)
{
    const JsonValue *inputs;
    const JsonValue *substack;
    const JsonValue *value;

    inputs = json_object_get(block_value, "inputs");
    substack = json_object_get(inputs, "SUBSTACK");

    if (substack == NULL ||
        substack->type != JSON_ARRAY ||
        substack->as.array.count < 2)
        return NULL;

    value = substack->as.array.items[1];

    if (value == NULL || value->type == JSON_NULL)
        return NULL;

    if (value->type == JSON_STRING)
        return sb3_strdup_local(value->as.string);

    if (value->type == JSON_ARRAY &&
        value->as.array.count >= 2) {
        value = value->as.array.items[1];

        if (value != NULL && value->type == JSON_STRING)
            return sb3_strdup_local(value->as.string);
    }

    return NULL;
}

static char *sb3_read_message(const JsonValue *block_value)
{
    const JsonValue *inputs;
    const JsonValue *message;
    const JsonValue *tag;
    const JsonValue *body;

    inputs = json_object_get(block_value, "inputs");
    if (inputs == NULL || inputs->type != JSON_OBJECT)
        return NULL;

    message = json_object_get(inputs, "MESSAGE");
    if (message == NULL || message->type != JSON_ARRAY || message->as.array.count < 2)
        return NULL;

    tag = message->as.array.items[0];
    body = message->as.array.items[1];

    if (tag == NULL || tag->type != JSON_NUMBER)
        return NULL;

    /* Seulement les littéraux (kind == 1) */
    if ((int)tag->as.number != 1)
        return NULL;

    if (body == NULL)
        return NULL;

    /* format [1, [10, "texte"]] ou [1, "texte"] */
    if (body->type == JSON_STRING)
        return sb3_strdup_local(body->as.string);

    if (body->type == JSON_ARRAY && body->as.array.count >= 2) {
        const JsonValue *inner = body->as.array.items[1];
        if (inner != NULL && inner->type == JSON_STRING)
            return sb3_strdup_local(inner->as.string);
    }

    return NULL;
}

static char *sb3_read_substack2(const JsonValue *block_value)
{
    const JsonValue *inputs = json_object_get(block_value, "inputs");
    const JsonValue *sub = json_object_get(inputs, "SUBSTACK2");
    const JsonValue *value;

    if (sub == NULL ||
        sub->type != JSON_ARRAY ||
        sub->as.array.count < 2)
        return NULL;

    value = sub->as.array.items[1];
    if (value == NULL || value->type == JSON_NULL)
        return NULL;

    if (value->type == JSON_STRING)
        return sb3_strdup_local(value->as.string);

    if (value->type == JSON_ARRAY &&
        value->as.array.count >= 2) {
        value = value->as.array.items[1];
        if (value != NULL && value->type == JSON_STRING)
            return sb3_strdup_local(value->as.string);
    }

    return NULL;
}

static void sb3_parse_variable_field(const JsonValue *block_value, SB3Block *block)
{
    const JsonValue *fields = json_object_get(block_value, "fields");
    const JsonValue *var    = fields ? json_object_get(fields, "VARIABLE") : NULL;

    if (var == NULL || var->type != JSON_ARRAY || var->as.array.count < 1)
        return;

    const JsonValue *name_j = var->as.array.items[0];
    if (name_j != NULL && name_j->type == JSON_STRING)
        block->variable_name = sb3_strdup_local(name_j->as.string);

    if (var->as.array.count >= 2) {
        const JsonValue *id_j = var->as.array.items[1];
        if (id_j != NULL && id_j->type == JSON_STRING)
            block->variable_id = sb3_strdup_local(id_j->as.string);
    }
}

static void sb3_parse_list_field(const JsonValue *block_value, SB3Block *block)
{
    const JsonValue *fields = json_object_get(block_value, "fields");
    const JsonValue *list   = fields ? json_object_get(fields, "LIST") : NULL;

    if (list == NULL || list->type != JSON_ARRAY || list->as.array.count < 1)
        return;

    const JsonValue *name_j = list->as.array.items[0];
    if (name_j != NULL && name_j->type == JSON_STRING)
        block->list_name = sb3_strdup_local(name_j->as.string);

    if (list->as.array.count >= 2) {
        const JsonValue *id_j = list->as.array.items[1];
        if (id_j != NULL && id_j->type == JSON_STRING)
            block->list_id = sb3_strdup_local(id_j->as.string);
    }
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
    sb3_parse_input_at(block_value, "STEPS", &block->input_steps);
    block->motion_steps = sb3_read_steps_value(block_value);
    block->has_motion_steps = 1;
    break;
    case SB3_BLOCK_KIND_MOTION_TURN_RIGHT:
    sb3_parse_input_at(block_value, "DEGREES", &block->input_degrees);
    block->motion_turn_degrees = sb3_read_turn_degrees_value(block_value);
    block->has_motion_turn_degrees = 1;
    break;
    case SB3_BLOCK_KIND_MOTION_TURN_LEFT:
    sb3_parse_input_at(block_value, "DEGREES", &block->input_degrees);
    block->motion_turn_degrees = sb3_read_turn_degrees_value(block_value);
    block->has_motion_turn_degrees = -1;
    break;
    case SB3_BLOCK_KIND_CONTROL_REPEAT:
    sb3_parse_input_at(block_value, "TIMES", &block->input_times);
    block->repeat_times = sb3_read_repeat_times(block_value);
    block->has_repeat_times = 1;
    block->substack = sb3_read_substack(block_value);
    break;
    case SB3_BLOCK_KIND_LOOKS_SAY:
    sb3_parse_input_at(block_value, "MESSAGE", &block->input_message);
    block->message = sb3_read_message(block_value);  /* maintenant safe */
    break;
    case SB3_BLOCK_KIND_CONTROL_FOREVER:
    block->substack = sb3_read_substack(block_value);
    break;
    case SB3_BLOCK_KIND_MOTION_GOTOXY:
    sb3_parse_input_at(block_value, "X", &block->input_x);
    sb3_parse_input_at(block_value, "Y", &block->input_y);
    block->motion_x = sb3_read_coordinate_value(block_value, "X");
    block->motion_y = sb3_read_coordinate_value(block_value, "Y");
    block->has_motion_gotoxy = 1;
    break;
    case SB3_BLOCK_KIND_MOTION_SETX:
    sb3_parse_input_at(block_value, "X", &block->input_x);
    block->motion_x = sb3_read_coordinate_value(block_value, "X");
    block->has_motion_setx = 1;
    break;
    case SB3_BLOCK_KIND_MOTION_SETY:
    sb3_parse_input_at(block_value, "Y", &block->input_y);
    block->motion_y = sb3_read_coordinate_value(block_value, "Y");
    block->has_motion_sety = 1;
    break;
    case SB3_BLOCK_KIND_CONTROL_WAIT:
    sb3_parse_input_at(block_value, "DURATION", &block->input_duration);
    block->wait_seconds = sb3_read_coordinate_value(block_value, "DURATION");
    block->has_wait_seconds = 1;
    break;
    case SB3_BLOCK_KIND_MOTION_POINTINDIRECTION:  
    sb3_parse_input_at(block_value, "DIRECTION", &block->input_direction);
    block->motion_direction = sb3_read_coordinate_value(block_value, "DIRECTION");  
    block->has_motion_pointindirection = 1;  
    break;  
    case SB3_BLOCK_KIND_MOTION_CHANGEXBY:
    sb3_parse_input_at(block_value, "DX", &block->input_x);  
    block->motion_x = sb3_read_coordinate_value(block_value, "DX");  
    block->has_motion_setx = 1;  
    break;  
    case SB3_BLOCK_KIND_MOTION_CHANGEYBY:
    sb3_parse_input_at(block_value, "DY", &block->input_y);
    block->motion_y = sb3_read_coordinate_value(block_value, "DY");
    block->has_motion_sety = 1;
    break;  
    case SB3_BLOCK_KIND_LOOKS_SETSIZETO:
    sb3_parse_input_at(block_value, "SIZE", &block->input_size);
    block->size_value = sb3_read_coordinate_value(block_value, "SIZE");
    block->has_size_value = 1;
    break;
    case SB3_BLOCK_KIND_LOOKS_CHANGESIZEBY:
    sb3_parse_input_at(block_value, "CHANGE", &block->input_change);
    block->size_value = sb3_read_coordinate_value(block_value, "CHANGE");
    block->has_size_value = 1;
    break;
    case SB3_BLOCK_KIND_LOOKS_THINK:
    sb3_parse_input_at(block_value, "MESSAGE", &block->input_message);
    block->message = sb3_read_message(block_value);
    break;
    case SB3_BLOCK_KIND_LOOKS_SHOW:
    // rien à lire
    break;
    case SB3_BLOCK_KIND_LOOKS_HIDE:  
     // rien à lire
    break; 

    case SB3_BLOCK_KIND_OPERATOR_ADD:
    case SB3_BLOCK_KIND_OPERATOR_SUBTRACT:
    case SB3_BLOCK_KIND_OPERATOR_MULTIPLY:
    case SB3_BLOCK_KIND_OPERATOR_DIVIDE:
    case SB3_BLOCK_KIND_OPERATOR_MOD:
    case SB3_BLOCK_KIND_OPERATOR_RANDOM:
    sb3_parse_input_at(block_value, "NUM1", &block->input_num1);
    sb3_parse_input_at(block_value, "NUM2", &block->input_num2);
    break;

    case SB3_BLOCK_KIND_OPERATOR_LT:
    case SB3_BLOCK_KIND_OPERATOR_GT:
    case SB3_BLOCK_KIND_OPERATOR_EQUALS:
    case SB3_BLOCK_KIND_OPERATOR_AND:
    case SB3_BLOCK_KIND_OPERATOR_OR:
    sb3_parse_input_at(block_value, "OPERAND1", &block->input_num1);
    sb3_parse_input_at(block_value, "OPERAND2", &block->input_num2);
    break;

    case SB3_BLOCK_KIND_OPERATOR_ROUND:
    sb3_parse_input_at(block_value, "NUM", &block->input_num1);
    break;

    case SB3_BLOCK_KIND_OPERATOR_NOT:
    sb3_parse_input_at(block_value, "OPERAND", &block->input_num1);
    break;

    case SB3_BLOCK_KIND_CONTROL_IF:
    sb3_parse_input_at(block_value, "CONDITION", &block->input_condition);
    block->substack = sb3_read_substack(block_value);
    break;

    case SB3_BLOCK_KIND_CONTROL_IF_ELSE:
    sb3_parse_input_at(block_value, "CONDITION", &block->input_condition);
    block->substack = sb3_read_substack(block_value);
    block->substack_else = sb3_read_substack2(block_value);
    break;
    case SB3_BLOCK_KIND_OPERATOR_JOIN:
    sb3_parse_input_at(block_value, "STRING1", &block->input_num1);
    sb3_parse_input_at(block_value, "STRING2", &block->input_num2);
    break;

    case SB3_BLOCK_KIND_OPERATOR_LENGTH:
    sb3_parse_input_at(block_value, "STRING", &block->input_num1);
    break;

    case SB3_BLOCK_KIND_OPERATOR_CONTAINS:
    sb3_parse_input_at(block_value, "STRING1", &block->input_num1);
    sb3_parse_input_at(block_value, "STRING2", &block->input_num2);
    break;
    case SB3_BLOCK_KIND_OPERATOR_LETTER_OF:
    sb3_parse_input_at(block_value, "LETTER", &block->input_num1);
    sb3_parse_input_at(block_value, "STRING", &block->input_num2);
    break;
    
    case SB3_BLOCK_KIND_OPERATOR_MATHOP:
    sb3_parse_input_at(block_value, "NUM", &block->input_num1);
    block->message = sb3_read_field_string(block_value, "OPERATOR");
    break;
    case SB3_BLOCK_KIND_DATA_SETVARIABLETO:
    sb3_parse_variable_field(block_value, block);
    sb3_parse_input_at(block_value, "VALUE", &block->input_num1);
    break;
    case SB3_BLOCK_KIND_DATA_CHANGEVARIABLEBY:
    sb3_parse_variable_field(block_value, block);
    sb3_parse_input_at(block_value, "VALUE", &block->input_num1);
    break;
    case SB3_BLOCK_KIND_DATA_VARIABLE:
    case SB3_BLOCK_KIND_DATA_SHOWVARIABLE:
    case SB3_BLOCK_KIND_DATA_HIDEVARIABLE:
    sb3_parse_variable_field(block_value, block);
    break;
    case SB3_BLOCK_KIND_DATA_ADDTOLIST:
    sb3_parse_list_field(block_value, block);
    sb3_parse_input_at(block_value, "ITEM", &block->input_num1);
    break;
    case SB3_BLOCK_KIND_DATA_DELETEOFLIST:
    sb3_parse_list_field(block_value, block);
    sb3_parse_input_at(block_value, "INDEX", &block->input_num1);
    break;
    case SB3_BLOCK_KIND_DATA_DELETEALLOFLIST:
    case SB3_BLOCK_KIND_DATA_LENGTHOFLIST:
    case SB3_BLOCK_KIND_DATA_SHOWLIST:
    case SB3_BLOCK_KIND_DATA_HIDELIST:
    case SB3_BLOCK_KIND_DATA_LISTCONTENTS:
        sb3_parse_list_field(block_value, block);
    break;
    case SB3_BLOCK_KIND_DATA_LISTCONTAINSITEM:  
    sb3_parse_list_field(block_value, block);
    sb3_parse_input_at(block_value, "ITEM", &block->input_num1);
    break;
    case SB3_BLOCK_KIND_DATA_INSERTATLIST:
    sb3_parse_list_field(block_value, block);
    sb3_parse_input_at(block_value, "ITEM",  &block->input_num1);
    sb3_parse_input_at(block_value, "INDEX", &block->input_num2);
    break;
    case SB3_BLOCK_KIND_DATA_REPLACEITEMOFLIST:
    sb3_parse_list_field(block_value, block);
    sb3_parse_input_at(block_value, "INDEX", &block->input_num1);
    sb3_parse_input_at(block_value, "ITEM",  &block->input_num2);
    break;
    case SB3_BLOCK_KIND_DATA_ITEMOFLIST:
    sb3_parse_list_field(block_value, block);
    sb3_parse_input_at(block_value, "INDEX", &block->input_num1);
    break;
    case SB3_BLOCK_KIND_DATA_ITEMNUMOFLIST:
    sb3_parse_list_field(block_value, block);
    sb3_parse_input_at(block_value, "ITEM", &block->input_num1);
    break;
    
    case SB3_BLOCK_KIND_LOOKS_NEXTCOSTUME:
    break; 

    case SB3_BLOCK_KIND_SENSING_KEYPRESSED: {
    /* 1. Chercher dans les fields (comme le fait Scratch) */
    const JsonValue *fields = json_object_get(block_value, "fields");
    const JsonValue *key_field = fields ? json_object_get(fields, "KEY_OPTION") : NULL;
    
    if (key_field && key_field->type == JSON_ARRAY && key_field->as.array.count >= 1) {
        const JsonValue *val = key_field->as.array.items[0];
        if (val && val->type == JSON_STRING) {
            block->message = sb3_strdup_local(val->as.string);
            break;
        }
    }

    /* 2. Si pas dans les fields, chercher dans les inputs (cas du shadow) */
    const JsonValue *inputs = json_object_get(block_value, "inputs");
    const JsonValue *key_in = inputs ? json_object_get(inputs, "KEY_OPTION") : NULL;
    
    if (key_in && key_in->type == JSON_ARRAY && key_in->as.array.count >= 2) {
        const JsonValue *body = key_in->as.array.items[1];
        
        if (body && body->type == JSON_ARRAY && body->as.array.count >= 2) {
            const JsonValue *inner = body->as.array.items[1];
            if (inner && inner->type == JSON_STRING)
                block->message = sb3_strdup_local(inner->as.string);
        } else if (body && body->type == JSON_STRING) {
            /* C'est l'ID du bloc sensing_keyoptions */
            block->input_num1.kind = SB3_INPUT_KIND_BLOCK;
            block->input_num1.block_id = sb3_strdup_local(body->as.string);
        }
    }
    break;
}

    case SB3_BLOCK_KIND_SENSING_KEYOPTIONS: {
    const JsonValue *fields = json_object_get(block_value, "fields");
    const JsonValue *key = fields ? json_object_get(fields, "KEY_OPTION") : NULL;
    if (key && key->type == JSON_ARRAY && key->as.array.count >= 1) {
        const JsonValue *val = key->as.array.items[0];
        if (val && val->type == JSON_STRING)
            block->message = sb3_strdup_local(val->as.string);
    }
    break;
}

    case SB3_BLOCK_KIND_EVENT_BROADCAST:
    case SB3_BLOCK_KIND_EVENT_BROADCASTANDWAIT: {
    /* 1. Essayer de lire dans les fields (comme pour le chapeau) */
    block->broadcast_name = sb3_read_field_array0(block_value, "BROADCAST_OPTION");
    
    /* 2. Si pas dans les fields, chercher dans les inputs (cas du bloc broadcast) */
    if (block->broadcast_name == NULL) {
        const JsonValue *inputs = json_object_get(block_value, "inputs");
        const JsonValue *b_in = inputs ? json_object_get(inputs, "BROADCAST_INPUT") : NULL;
        
        if (b_in && b_in->type == JSON_ARRAY && b_in->as.array.count >= 2) {
            const JsonValue *body = b_in->as.array.items[1];
            
            /* Le body est un tableau : [11, "dfghjkl", "ID"] */
            if (body && body->type == JSON_ARRAY && body->as.array.count >= 2) {
                const JsonValue *name_j = body->as.array.items[1];
                if (name_j && name_j->type == JSON_STRING) {
                    block->broadcast_name = sb3_strdup_local(name_j->as.string);
                }
            }
        }
    }
    break;
}

    case SB3_BLOCK_KIND_EVENT_WHENBROADCASTRECEIVED:
    block->broadcast_name = sb3_read_field_array0(block_value, "BROADCAST_OPTION");
    break;
    
    case SB3_BLOCK_KIND_EVENT_WHENKEYPRESSED:
    block->event_option = sb3_read_field_array0(block_value, "KEY_OPTION");
    break;
    case SB3_BLOCK_KIND_EVENT_WHENGREATERTHAN:
    block->event_option = sb3_read_field_array0(block_value, "WHENGREATERTHANMENU");
    sb3_parse_input_at(block_value, "VALUE", &block->input_num1);
    break;
    case SB3_BLOCK_KIND_EVENT_WHENTHISSPRITECLICKED:
    case SB3_BLOCK_KIND_EVENT_WHENSTAGECLICKED:
    break;
    case SB3_BLOCK_KIND_UNKNOWN:
    case SB3_BLOCK_KIND_EVENT_WHEN_FLAG_CLICKED:
    default:
        break;
}
    return 1;
}

static int sb3_parse_variables(const JsonValue *target_value, SB3Target *target)
{
    const JsonValue *vars = json_object_get(target_value, "variables");
    if (vars == NULL || vars->type != JSON_OBJECT || vars->as.object.count == 0)
        return 1;

    target->variables = sb3_calloc(vars->as.object.count,
                                    sizeof(*target->variables));
    if (target->variables == NULL)
        return 0;

    for (size_t i = 0; i < vars->as.object.count; ++i) {
        const JsonPair *pair = &vars->as.object.pairs[i];
        const JsonValue *val = pair->value;

        if (val == NULL || val->type != JSON_ARRAY || val->as.array.count < 2)
            continue;

        const JsonValue *name_j  = val->as.array.items[0];
        const JsonValue *value_j = val->as.array.items[1];

        if (name_j == NULL || name_j->type != JSON_STRING)
            continue;

        SB3Variable *v = &target->variables[target->variable_count];
        v->id   = sb3_strdup_local(pair->key);
        v->name = sb3_strdup_local(name_j->as.string);

        if (value_j != NULL) {
            if (value_j->type == JSON_NUMBER)
                v->value = value_j->as.number;
            else if (value_j->type == JSON_STRING)
                v->value = strtod(value_j->as.string, NULL);
        }

        target->variable_count++;
    }

    return 1;
}

static int sb3_parse_lists(const JsonValue *target_value, SB3Target *target)
{
    const JsonValue *lists = json_object_get(target_value, "lists");
    if (lists == NULL || lists->type != JSON_OBJECT || lists->as.object.count == 0)
        return 1;

    target->lists = sb3_calloc(lists->as.object.count, sizeof(*target->lists));
    if (target->lists == NULL)
        return 0;

    for (size_t i = 0; i < lists->as.object.count; ++i) {
        const JsonPair *pair = &lists->as.object.pairs[i];
        const JsonValue *val = pair->value;

        if (val == NULL || val->type != JSON_ARRAY || val->as.array.count < 2)
            continue;

        const JsonValue *name_j   = val->as.array.items[0];
        const JsonValue *items_j  = val->as.array.items[1];

        if (name_j == NULL || name_j->type != JSON_STRING)
            continue;

        SB3List *lst = &target->lists[target->list_count];
        lst->id   = sb3_strdup_local(pair->key);
        lst->name = sb3_strdup_local(name_j->as.string);
        lst->items = NULL;
        lst->item_count = 0;

        if (items_j != NULL && items_j->type == JSON_ARRAY &&
            items_j->as.array.count > 0) {
            lst->items = sb3_calloc(items_j->as.array.count,
                                     sizeof(char *));
            if (lst->items != NULL) {
                for (size_t j = 0; j < items_j->as.array.count; ++j) {
                    const JsonValue *item = items_j->as.array.items[j];
                    char buf[64];
                    if (item == NULL) continue;
                    if (item->type == JSON_STRING)
                        lst->items[lst->item_count++] =
                            sb3_strdup_local(item->as.string);
                    else if (item->type == JSON_NUMBER) {
                        snprintf(buf, sizeof(buf), "%g", item->as.number);
                        lst->items[lst->item_count++] =
                            sb3_strdup_local(buf);
                    }
                }
            }
        }

        target->list_count++;
    }

    return 1;
}

static int sb3_parse_costumes(const JsonValue *target_value, SB3Target *target)
{
    const JsonValue *arr = json_object_get(target_value, "costumes");
    if (arr == NULL || arr->type != JSON_ARRAY || arr->as.array.count == 0)
        return 1;

    target->costumes = sb3_calloc(arr->as.array.count,
                                   sizeof(*target->costumes));
    if (target->costumes == NULL) return 0;

    for (size_t i = 0; i < arr->as.array.count; ++i) {
        const JsonValue *c = arr->as.array.items[i];
        if (c == NULL || c->type != JSON_OBJECT) continue;

        SB3Costume *co = &target->costumes[target->costume_count];
        co->name        = json_get_string_dup(c, "name");
        co->asset_id    = json_get_string_dup(c, "assetId");
        co->md5ext      = json_get_string_dup(c, "md5ext");
        co->data_format = json_get_string_dup(c, "dataFormat");
        if (co->asset_id == NULL) {
            free(co->name); free(co->md5ext); free(co->data_format);
            continue;
        }
        target->costume_count++;
    }

    target->current_costume =
        (size_t)json_get_number(target_value, "currentCostume", 0);
    if (target->current_costume >= target->costume_count)
        target->current_costume = 0;

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
    if (!sb3_parse_variables(target_value, target))
    return 0;
    if (!sb3_parse_lists(target_value, target))
    return 0;
    if (!sb3_parse_costumes(target_value, target)) 
    return 0;

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
    if (target == NULL || block_id == NULL) return NULL;

    for (size_t i = 0; i < target->block_count; ++i) {
        const SB3Block *b = &target->blocks[i];
        if (b->block_id && strcmp(b->block_id, block_id) == 0) {
            return b;
        }
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


SB3Variable *sb3_project_find_variable(SB3Project *project,
                                        SB3Target *context,
                                        const char *name)
{
    if (project == NULL || name == NULL)
        return NULL;

    if (context != NULL) {
        for (size_t i = 0; i < context->variable_count; ++i) {
            if (context->variables[i].name &&
                strcmp(context->variables[i].name, name) == 0)
                return &context->variables[i];
        }
    }

    for (size_t i = 0; i < project->target_count; ++i) {
        SB3Target *t = &project->targets[i];
        if (!t->is_stage) continue;
        for (size_t j = 0; j < t->variable_count; ++j) {
            if (t->variables[j].name &&
                strcmp(t->variables[j].name, name) == 0)
                return &t->variables[j];
        }
    }

    return NULL;
}

SB3List *sb3_project_find_list(SB3Project *project,
                                SB3Target *context,
                                const char *name)
{
    if (project == NULL || name == NULL)
        return NULL;

    if (context != NULL) {
        for (size_t i = 0; i < context->list_count; ++i) {
            if (context->lists[i].name &&
                strcmp(context->lists[i].name, name) == 0)
                return &context->lists[i];
        }
    }

    for (size_t i = 0; i < project->target_count; ++i) {
        SB3Target *t = &project->targets[i];
        if (!t->is_stage) continue;
        for (size_t j = 0; j < t->list_count; ++j) {
            if (t->lists[j].name &&
                strcmp(t->lists[j].name, name) == 0)
                return &t->lists[j];
        }
    }

    return NULL;
}