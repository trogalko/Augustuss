#include "xml.h"

#include "assets/assets.h"
#include "assets/group.h"
#include "assets/image.h"
#include "core/calc.h"
#include "core/dir.h"
#include "core/file.h"
#include "core/log.h"
#include "core/png_read.h"
#include "core/string.h"
#include "core/xml_parser.h"
#include "graphics/renderer.h"

#include <string.h>

#define XML_BUFFER_SIZE 1024
#define XML_TOTAL_ELEMENTS 5

static int xml_start_assetlist_element(void);
static int xml_start_image_element(void);
static int xml_start_layer_element(void);
static int xml_start_animation_element(void);
static int xml_start_frame_element(void);
static void xml_end_assetlist_element(void);
static void xml_end_image_element(void);
static void xml_end_animation_element(void);

static struct {
    char base_path[FILE_NAME_MAX];
    int finished;
    int in_animation;
    image_groups *current_group;
    asset_image *current_image;
    int initialized;
} data;

static const xml_parser_element xml_elements[XML_TOTAL_ELEMENTS] = {
    { "assetlist", xml_start_assetlist_element, xml_end_assetlist_element },
    { "image", xml_start_image_element, xml_end_image_element, "assetlist" },
    { "layer", xml_start_layer_element, 0, "image" },
    { "animation", xml_start_animation_element, xml_end_animation_element, "image" },
    { "frame", xml_start_frame_element, 0, "animation"}
};

static const char *INVERT_VALUES[3] = { "horizontal", "vertical", "both" };
static const char *ROTATE_VALUES[3] = { "90", "180", "270" };

static void set_asset_image_base_path(const char *name)
{
    snprintf(data.base_path, FILE_NAME_MAX, "%s/%s", ASSETS_IMAGE_PATH, name);
}

static int xml_start_assetlist_element(void)
{
    data.current_group = group_get_new();
    char *name = xml_parser_copy_attribute_string("name");
    if (!name || *name == '\0') {
        free(name);
        return 0;
    }
    data.current_group->name = name;
    data.current_image = 0;
    set_asset_image_base_path(data.current_group->name);
    return 1;
}

static int xml_start_image_element(void)
{
    asset_image *img = asset_image_create();
    if (!img) {
        return 0;
    }
    if (!data.current_image) {
        data.current_group->first_image_index = img->index;
    }
    data.current_group->last_image_index = img->index;
    data.current_image = img;

    img->id = xml_parser_copy_attribute_string("id");
    const char *path = xml_parser_get_attribute_string("src");
    img->img.width = xml_parser_get_attribute_int("width");
    img->img.height = xml_parser_get_attribute_int("height");
    const char *group = xml_parser_get_attribute_string("group");
    const char *image_id = xml_parser_get_attribute_string("image");
    img->img.is_isometric = xml_parser_get_attribute_bool("isometric");
    if (img->img.is_isometric) {
        asset_image_count_isometric();
    }

#ifdef BUILDING_ASSET_PACKER
    if (img->img.width || img->img.height) {
        img->has_defined_size = 1;
    }
#endif

    img->last_layer = &img->first_layer;
    if (path || group) {
        asset_image_add_layer(img, path, group, image_id, 0, 0, 0, 0, 0, 0, INVERT_NONE, ROTATE_NONE, PART_BOTH, 0);
    }
    return 1;
}

static int xml_start_layer_element(void)
{
    asset_image *img = data.current_image;
    static const char *part_values[2] = { "footprint", "top" };
    static const char *mask_values[2] = { "grayscale", "alpha" };

    const char *path = xml_parser_get_attribute_string("src");
    const char *group = xml_parser_get_attribute_string("group");
    const char *image_id = xml_parser_get_attribute_string("image");
    int src_x = xml_parser_get_attribute_int("src_x");
    int src_y = xml_parser_get_attribute_int("src_y");
    int offset_x = xml_parser_get_attribute_int("x");
    int offset_y = xml_parser_get_attribute_int("y");
    int width = xml_parser_get_attribute_int("width");
    int height = xml_parser_get_attribute_int("height");
    layer_invert_type invert = xml_parser_get_attribute_enum("invert", INVERT_VALUES, 3, INVERT_HORIZONTAL);
    layer_rotate_type rotate = xml_parser_get_attribute_enum("rotate", ROTATE_VALUES, 3, ROTATE_90_DEGREES);
    layer_isometric_part part = xml_parser_get_attribute_enum("part", part_values, 2, PART_FOOTPRINT);
    layer_mask mask = xml_parser_get_attribute_enum("mask", mask_values, 2, LAYER_MASK_GRAYSCALE);

    if (!asset_image_add_layer(img, path, group, image_id, src_x, src_y,
        offset_x, offset_y, width, height, invert, rotate, part == PART_NONE ? PART_BOTH : part, mask)) {
        log_info("Invalid layer for image", img->id, 0);
    }
    return 1;
}

