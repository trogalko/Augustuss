#include "assets.h"

#include "assets/group.h"
#include "assets/image.h"
#include "assets/xml.h"
#include "core/dir.h"
#include "core/log.h"
#include "graphics/renderer.h"
#include "core/png_read.h"

#include <stdlib.h>
#include <string.h>

static struct {
    int roadblock_image_id;
    asset_image *roadblock_image;
    int asset_lookup[ASSET_MAX_KEY];
} data;

void assets_init(int force_reload, color_t **main_images, int *main_image_widths)
{
    if (graphics_renderer()->has_image_atlas(ATLAS_EXTRA_ASSET) && !force_reload) {
        asset_image_reload_climate();
        return;
    }

    graphics_renderer()->free_image_atlas(ATLAS_EXTRA_ASSET);

    const dir_listing *xml_files = dir_find_files_with_extension(ASSETS_DIRECTORY "/" ASSETS_IMAGE_PATH, "xml");

    if (!group_create_all(xml_files->num_files) || !asset_image_init_array()) {
        log_error("Not enough memory to initialize extra assets. The game will probably crash.", 0, 0);
    }

    xml_init();

    for (int i = 0; i < xml_files->num_files; ++i) {
        xml_process_assetlist_file(xml_files->files[i].name);
    }

    xml_finish();

    asset_image_load_all(main_images, main_image_widths);

    group_set_for_external_files();

    // By default, if the requested image is not found, the roadblock image will be shown.
    // This ensures compatibility with previous release versions of Augustus, which only had roadblocks
    data.roadblock_image_id = assets_get_group_id("Admin_Logistics");
    data.roadblock_image = asset_image_get_from_id(data.roadblock_image_id - IMAGE_MAIN_ENTRIES);
    data.asset_lookup[ASSET_HIGHWAY_BASE_START] = assets_get_image_id("Admin_Logistics", "Highway_Base_Start");
    data.asset_lookup[ASSET_HIGHWAY_BARRIER_START] = assets_get_image_id("Admin_Logistics", "Highway_Barrier_Start");
    data.asset_lookup[ASSET_AQUEDUCT_WITH_WATER] = assets_get_image_id("Admin_Logistics", "Aqueduct_Bridge_Left_Water");
    data.asset_lookup[ASSET_AQUEDUCT_WITHOUT_WATER] = assets_get_image_id("Admin_Logistics", "Aqueduct_Bridge_Left_Empty");
    data.asset_lookup[ASSET_GOLD_SHIELD] = assets_get_image_id("UI", "GoldShield");
    data.asset_lookup[ASSET_HAGIA_SOPHIA_FIX] = assets_get_image_id("UI", "H Sophia Fix");
    data.asset_lookup[ASSET_FIRST_ORNAMENT] = assets_get_image_id("UI", "First Ornament");
    data.asset_lookup[ASSET_CENTER_CAMERA_ON_BUILDING] = assets_get_image_id("UI", "Center Camera Button");
    data.asset_lookup[ASSET_OX] = assets_get_image_id("Walkers", "Ox_Portrait");
    data.asset_lookup[ASSET_UI_RISKS] = assets_get_image_id("UI", "Risk_Widget_Collapse");
    data.asset_lookup[ASSET_UI_SELECTION_CHECKMARK] = assets_get_image_id("UI", "Selection_Checkmark");
}

int assets_load_single_group(const char *file_name, color_t **main_images, int *main_image_widths)
{
    if (!group_create_all(1) || !asset_image_init_array()) {
        log_error("Not enough memory to initialize extra assets. The game will probably crash.", 0, 0);
        return 0;
    }
    xml_init();
    graphics_renderer()->free_image_atlas(ATLAS_EXTRA_ASSET);
    return xml_process_assetlist_file(file_name) && asset_image_load_all(main_images, main_image_widths);
}

int assets_get_group_id(const char *assetlist_name)
{
    image_groups *group = group_get_from_name(assetlist_name);
    if (group) {
        return group->first_image_index + IMAGE_MAIN_ENTRIES;
    }
    log_info("Asset group not found: ", assetlist_name, 0);
    return data.roadblock_image_id;
}

int assets_get_image_id(const char *assetlist_name, const char *image_name)
{
    if (!image_name || !*image_name) {
        return data.roadblock_image_id;
    }
    image_groups *group = group_get_from_name(assetlist_name);
    if (!group) {
        log_info("Asset group not found: ", assetlist_name, 0);
        return data.roadblock_image_id;
    }
    const asset_image *img = asset_image_get_from_id(group->first_image_index);
    while (img && img->index <= group->last_image_index) {
        if (img->id && strcmp(img->id, image_name) == 0) {
            return img->index + IMAGE_MAIN_ENTRIES;
        }
        img = asset_image_get_from_id(img->index + 1);
    }
    log_info("Asset image not found: ", image_name, 0);
    log_info("Asset group is: ", assetlist_name, 0);
    return data.roadblock_image_id;
}

int assets_get_external_image(const char *path, int force_reload)
{
    if (!path || !*path) {
        return 0;
    }
    image_groups *group = group_get_from_name(ASSET_EXTERNAL_FILE_LIST);
    asset_image *img = asset_image_get_from_id(group->first_image_index);
    int was_found = 0;
    while (img && img->index <= group->last_image_index) {
        if (img->id && strcmp(img->id, path) == 0) {
            if (!force_reload) {
                return img->index + IMAGE_MAIN_ENTRIES;
            } else {
                was_found = 1;
                break;
            }
        }
        img = asset_image_get_from_id(img->index + 1);
    }
    if (was_found) {
        graphics_renderer()->free_unpacked_image(&img->img);
        asset_image_unload(img);
    }
    const asset_image *new_img = asset_image_create_external(path);
    if (!new_img) {
        return 0;
    }
    if (group->first_image_index == -1) {
        group->first_image_index = new_img->index;
    }
    group->last_image_index = new_img->index;
    return new_img->index + IMAGE_MAIN_ENTRIES;
}

int assets_lookup_image_id(asset_id id)
{
    return data.asset_lookup[id];
}

const image *assets_get_image(int image_id)
{
    asset_image *img = asset_image_get_from_id(image_id - IMAGE_MAIN_ENTRIES);
    if (!img) {
        img = data.roadblock_image;
    }
    if (!img) {
        return image_get(0);
    }
    return &img->img;
}

void assets_load_unpacked_asset(int image_id)
{
    asset_image *img = asset_image_get_from_id(image_id - IMAGE_MAIN_ENTRIES);
    if (!img) {
        return;
    }
    const color_t *pixels;
    if (img->is_reference) {
        asset_image *referenced_asset =
            asset_image_get_from_id(img->first_layer.calculated_image_id - IMAGE_MAIN_ENTRIES);
        pixels = referenced_asset->data;
    } else {
        pixels = img->data;
    }
    graphics_renderer()->load_unpacked_image(&img->img, pixels);
}
