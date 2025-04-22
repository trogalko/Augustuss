#ifndef CORE_ARRAY_H
#define CORE_ARRAY_H

#include <stdlib.h>
#include <string.h>

/**
 * Creates an array structure
 * @param T The type of item that the array holds
 */
#define array(T) \
struct { \
    T **items; \
    unsigned int size; \
    unsigned int blocks; \
    unsigned int block_offset; \
    unsigned int bit_offset; \
    void (*constructor)(T *, unsigned int); \
    int (*in_use)(const T *); \
}

/**
 * Clears an array
 * @param a The array structure
 */
#define array_clear(a) \
( \
    array_free((void **)(a).items, (a).blocks), \
    memset(&(a), 0, sizeof(a)) \
)

/**
 * Initiates an array
 * @param a The array structure
 * @param size The size of each block of items
 * @param new_item_callback A function to run when a new item is added to an array. Can be null.
 *        Items are are always zeroed before the function is called.
 *        The function should have the following signature:
 *        void(<item> *, int position). <item> is the new array item, position is its index.
 * @param in_use_callback A function to check if the current position has a valid item. Can be null.
 *        If null, it is assumed that every item in the array is valid.
 *        The function should have the following signature:
 *        int(const <item> *). <item> is the current array item. Should return 1 if the item is being used.
 * @return Whether memory was properly allocated.
 */
#define array_init(a, size, new_item_callback, in_use_callback) \
( \
    array_clear(a), \
    (a).constructor = new_item_callback, \
    (a).in_use = in_use_callback, \
    (a).block_offset = array_next_power_of_two(size) - 1, \
    (a).bit_offset = array_active_bit(array_next_power_of_two(size)), \
    array_create_blocks(a, 1) \
)

/**
 * Creates a new item for the array, either by finding an available empty item or by expanding the array.
 * @param a The array structure
 * @param ptr A pointer that will get the new item. Will be null if there was a memory allocation error.
 */
#define array_new_item(a, ptr) \
{ \
    ptr = 0; \
    int error = 0; \
    if ((a).in_use) { \
        for (unsigned int array_index = 0; array_index < (a).size; array_index++) { \
            if (!(a).in_use(array_item(a, array_index))) { \
                ptr = array_item(a, array_index); \
                memset(ptr, 0, sizeof(**(a).items)); \
                if ((a).constructor) { \
                    (a).constructor(ptr, array_index); \
                } \
                break; \
            } \
        } \
    } \
    if (!error && !ptr) { \
        ptr = array_advance(a); \
    } \
}

/**
 * Creates a new item for the array, either by finding an available empty item or by expanding the array.
 * @param a The array structure
 * @param index The index upon which to start searching for a free slot. If index is greater than the array size,
 *        the array will be expanded.
 * @param ptr A pointer that will get the new item. Will be null if there was a memory allocation error.
 */
#define array_new_item_after_index(a, index, ptr) \
{ \
    ptr = 0; \
    int error = 0; \
    while (index > (a).size) { \
        if (!array_advance(a)) { \
            error = 1; \
            break; \
        } \
    } \
    if (!error && (a).in_use) { \
        for (unsigned int array_index = index; array_index < (a).size; array_index++) { \
            if (!(a).in_use(array_item(a, array_index))) { \
                ptr = array_item(a, array_index); \
                memset(ptr, 0, sizeof(**(a).items)); \
                if ((a).constructor) { \
                    (a).constructor(ptr, array_index); \
                } \
                break; \
            } \
        } \
    } \
    if (!error && !ptr) { \
        ptr = array_advance(a); \
    } \
}

/**
 * Removes an item from an array, moving the other items left and calling their constructors if applicable
 * @param a The array structure
 * @param index The array index to remove
 */
#define array_remove_item(a, index) \
{ \
    for (unsigned int array_index = index; array_index + 1 < (a).size; array_index++) { \
        memcpy(array_item(a, array_index), array_item(a, array_index + 1), sizeof(**(a).items)); \
        if ((a).constructor && (!(a).in_use || (a).in_use(array_item(a, array_index)))) { \
            (a).constructor(array_item(a, array_index), array_index); \
        } \
    } \
    if (index < (a).size) { \
        memset(array_item(a, (a).size - 1), 0, sizeof(**(a).items)); \
        (a).size--; \
    } \
}

/**
 * Advances an array, creating a new item, incrementing size and increasing the memory buffer if needed
 * @param a The array structure
 * @return A pointer to the newest item of the array, or 0 if there was a memory allocation error
 */
#define array_advance(a) \
( \
    (a).size >> (a).bit_offset < (a).blocks || \
    array_create_blocks(a, 1) ? \
    array_next(a) : 0 \
)

/**
 * Gets the array item at the specified position
 * @param a The array structure
 * @param position The position to fetch
 * @return A pointer to the item at the requested position
 */
#define array_item(a, position) \
( \
    &(a).items[(position) >> (a).bit_offset][(position) & (a).block_offset] \
)

/**
 * Returns the first item of the array, or 0 if the array has no items
 * @param a The array structure
 * @return A pointer to the first item of the array, or 0 if the array has no items
 */
#define array_first(a) \
    ( (a).size > 0 ? (a).items[0] : 0 )

/**
 * Returns the last item of the array, or 0 if the array has no items
 * @param a The array structure
 * @return A pointer to the last item of the array, or 0 if the array has no items
 */
#define array_last(a) \
    ( (a).size > 0 ? array_item(a, (a).size - 1) : 0 )

