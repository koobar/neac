#ifndef BIT_STREAM_H
#define BIT_STREAM_H

#define BIT_STREAM_MODE_READ	0x00
#define BIT_STREAM_MODE_WRITE	0x01

#include <stdio.h>
#include <stdint.h>
#include "file_access.h"
#include "neac_error.h"

typedef struct bit_stream {
	FILE* file;
	uint32_t mode;
	uint8_t buffer;
	uint32_t buffer_position;

} bit_stream;

bit_stream* bit_stream_create(FILE* file, uint32_t mode);
bool bit_stream_read_bit(bit_stream* stream);
uint32_t bit_stream_read_uint(bit_stream* stream, uint32_t bits);
void bit_stream_write_bit(bit_stream* stream, bool bit);
void bit_stream_write_uint(bit_stream* stream, uint32_t value, uint32_t num_bits);
void bit_stream_close(bit_stream* stream);

#endif