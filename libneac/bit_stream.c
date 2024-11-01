#include "./include/bit_stream.h"
#include "./include/file_access.h"
#include "./include/macro.h"
#include "./include/neac_error.h"
#include <stdlib.h>

static const uint8_t buffer_read_position_table[8] = { 1, 2, 3, 4, 5, 6, 7, 0 };
static const uint32_t bit_mask_table[32] = {
    0x00000001, /* 0000 0000 0000 0000 0000 0000 0000 0001 */
    0x00000002, /* 0000 0000 0000 0000 0000 0000 0000 0010 */
    0x00000004, /* 0000 0000 0000 0000 0000 0000 0000 0100 */
    0x00000008, /* 0000 0000 0000 0000 0000 0000 0000 1000 */
    0x00000010, /* 0000 0000 0000 0000 0000 0000 0001 0000 */
    0x00000020, /* 0000 0000 0000 0000 0000 0000 0010 0000 */
    0x00000040, /* 0000 0000 0000 0000 0000 0000 0100 0000 */
    0x00000080, /* 0000 0000 0000 0000 0000 0000 1000 0000 */
    0x00000100, /* 0000 0000 0000 0000 0000 0001 0000 0000 */
    0x00000200, /* 0000 0000 0000 0000 0000 0010 0000 0000 */
    0x00000400, /* 0000 0000 0000 0000 0000 0100 0000 0000 */
    0x00000800, /* 0000 0000 0000 0000 0000 1000 0000 0000 */
    0x00001000, /* 0000 0000 0000 0000 0001 0000 0000 0000 */
    0x00002000, /* 0000 0000 0000 0000 0010 0000 0000 0000 */
    0x00004000, /* 0000 0000 0000 0000 0100 0000 0000 0000 */
    0x00008000, /* 0000 0000 0000 0000 1000 0000 0000 0000 */
    0x00010000, /* 0000 0000 0000 0001 0000 0000 0000 0000 */
    0x00020000, /* 0000 0000 0000 0010 0000 0000 0000 0000 */
    0x00040000, /* 0000 0000 0000 0100 0000 0000 0000 0000 */
    0x00080000, /* 0000 0000 0000 1000 0000 0000 0000 0000 */
    0x00100000, /* 0000 0000 0001 0000 0000 0000 0000 0000 */
    0x00200000, /* 0000 0000 0010 0000 0000 0000 0000 0000 */
    0x00400000, /* 0000 0000 0100 0000 0000 0000 0000 0000 */
    0x00800000, /* 0000 0000 1000 0000 0000 0000 0000 0000 */
    0x01000000, /* 0000 0001 0000 0000 0000 0000 0000 0000 */
    0x02000000, /* 0000 0010 0000 0000 0000 0000 0000 0000 */
    0x04000000, /* 0000 0100 0000 0000 0000 0000 0000 0000 */
    0x08000000, /* 0000 1000 0000 0000 0000 0000 0000 0000 */
    0x10000000, /* 0001 0000 0000 0000 0000 0000 0000 0000 */
    0x20000000, /* 0010 0000 0000 0000 0000 0000 0000 0000 */
    0x40000000, /* 0100 0000 0000 0000 0000 0000 0000 0000 */
    0x80000000  /* 1000 0000 0000 0000 0000 0000 0000 0000 */
};

static inline void bit_stream_write_buffer(bit_stream* stream) {
    if (stream->mode != BIT_STREAM_MODE_WRITE) {
        report_error(NEAC_ERROR_BIT_STREAM_NOT_WRITE_MODE);
        return;
    }

    write_uint8(stream->file, stream->buffer);
    stream->buffer_position = 0;
    stream->buffer = 0;
}

/*!
 * @brief           ビットストリームの領域を確保し、そのハンドルを返します。
 * @param *file     ビットストリームで扱うファイルのファイルハンドル
 * @param mode      ビットストリームのモード
 * @return          ビットストリームのハンドル
 */
