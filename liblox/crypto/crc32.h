/**
 * CRC32 hash.
 */
#pragma once

#include <stddef.h>
#include <stdint.h>

/**
 * Hash the input data with CRC32
 * @param buffer input buffer.
 * @param length input buffer size.
 * @return crc32 hash.
 */
uint32_t hash_crc32(uint8_t* buffer, size_t length);

/**
 * Continue a hash with more input data.
 * @param buffer input buffer.
 * @param length input length.
 * @param init last hash output.
 * @return crc32 hash.
 */
uint32_t hash_crc32_continue(uint8_t* buffer, size_t length, uint32_t init);
