#include "asset.h"

#include <stdlib.h>
#include <string.h>

#include "sb3.h"

void sb3_asset_init(SB3Asset *asset)
{
    if (asset == NULL)
        return;

    asset->asset_id = NULL;
    asset->md5ext = NULL;
    asset->file_name = NULL;
    asset->data_format = NULL;
    asset->data = NULL;
    asset->data_size = 0;
}

void sb3_asset_free(SB3Asset *asset)
{
    if (asset == NULL)
        return;

    free(asset->asset_id);
    free(asset->md5ext);
    free(asset->file_name);
    free(asset->data_format);
    free(asset->data);
    sb3_asset_init(asset);
}

const SB3Asset *sb3_asset_find(const SB3Project *project, const char *asset_id)
{
    size_t index;

    if (project == NULL || asset_id == NULL)
        return NULL;

    for (index = 0; index < project->asset_count; ++index) {
        const SB3Asset *asset = &project->assets[index];

        if ((asset->asset_id != NULL && strcmp(asset->asset_id, asset_id) == 0) ||
            (asset->md5ext != NULL && strcmp(asset->md5ext, asset_id) == 0) ||
            (asset->file_name != NULL && strcmp(asset->file_name, asset_id) == 0)) {
            return asset;
        }
    }

    return NULL;
}