bit_stream* bit_stream_create(FILE* file, uint32_t mode) {
    bit_stream* stream = (bit_stream*)malloc(sizeof(bit_stream));
    
    if (stream == NULL) {
        report_error(NEAC_ERROR_BIT_STREAM_CANNOT_ALLOCATE_MEMORY);
        return NULL;
    }

    stream->mode = mode;
    stream->file = file;
    stream->buffer_position = 0;
    stream->buffer = 0;

    return stream;
}

/*!
 * @brief           ビットストリームを初期化します。
 * @param *stream   ビットストリームのハンドル
 */
void bit_stream_init(bit_stream* stream) {
    stream->buffer = 0;
    stream->buffer_position = 0;
}

/*!
 * @brief           ビットストリームから1ビット読み込みます。
 * @param *stream   ビットストリームのハンドル
 * @return          読み込まれたビット
 */
bool bit_stream_read_bit(bit_stream* stream) {
    register bool bit;

    if (stream->mode != BIT_STREAM_MODE_READ) {
        report_error(NEAC_ERROR_BIT_STREAM_NOT_READ_MODE);
        return false;
    }

    if (stream->buffer_position == 0) {
        stream->buffer = read_uint8(stream->file);
    }

    bit = stream->buffer & bit_mask_table[stream->buffer_position];                 /* bit = stream->buffer & LSHIFT(1, stream->buffer_position); */
    stream->buffer_position = buffer_read_position_table[stream->buffer_position];

    return bit;
}

/*!
 * @brief           ビットストリームから任意ビット数の整数を読み込みます。
 * @param *stream   ビットストリームのハンドル
 * @param bits      読み込む整数のビット数
 * @return          読み込まれた整数
 */
uint32_t bit_stream_read_uint(bit_stream* stream, uint32_t bits) {
    register uint32_t value = 0;
    register uint32_t i;

    for (i = 0; i < bits; ++i) {
        value |= LSHIFT(bit_stream_read_bit(stream), bits - 1 - i);
    }

    return value;
}

/*!
 * @brief           ビットストリームに1ビット書き込みます。
 * @param *stream   ビットストリームのハンドル
 * @param bit       書き込むビット
 */
void bit_stream_write_bit(bit_stream* stream, bool bit) {
    if (stream->buffer_position == 8) {
        bit_stream_write_buffer(stream);
    }

    stream->buffer |= LSHIFT(bit, stream->buffer_position);
    stream->buffer_position++;
}

/*!
 * @brief           ビットストリームに任意ビット数の整数を書き込みます。
 * @param *stream   ビットストリームのハンドル
 * @param value     書き込む整数
 * @param num_bits  書き込む整数のビット数
 */
void bit_stream_write_uint(bit_stream* stream, uint32_t value, uint32_t num_bits) {
    register int32_t i;     /* uint32_t型だと、i>=0が永遠に成り立つため、無限ループとなる。*/

    if (stream->mode != BIT_STREAM_MODE_WRITE) {
        report_error(NEAC_ERROR_BIT_STREAM_NOT_WRITE_MODE);
        return;
    }

    for (i = num_bits - 1; i >= 0; --i) {
        bit_stream_write_bit(stream, value & bit_mask_table[i]);    /* bit_stream_write_bit(stream, RSHIFT(value, i) & 1); */
    }
}

/*!
 * @brief			バッファに残っている、ファイルポインタが示すファイルに書き込まれていない値を無条件に書き込みます。
 * @param *stream	ビットストリームのハンドル
 */
void bit_stream_close(bit_stream* stream) {
    if (stream->mode == BIT_STREAM_MODE_WRITE) {
        /* バッファに残っている未書き込みのビットを書き込む */
        bit_stream_write_buffer(stream);
    }

    /* 後始末 */
    stream->buffer_position = 0;
    stream->buffer = 0;
}