static int xml_start_animation_element(void)
{
    asset_image *img = data.current_image;
    if (img->img.animation) {
        return 1;
    }
    img->img.animation = malloc(sizeof(image_animation));
    if (!img->img.animation) {
        return 0;
    }
    memset(img->img.animation, 0, sizeof(image_animation));

    img->img.animation->num_sprites = xml_parser_get_attribute_int("frames");
    img->img.animation->speed_id = calc_bound(xml_parser_get_attribute_int("speed"), 0, 50);
    img->img.animation->can_reverse = xml_parser_get_attribute_bool("reversible");
    img->img.animation->sprite_offset_x = xml_parser_get_attribute_int("x");
    img->img.animation->sprite_offset_y = xml_parser_get_attribute_int("y");

    if (!img->img.animation->num_sprites) {
        data.in_animation = 1;
    }
    return 1;
}

static int xml_start_frame_element(void)
{
    if (!data.in_animation) {
        return 1;
    }
    asset_image *img = asset_image_create();
    if (!img) {
        return 0;
    }

    const char *path = xml_parser_get_attribute_string("src");
    const char *group = xml_parser_get_attribute_string("group");
    const char *image_id = xml_parser_get_attribute_string("image");
    int src_x = xml_parser_get_attribute_int("src_x");
    int src_y = xml_parser_get_attribute_int("src_y");
    int offset_x = xml_parser_get_attribute_int("x");
    int offset_y = xml_parser_get_attribute_int("y");
    int width = xml_parser_get_attribute_int("width");
    int height = xml_parser_get_attribute_int("height");
    layer_invert_type invert = xml_parser_get_attribute_enum("invert", INVERT_VALUES, 3, INVERT_HORIZONTAL);
    layer_rotate_type rotate = xml_parser_get_attribute_enum("rotate", ROTATE_VALUES, 3, ROTATE_90_DEGREES);

    img->last_layer = &img->first_layer;
    if (!asset_image_add_layer(img, path, group, image_id, src_x, src_y,
        offset_x, offset_y, width, height, invert, rotate, PART_BOTH, 0)) {
        img->active = 0;
        return 1;
    }
#ifndef BUILDING_ASSET_PACKER
    if (!img->img.width || !img->img.height) {
        asset_image_unload(img);
        return 1;
    }
    img->img.width += offset_x;
    img->img.height += offset_y;
    asset_image_check_and_handle_reference(img);
#else
    data.current_image->has_frame_elements = 1;
#endif
    data.current_group->last_image_index = img->index;
    data.current_image->img.animation->num_sprites++;

    return 1;
}

static void xml_end_assetlist_element(void)
{
    data.finished = 1;
    data.current_group = 0;
}

static void xml_end_image_element(void)
{
#ifndef BUILDING_ASSET_PACKER
    image *img = &data.current_image->img;
    if (img->is_isometric) {
        if (((img->width + 2) % (FOOTPRINT_WIDTH + 2)) != 0) {
            log_info("Isometric image has invalid width", data.current_image->id, img->width);
        }
    }
    if (!img->width || !img->height) {
        asset_image_unload(data.current_image);
        return;
    }

    asset_image_check_and_handle_reference(data.current_image);
#endif
}

static void xml_end_animation_element(void)
{
    data.in_animation = 0;
}

void xml_init(void)
{
    if (!data.initialized) {
        xml_parser_init(xml_elements, XML_TOTAL_ELEMENTS, 0);
        data.initialized = 1;
    }
}

int xml_process_assetlist_file(const char *xml_file_name)
{
    log_info("Loading assetlist file", xml_file_name, 0);

    char full_path[FILE_NAME_MAX];
    snprintf(full_path, FILE_NAME_MAX, "%s/%s", ASSETS_IMAGE_PATH, xml_file_name);

    FILE *xml_file = file_open_asset(full_path, "r");

    if (!xml_file) {
        log_error("Error opening assetlist file", xml_file_name, 0);
        return 0;
    }

    char buffer[XML_BUFFER_SIZE];
    int done = 0;
    int error = 0;

    xml_parser_reset();

    do {
        size_t bytes_read = fread(buffer, 1, XML_BUFFER_SIZE, xml_file);
        done = bytes_read < sizeof(buffer);
        if (!xml_parser_parse(buffer, (unsigned int) bytes_read, done)) {
            log_error("Error parsing file", xml_file_name, 0);
            error = 1;
            break;
        }
    } while (!done);

    if (data.current_group && (error || !data.finished)) {
        group_unload_current();
    }
#ifdef BUILDING_ASSET_PACKER
    else {
        size_t buf_size = sizeof(char *) * (strlen(xml_file_name) + 1);
        char *path = malloc(buf_size);
        if (!path) {
            error = 1;
            group_unload_current();
        } else {
            memcpy(path, xml_file_name, buf_size);
            group_get_current()->path = path;
        }
    }
#endif

    data.finished = 0;

    file_close(xml_file);

    return !error;
}

void xml_finish(void)
{
    xml_parser_free();
    data.initialized = 0;
}

void xml_get_full_image_path(char *full_path, const char *image_file_name)
{
    if (snprintf(full_path, FILE_NAME_MAX, "%s/%s.png", data.base_path, image_file_name) > FILE_NAME_MAX) {
        log_error("Image path too long", image_file_name, 0);
    }
}