/**
 * Iterates through an array
 * @param a The array structure
 * @param item A pointer to the array item that will be used to traverse the structure
 * @note You can use the array_index parameter to retrieve the index of the current item
 */
#define array_foreach(a, item) \
    for(unsigned int array_index = 0; array_index < (a).size && ((item) = array_item(a, array_index)) != 0; array_index++)

/**
 * Iterates through an array, calling the callback function for each item
 * @param a The array structure
 * @param callback The function callback to call for each item, in the format "callback(T *item, unsigned int array_index)"
 * @note You can use the array_index parameter to retrieve the index of the current item
 */
#define array_foreach_callback(a, callback) \
    for(unsigned int array_index = 0; array_index < (a).size && array_item(a, array_index) != 0; array_index++) { \
        callback(array_item(a, array_index)); \
    }

/**
 * Trims an array, removing its latest items that aren't being used until the first one is used.
 * The first item of the array is always kept.
 * @param a The array structure
 */
#define array_trim(a) \
{ \
    if ((a).size > 1 && (a).in_use) { \
        while ((a).size - 1 && !(a).in_use(array_item(a, (a).size - 1))) { \
            (a).size--; \
        } \
    } \
}

/**
 * Packs an array, making all used elements of an array contiguous
 * Moved elements call their constructors
 * This function only does anything if the array has an in_use callback
 * @param a The array structure
 */
#define array_pack(a) \
{ \
    if ((a).in_use) { \
        unsigned int items_to_move = 0; \
        for (unsigned int array_index = 0; array_index < (a).size; array_index++) { \
            if (!(a).in_use(array_item(a, array_index))) { \
                items_to_move++; \
                continue; \
            } \
            if (!items_to_move) { \
                continue; \
            } \
            unsigned int new_index = array_index - items_to_move; \
            memcpy(array_item(a, new_index), array_item(a, array_index), sizeof(**(a).items)); \
            if ((a).constructor) { \
                (a).constructor(array_item(a, new_index), new_index); \
            } \
        } \
        if (items_to_move) { \
            for (unsigned int array_index = (a).size - items_to_move; array_index < (a).size; array_index++) { \
                memset(array_item(a, array_index), 0, sizeof(**(a).items)); \
            } \
            (a).size -= items_to_move; \
        } \
    } \
}

/**
 * Expands an array to fit the specified amount of items. The array may become larger than size, but never smaller.
 * @param a The array structure
 * @param Size The size that the array should at least expand into
 * @return Whether memory was properly allocated.
 */
#define array_expand(a, size) \
( \
    array_create_blocks(a, (size) > 0 ? (((size) - 1) >> (a).bit_offset) + 1 - (a).blocks : 0) \
)

/**
 * Gets the next item of the array without checking for memory bounds.
 * ONLY use when you're SURE the array memory bounds won't be exceeded!
 * @param a The array structure
 * @return A pointer to the newest item of the array, or 0 if there was a memory allocation error
 */
#define array_next(a) \
( \
    memset(array_item(a, (a).size), 0, sizeof(**(a).items)), \
    (a).constructor ? (a).constructor(array_item(a, (a).size), (a).size) : (void) 0, \
    (a).size++, \
    array_item(a, (a).size - 1) \
)

/**
 * This definition is private and should not be used
 */
#define array_create_blocks(a, num_blocks) \
( \
    array_add_blocks((void ***)&(a).items, &(a).blocks, (a).block_offset + 1, sizeof(**(a).items), num_blocks) \
)

/**
 * This function is private and should not be used
 */
int array_add_blocks(void ***data, unsigned int *blocks, unsigned int items_per_block, unsigned int item_size, unsigned int num_blocks);

/**
 * This function is private and should not be used
 */
void array_free(void **data, unsigned int blocks);

/**
 * Private helper compile-time functions for finding the next power of two into which a number fits
 */
#define array_next_power_of_two_0(v) ((v) - 1) 
#define array_next_power_of_two_1(v) (array_next_power_of_two_0(v) | array_next_power_of_two_0(v) >> 1)
#define array_next_power_of_two_2(v) (array_next_power_of_two_1(v) | array_next_power_of_two_1(v) >> 2)
#define array_next_power_of_two_3(v) (array_next_power_of_two_2(v) | array_next_power_of_two_2(v) >> 4)
#define array_next_power_of_two_4(v) (array_next_power_of_two_3(v) | array_next_power_of_two_3(v) >> 8)
#define array_next_power_of_two_5(v) (array_next_power_of_two_4(v) | array_next_power_of_two_4(v) >> 16)
#define array_next_power_of_two(v) (((v) < 1) ? 1 : array_next_power_of_two_5(v) + 1)

/**
 * Private helper compile-time functions for finding the which bit is active on a power of two
 */
#define array_active_bit_1(n) (((n) >= 2) ? 1 : 0)
#define array_active_bit_2(n) (((n) >= 1 << 2) ? (2 + array_active_bit_1((n) >> 2)) : array_active_bit_1(n))
#define array_active_bit_3(n) (((n) >= 1 << 4) ? (4 + array_active_bit_2((n) >> 4)) : array_active_bit_2(n))
#define array_active_bit_4(n) (((n) >= 1 << 8) ? (8 + array_active_bit_3((n) >> 8)) : array_active_bit_3(n))
#define array_active_bit(n)   (((n) >= 1 << 16) ? (16 + array_active_bit_4((n) >> 16)) : array_active_bit_4(n))

#endif // CORE_ARRAY_H
