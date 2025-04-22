#include "custom_messages_export_xml.h"

#include "core/buffer.h"
#include "core/io.h"
#include "core/log.h"
#include "core/string.h"
#include "core/xml_exporter.h"
#include "scenario/custom_messages.h"
#include "scenario/event/parameter_data.h"
#include "window/plain_message_dialog.h"

#include <string.h>

#define XML_EXPORT_MAX_SIZE 5000000

static int export_attribute_media_type(const char *name, int target)
{
    special_attribute_mapping_t *found = scenario_events_parameter_data_get_attribute_mapping_by_value(PARAMETER_TYPE_MEDIA_TYPE, target);
    if (found != 0) {
        xml_exporter_add_attribute_text(name, found->text);
        return 1;
    }
    return 0;
}

static int export_message(custom_message_t *message)
{
    if (!message->in_use) {
        return 1;
    }

    xml_exporter_new_element("message");
    xml_exporter_add_attribute_encoded_text("uid", message->linked_uid->text);

    if (custom_messages_get_title(message)) {
        xml_exporter_new_element("title");
        xml_exporter_add_element_encoded_text(custom_messages_get_title(message));
        xml_exporter_close_element();
    }

    if (custom_messages_get_subtitle(message)) {
        xml_exporter_new_element("subtitle");
        xml_exporter_add_element_encoded_text(custom_messages_get_subtitle(message));
        xml_exporter_close_element();
    }

    xml_exporter_new_element("text");
    xml_exporter_add_element_encoded_text(custom_messages_get_text(message));
    xml_exporter_close_element();

    for (int i = 0; i < CUSTOM_MEDIA_MAX; i++) {
        if (message->linked_media[i] && message->linked_media[i]->type != CUSTOM_MEDIA_UNDEFINED) {
            xml_exporter_new_element("media");
            export_attribute_media_type("type", message->linked_media[i]->type);
            xml_exporter_add_attribute_encoded_text("filename", message->linked_media[i]->filename->text);
            xml_exporter_close_element();
        }
    }

    if (message->linked_background_music && message->linked_background_music->type != CUSTOM_MEDIA_UNDEFINED) {
        xml_exporter_new_element("background_music");
        xml_exporter_add_attribute_encoded_text("filename", message->linked_background_music->filename->text);
        xml_exporter_close_element();
    }

    xml_exporter_close_element();
    
    return 1;
}

static void export_custom_messages(buffer *buf)
{
    xml_exporter_init(buf, "messages");
    xml_exporter_new_element("messages");
    xml_exporter_add_attribute_int("version", CUSTOM_MESSAGES_XML_VERSION);

    int message_count = custom_messages_count();
    for (int i = 1; i <= message_count; i++) {
        custom_message_t *message = custom_messages_get(i);
        export_message(message);
    }
    xml_exporter_close_element();
    xml_exporter_newline();
}

int custom_messages_export_to_xml(const char *filename)
{
    buffer buf;
    int buf_size = XML_EXPORT_MAX_SIZE;
    uint8_t *buf_data = malloc(buf_size);
    if (!buf_data) {
        log_error("Unable to allocate buffer to export messages XML", 0, 0);
        free(buf_data);
        return 0;
    }
    buffer_init(&buf, buf_data, buf_size);
    export_custom_messages(&buf);
    io_write_buffer_to_file(filename, buf.data, buf.index);
    free(buf_data);
    return 1;
}
