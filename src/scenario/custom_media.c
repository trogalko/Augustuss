#include "custom_media.h"

#include "core/array.h"
#include "core/log.h"

#define CUSTOM_MEDIA_ARRAY_SIZE_STEP 100

static array(custom_media_t) custom_media;

static int entry_in_use(const custom_media_t *entry)
{
    return entry->type != CUSTOM_MEDIA_UNDEFINED;
}

static void new_entry(custom_media_t *obj, unsigned int position)
{
    obj->id = position;
}

void custom_media_clear(void)
{
    if (custom_media.size) {
        custom_media_t *entry;
        array_foreach(custom_media, entry) {
            message_media_text_blob_mark_entry_as_unused(entry->filename);
        }
    }

    if (!array_init(custom_media, CUSTOM_MEDIA_ARRAY_SIZE_STEP, new_entry, entry_in_use) ||
        !array_next(custom_media)) {
        log_error("Unable to allocate enough memory for the custom media array. The game will now crash.", 0, 0);
    }
}

custom_media_t *custom_media_get(int media_id)
{
    return array_item(custom_media, media_id);
}

custom_media_t *custom_media_create_blank(void)
{
    custom_media_t *entry = 0;
    array_new_item_after_index(custom_media, 1, entry);
    return entry;
}

custom_media_t *custom_media_create(custom_media_type type, const uint8_t *filename, custom_media_link_type link_type, int link_id)
{
    custom_media_t *entry = custom_media_create_blank();
    entry->type = type;
    entry->filename = message_media_text_blob_add(filename);
    entry->link_type = link_type;
    entry->link_id = link_id;

    return entry;
}

void custom_media_save_state(buffer *buf)
{
    uint32_t array_size = custom_media.size;
    uint32_t struct_size = (4 * sizeof(int32_t)) + (1 * sizeof(int16_t));
    buffer_init_dynamic_array(buf, array_size, struct_size);

    custom_media_t *entry;
    array_foreach(custom_media, entry) {
        int entry_id = entry && entry->id ? entry->id : 0;
        buffer_write_i32(buf, entry_id);
        int entry_type = entry && entry->type ? entry->type : 0;
        buffer_write_i32(buf, entry_type);
        int entry_filename_id = entry && entry->filename && entry->filename->id ? entry->filename->id : 0;
        buffer_write_i32(buf, entry_filename_id);
        int entry_link_type = entry && entry->link_type ? entry->link_type : 0;
        buffer_write_i16(buf, entry_link_type);
        int entry_link_id = entry && entry->link_id ? entry->link_id : 0;
        buffer_write_i32(buf, entry_link_id);
    }
}

void custom_media_load_state_entry(buffer *buf, custom_media_t *entry, custom_media_link_type *link_type, int *link_id)
{
    entry->id = buffer_read_i32(buf);
    entry->type = buffer_read_i32(buf);

    // Expects the media text blob to be loaded already.
    int linked_text_blob_id = buffer_read_i32(buf);
    entry->filename = message_media_text_blob_get_entry(linked_text_blob_id);
    entry->link_type = buffer_read_i16(buf);
    entry->link_id = buffer_read_i32(buf);

    *link_type = entry->link_type;
    *link_id = entry->link_id;
}

int custom_media_relink_text_blob(int text_id, text_blob_string_t *new_text_link)
{
    custom_media_t *entry;
    array_foreach(custom_media, entry) {
        if (entry && entry->filename && entry->filename->id == text_id) {
            entry->filename = new_text_link;
            return 1;
        }
    }
    return 0;
}
