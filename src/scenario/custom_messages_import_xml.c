#include "custom_messages_import_xml.h"

#include "core/file.h"
#include "core/lang.h"
#include "core/log.h"
#include "core/string.h"
#include "core/xml_parser.h"
#include "scenario/custom_messages.h"
#include "scenario/editor.h"
#include "scenario/event/parameter_data.h"
#include "window/plain_message_dialog.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#define ERROR_MESSAGE_LENGTH 200

static struct {
    int success;
    char error_message[ERROR_MESSAGE_LENGTH];
    int error_line_number;
    uint8_t error_line_number_text[50];
    int version;
    custom_message_t *current_message;
} data;

static int xml_start_custom_messages(void);

static int xml_start_message(void);
static void xml_end_message(void);

static int xml_start_media(void);
static int xml_start_background_music(void);

static void xml_on_title(const char *text);
static void xml_on_subtitle(const char *text);
static void xml_on_text(const char *text);

static void display_and_log_error(const char *msg);

#define XML_TOTAL_ELEMENTS 7

static const xml_parser_element xml_elements[XML_TOTAL_ELEMENTS] = {
    { "messages", xml_start_custom_messages },
    { "message", xml_start_message, xml_end_message, "messages" },
    { "title", 0, 0, "message", xml_on_title },
    { "subtitle", 0, 0, "message", xml_on_subtitle },
    { "text", 0, 0, "message", xml_on_text },
    { "media", xml_start_media, 0, "message" },
    { "background_music", xml_start_background_music, 0, "message" },
};

static int xml_start_custom_messages(void)
{
    if (!data.success) {
        return 0;
    }

    data.version = xml_parser_get_attribute_int("version");
    if (!data.version) {
        data.success = 0;
        display_and_log_error("No version set");
        return 0;
    }
    return 1;
}

static int xml_start_message(void)
{
    if (!data.success) {
        return 0;
    }

    if (!xml_parser_has_attribute("uid")) {
        display_and_log_error("Message has no unique identifier (uid)");
        return 0;
    }

    const char *uid = xml_parser_get_attribute_string("uid");
    data.current_message = custom_messages_create(string_from_ascii(uid));
    if (data.current_message == 0) {
        display_and_log_error("Message unique identifier is not unique");
        return 0;
    }
    return 1;
}

static void xml_end_message(void)
{
    if (!data.current_message->display_text) {
        display_and_log_error("No display text set for message");
    }
}

static void xml_on_title(const char *text)
{
    data.current_message->title = message_media_text_blob_add_encoded(text);
}

static void xml_on_subtitle(const char *text)
{
    data.current_message->subtitle = message_media_text_blob_add_encoded(text);
}

static void xml_on_text(const char *text)
{
    data.current_message->display_text = message_media_text_blob_add_encoded(text);
}

static int xml_start_media(void)
{
    if (!xml_parser_has_attribute("type")) {
        display_and_log_error("No media type specified");
        return 0;
    }

    if (!xml_parser_has_attribute("filename")) {
        display_and_log_error("No media filename specified");
        return 0;
    }

    const char *value = xml_parser_get_attribute_string("type");
    special_attribute_mapping_t *found = scenario_events_parameter_data_get_attribute_mapping_by_text(PARAMETER_TYPE_MEDIA_TYPE, value);
    if (!found) {
        display_and_log_error("Media type invalid");
        return 0;
    }

    const char *media_location = xml_parser_get_attribute_string("filename");
    if (!media_location) {
        media_location = xml_parser_get_attribute_string("location");
    }
    if (!media_location) {
        display_and_log_error("No media location provided");
        return 0;
    }
    data.current_message->linked_media[found->value] = custom_media_create(found->value,
        string_from_ascii(media_location), CUSTOM_MEDIA_LINK_TYPE_CUSTOM_MESSAGE_AS_MAIN, data.current_message->id);
    return 1;
}

static int xml_start_background_music(void)
{
    if (!xml_parser_has_attribute("filename")) {
        display_and_log_error("No media filename specified");
        return 0;
    }

    const char *media_filename = xml_parser_get_attribute_string("filename");
    data.current_message->linked_background_music = custom_media_create(1, string_from_ascii(media_filename),
        CUSTOM_MEDIA_LINK_TYPE_CUSTOM_MESSAGE_AS_BACKGROUND_MUSIC, data.current_message->id);
    return 1;
}

static void display_and_log_error(const char *msg)
{
    data.success = 0;
    data.error_line_number = xml_parser_get_current_line_number();
    snprintf(data.error_message, ERROR_MESSAGE_LENGTH, "%s", msg);
    log_error("Error while importing custom messages from XML. ", data.error_message, 0);
    log_error("Line:", 0, data.error_line_number);

    string_copy(translation_for(TR_EDITOR_IMPORT_LINE), data.error_line_number_text, 50);
    int length = string_length(data.error_line_number_text);

    uint8_t number_as_text[15];
    string_from_int(number_as_text, data.error_line_number, 0);
    string_copy(number_as_text, data.error_line_number_text + length, 50);

    window_plain_message_dialog_show_with_extra(
        TR_EDITOR_UNABLE_TO_LOAD_EVENTS_TITLE, TR_EDITOR_CHECK_LOG_MESSAGE,
        string_from_ascii(data.error_message),
        data.error_line_number_text);
}

static void reset_data(void)
{
    data.success = 1;
    data.version = -1;
    data.error_line_number = -1;
}

static int parse_xml(char *buf, int buffer_length)
{
    reset_data();
    custom_messages_clear_all();
    data.success = 1;
    if (!xml_parser_init(xml_elements, XML_TOTAL_ELEMENTS, 0)) {
        data.success = 0;
    }
    if (data.success) {
        if (!xml_parser_parse(buf, buffer_length, 1)) {
            data.success = 0;
            custom_messages_clear_all();
        }
    }
    xml_parser_free();

    int message_id = scenario_intro_message();

    if (message_id > custom_messages_count() || !custom_messages_get(message_id)->in_use) {
        scenario_editor_set_custom_message_introduction(0);
    }
    message_id = scenario_victory_message();
        if (message_id > custom_messages_count() || !custom_messages_get(message_id)->in_use) {
        scenario_editor_set_custom_victory_message(0);
    }

    return data.success;
}

static char *file_to_buffer(const char *filename, int *output_length)
{
    FILE *file = file_open(filename, "r");
    if (!file) {
        log_error("Error opening empire file", filename, 0);
        return 0;
    }
    fseek(file, 0, SEEK_END);
    int size = ftell(file);
    rewind(file);

    char *buf = malloc(size);
    if (!buf) {
        log_error("Error opening empire file", filename, 0);
        return 0;
    }
    memset(buf, 0, size);
    if (!buf) {
        log_error("Unable to allocate buffer to read XML file", filename, 0);
        free(buf);
        file_close(file);
        return 0;
    }
    *output_length = (int) fread(buf, 1, size, file);
    if (*output_length > size) {
        log_error("Unable to read file into buffer", filename, 0);
        free(buf);
        file_close(file);
        *output_length = 0;
        return 0;
    }
    file_close(file);
    return buf;
}

int custom_messages_xml_parse_file(const char *filename)
{
    int output_length = 0;
    char *xml_contents = file_to_buffer(filename, &output_length);
    if (!xml_contents) {
        return 0;
    }
    int success = parse_xml(xml_contents, output_length);
    free(xml_contents);
    if (!success) {
        log_error("Error parsing file", filename, 0);
    }
    return success;
}
