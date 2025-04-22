#ifndef CORE_XML_PARSER_H
#define CORE_XML_PARSER_H

#define XML_PARSER_MAX_ATTRIBUTES 13
#define XML_PARSER_TAG_MAX_LENGTH 12

/*
 * @brief A structure that holds info about a xml element.
 *
 * Note: You can have many elements with the same name belonging to different parents.
 */
typedef struct {
    const char *name; /**< The name of the element. */
    int (*on_enter)(void);     /**< A pointer to a function that should be called when the parser finds an element with this name. Can be NULL. */
    void (*on_exit)(void);     /**< A pointer to a function that should be called when the parser gets to the end tag of the element with this name. Can be NULL. */
    const char *parent_names;  /**< A list of valid parent names. If the element can have more than one parent, it should be separated by a "|"".*/
    void (*on_text)(const char *text); /**< A pointer to a function that should be called when the parser finds text inside a tag. Can be NULL. */
} xml_parser_element;

int xml_parser_compare_multiple(const char *string, const char *match);

/**
 * @brief Initiates the xml parser structure.
 * 
 * @param elements A list of xml_parser_element structures, indicating the format of the xml file. This will be used for error checking.
 * @param total_elements The total number of elements passed.
 * @param stop_on_invalid_xml Whether the parser should stop when it finds an invalid xml element.
 * @return 1 if init was successful, 0 otherwise.
 */
int xml_parser_init(const xml_parser_element *elements, int total_elements, int stop_on_invalid_xml);

/**
 * @brief Parses a chunk of a xml file. 
 * 
 * @param buffer The pointer to the xml string.
 * @param buffer_size The size of the xml string.
 * @param is_final Whether this is the last piece of the xml string. If you're passing a single string, this should be set to 1.
 * @return 1 if parsing was successful, 0 otherwise.
 */
int xml_parser_parse(const char *buffer, unsigned int buffer_size, int is_final);

/**
 * @brief Whether an element has a specific attribute.
 * 
 * @param key The key to check.
 * @return 1 if the element has the attribute, 0 otherwise.
 */
int xml_parser_has_attribute(const char *key);

/**
 * @brief Gets an attribute as an int.
 * 
 * @param key The key to obtain.
 * @return The value as an int if successful, 0 otherwise.
 */
int xml_parser_get_attribute_int(const char *key);

/**
 * @brief Gets an attribute as a string.
 *
 * @param key The key to obtain.
 * @return  The value as an string if successful, NULL otherwise.
 */
const char *xml_parser_get_attribute_string(const char *key);

/**
 * @brief Copies the string attribute to a new memory space (via malloc) and returns it. You must free() the allocated string.
 *
 * @param key The key to obtain.
 * @return The new string memory if successful, NULL otherwise.
 */
char *xml_parser_copy_attribute_string(const char *key);

/**
 * @brief Gets an attribute as a boolean (0 or 1).
 *
 * Valid values for returning 1 are: "true", "1", "yes", "y", and the same value as the attribute's key.
 *
 * @param key The key to obtain.
 * @return 1 if true, 0 otherwise.
 */
int xml_parser_get_attribute_bool(const char *key);

/**
 * @brief Gets an attribute as an enumeration
 * 
 * @param key The key to obtain.
 * @param values The possibe values for the enum, as an array of strings. You can have multiple values per index by separating them with "|".
 *               If a value that sits in the middle of the enum should never be reached, you can pass NULL for that value instead of a string.
 * @param total_values The total number of values in the array.
 * @param start_offset The offset at which to set the result. see the documentation for the return value.
 * @return If a value is found, it returns "value index + start_offset". Otherwise it returns "start_offset - 1". 
 */
int xml_parser_get_attribute_enum(const char *key, const char **values, int total_values, int start_offset);

/**
 * @brief Gets the current line number the parser is on.
 * 
 * @return int The current line number.
 */
int xml_parser_get_current_line_number(void);

/**
 * @brief Returns the name of the current xml element that the parser is busy with.
 * 
 * @return Name of the current element.
 */
const char *xml_parser_get_current_element_name(void);

/**
 * @brief Returns the name of the parent xml element that the parser is busy with.
 * 
 * @return Name of the parent element, or NULL if root element..
 */
const char *xml_parser_get_parent_element_name(void);

/**
 * @brief Resets the parser, allowing it to be used with another file that has the same element structure.
 */
void xml_parser_reset(void);

/**
 * @brief Frees the memory associated with the parser.
 */
void xml_parser_free(void);

#endif // CORE_XML_PARSER_H
