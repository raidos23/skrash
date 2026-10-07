#ifndef SKRASH_ASSET_H
#define SKRASH_ASSET_H

#include <stddef.h>

typedef struct SB3Asset {
    char *asset_id;
    char *md5ext;
    char *file_name;
    char *data_format;

    unsigned char *data;
    size_t data_size;
} SB3Asset;

void sb3_asset_init(SB3Asset *asset);
void sb3_asset_free(SB3Asset *asset);

#endif