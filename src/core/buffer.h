#ifndef CORE_BUFFER_H
#define CORE_BUFFER_H

#include <stddef.h>
#include <stdint.h>

/**
* @file
* Read to or write from memory buffer.
*/

/**
* Struct representing a buffer to read from / write to
*/
typedef struct {
    uint8_t *data; /**< Read-only: data */
    size_t size; /**< Read-only: size of the data */
    size_t index; /**< Read-only: bytes read/written so far */
    int overflow; /**< Read-only: indicates attempt to read/write beyond end of buffer */
} buffer;

/**
 * Initializes the buffer with the given data and size.
 * @param buffer Buffer
 * @param data Data to use
 * @param size Size of the data
 */
void buffer_init(buffer *buffer, void *data, int size);

/**
 * Resets the buffer so that reading/writing starts at the beginning again
 * @param buffer Buffer
 */
void buffer_reset(buffer *buffer);

/**
 * Sets the buffer so that reading/writing starts at the specififed offset
 * @param buffer Buffer
 * @param offset Offset to set it to
 */
void buffer_set(buffer *buffer, int offset);

/**
 * Writes an unsigned 8-bit integer
 * @param buffer Buffer
 * @param value Value to write
 */
void buffer_write_u8(buffer *buffer, uint8_t value);

/**
 * Writes an unsigned 16-bit integer
 * @param buffer Buffer
 * @param value Value to write
 */
void buffer_write_u16(buffer *buffer, uint16_t value);

/**
 * Writes an unsigned 32-bit integer
 * @param buffer Buffer
 * @param value Value to write
 */
void buffer_write_u32(buffer *buffer, uint32_t value);

/**
 * Writes a signed 8-bit integer
 * @param buffer Buffer
 * @param value Value to write
 */
void buffer_write_i8(buffer *buffer, int8_t value);

/**
 * Writes a signed 16-bit integer
 * @param buffer Buffer
 * @param value Value to write
 */
void buffer_write_i16(buffer *buffer, int16_t value);

/**
 * Writes a signed 32-bit integer
 * @param buffer Buffer
 * @param value Value to write
 */
void buffer_write_i32(buffer *buffer, int32_t value);

/**
 * Writes raw data
 * @param buffer Buffer
 * @param value Value to write
 * @param size Size in bytes
 */
void buffer_write_raw(buffer *buffer, const void *value, size_t size);

/**
 * Reads an unsigned 8-bit integer
 * @param buffer Buffer
 * @return Read value
 */
uint8_t buffer_read_u8(buffer *buffer);

/**
 * Reads an unsigned 16-bit integer
 * @param buffer Buffer
 * @return Read value
 */
uint16_t buffer_read_u16(buffer *buffer);

/**
 * Reads an unsigned 32-bit integer
 * @param buffer Buffer
 * @return Read value
 */
uint32_t buffer_read_u32(buffer *buffer);

/**
 * Reads a signed 8-bit integer
 * @param buffer Buffer
 * @return Read value
 */
int8_t buffer_read_i8(buffer *buffer);

/**
 * Reads a signed 16-bit integer
 * @param buffer Buffer
 * @return Read value
 */
int16_t buffer_read_i16(buffer *buffer);

/**
 * Reads a signed 32-bit integer
 * @param buffer Buffer
 * @return Read value
 */
int32_t buffer_read_i32(buffer *buffer);

/**
 * Reads raw data
 * @param buffer Buffer
 * @param value Value to read into
 * @param max_size Size of the value, max bytes to read
 * @return Bytes read
 */
size_t buffer_read_raw(buffer *buffer, void *value, size_t max_size);

/**
 * Skip data in the buffer
 * @param buffer Buffer
 * @param size Bytes to skip
 */
void buffer_skip(buffer *buffer, size_t size);

/**
 * Returns whether the pointer of this buffer is at the end of the buffer
 * @param buffer Buffer
 * @return True if pointer is at end of buffer, false otherwise
 */
int buffer_at_end(buffer *buffer);

/**
 * Initializes a buffer for saving a dynamic state piece / file piece.
 * Also stored the actual buffer size at the start of the buffer.
 *
 * @param buf Buffer
 * @param size Size in bytes of the buffer to be saved
 */
void buffer_init_dynamic(buffer *buf, uint32_t size);

/**
 * Initializes a buffer for saving a dynamic state piece / file piece.
 * Also stored the actual buffer size at the start of the buffer.
 *
 * @param buf Buffer
 * @return The size of the buffer in bytes.
 */
uint32_t buffer_load_dynamic(buffer *buf);

/**
 * Initializes a buffer for saving a dynamic state piece / file piece.
 * Also stored the standard info of the piece at the start of the buffer.
 * Size of the piece in bytes will be calculated as (Standard header size + (array_size * struct_size))
 * @param buf Buffer
 * @param array_size Number of elements in the array. If you are saving only a single struct, then this must be 1.
 * @param element_size Size in bytes of a single entry of the array.
 */
void buffer_init_dynamic_array(buffer *buf, uint32_t array_size, uint32_t element_size);

/**
 * Reads the size, version, array_size and struct_size headers from the piece buffer.
 * @param buf Buffer
 * @return Number of array items.
 */
uint32_t buffer_load_dynamic_array(buffer *buf);

#endif // CORE_BUFFER_H
