#include "group.h"

#include "assets/assets.h"
#include "core/log.h"

#include <stdlib.h>
#include <string.h>

static struct {
    int total_groups;
    int groups_in_memory;
    image_groups *groups;
} data;

int group_create_all(int total)
{
    total += 1; // Create extra group for external files
    for (int i = 0; i < data.total_groups; i++) {
        free((char *)data.groups[i].name);
    }
    if (data.groups_in_memory < total) {
        free(data.groups);
        data.groups_in_memory = 0;
        data.groups = malloc(sizeof(image_groups) * total);
        if (!data.groups) {
            return 0;
        }
        data.groups_in_memory = total;
    }
    memset(data.groups, 0, sizeof(image_groups) * total);
    data.total_groups = 0;
    return 1;
}

image_groups *group_get_new(void)
{
    return &data.groups[data.total_groups++];
}

image_groups *group_get_current(void)
{
    if (data.total_groups == 0) {
        return 0;
    }
    return &data.groups[data.total_groups - 1];
}

void group_set_for_external_files(void)
{
    image_groups *external_files_group = group_get_new();
    char *name = malloc(sizeof(ASSET_EXTERNAL_FILE_LIST));
    if (!name) {
        log_error("Failed to allocate memory for external files group name. The game will now crash.", 0, 0);
        return;
    }
    memcpy(name, ASSET_EXTERNAL_FILE_LIST, sizeof(ASSET_EXTERNAL_FILE_LIST));
    external_files_group->name = name;
    external_files_group->first_image_index = -1;
    external_files_group->last_image_index = -1;
}

void group_unload_current(void)
{
    image_groups *group = group_get_current();
    asset_image *img = asset_image_get_from_id(group->last_image_index);
    while (img && img->index >= group->first_image_index) {
        asset_image_unload(img);
        img = asset_image_get_from_id(img->index - 1);
    }
    free((char *) group->name);
#ifdef BUILDING_ASSET_PACKER
    free((char *) group->path);
#endif
    memset(group, 0, sizeof(image_groups));
    data.total_groups--;
}

image_groups *group_get_from_id(int id)
{
    return &data.groups[id];
}

int group_get_total(void)
{
    return data.total_groups;
}

image_groups *group_get_from_name(const char *name)
{
    if (!name || !*name) {
        return 0;
    }
    for (int i = 0; i < data.total_groups; i++) {
        image_groups *current = &data.groups[i];
        if (strcmp(current->name, name) == 0) {
            return current;
        }
    }
    return 0;
}

image_groups *group_get_from_image_index(int index)
{
    for (int i = 0; i < data.total_groups; i++) {
        image_groups *current = &data.groups[i];
        if (index >= current->first_image_index && index <= current->last_image_index) {
            return current;
        }
    }
    return 0;
}
