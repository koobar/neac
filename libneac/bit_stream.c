#include "./include/bit_stream.h"

/*!
 * @brief           ビットストリームが書き込みモードの場合に、バッファに格納されている値をファイルに書き込み、バッファとバッファ位置をリセットします。
 * @param *stream   ビットストリームのハンドル
 */
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
 * @brief			ビットストリームの領域を確保し、そのハンドルを返します。
 * @param *file		ビットストリームで扱うファイルのファイルハンドル
 * @param mode		ビットストリームのモード
 * @return			ビットストリームのハンドル
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
 * @brief			ビットストリームから1ビット読み込みます。
 * @param stream	ビットストリームのハンドル
 * @return			読み込まれたビット
 */
bool bit_stream_read_bit(bit_stream* stream) {
    bool bit;

    if (stream->mode != BIT_STREAM_MODE_READ) {
        report_error(NEAC_ERROR_BIT_STREAM_NOT_READ_MODE);
        return false;
    }

    if (stream->buffer_position == 0) {
        stream->buffer = read_uint8(stream->file);
    }

    bit = stream->buffer & (1 << stream->buffer_position);
    stream->buffer_position = (stream->buffer_position + 1) & 7;

    return bit;
}

/*!
 * @brief			ビットストリームから任意ビット数の整数を読み込みます。
 * @param reader	ビットストリームのハンドル
 * @param bits		読み込む整数のビット数
 * @return			読み込まれた整数
 */
uint32_t bit_stream_read_uint(bit_stream* stream, uint32_t bits) {
    uint32_t value = 0, i;

    if (stream->mode != BIT_STREAM_MODE_READ) {
        report_error(NEAC_ERROR_BIT_STREAM_NOT_READ_MODE);
        return 0;
    }

    for (i = 0; i < bits; ++i) {
        value |= (bit_stream_read_bit(stream) << (bits - 1 - i));
    }

    return value;
}

/*!
 * @brief			ビットストリームに1ビット書き込みます。
 * @param stream	ビットストリームのハンドル
 * @param bit		書き込むビット
 */
void bit_stream_write_bit(bit_stream* stream, bool bit) {
    if (stream->buffer_position == 8) {
        bit_stream_write_buffer(stream);
    }

    stream->buffer |= (bit << stream->buffer_position);
    stream->buffer_position++;
}

/*!
 * @brief			ビットストリームに任意ビット数の整数を書き込みます。
 * @param stream	ビットストリームのハンドル
 * @param value		書き込む整数
 * @param num_bits	書き込む整数のビット数
 */
void bit_stream_write_uint(bit_stream* stream, uint32_t value, uint32_t num_bits) {
    int32_t i;		/* uint32_t にすると、i >= 0 が永遠に成り立つため、無限ループになることに注意 */

    if (stream->mode != BIT_STREAM_MODE_WRITE) {
        report_error(NEAC_ERROR_BIT_STREAM_NOT_WRITE_MODE);
        return;
    }

    for (i = num_bits - 1; i >= 0; --i) {
        bit_stream_write_bit(stream, (value >> i) & 1);
    }
}

/*!
 * @brief			バッファに残っている、ファイルポインタが示すファイルに書き込まれていない値を無条件に書き込みます。
 * @param stream	ビットストリームのハンドル
 */
void bit_stream_close(bit_stream* stream) {
    if (stream->mode == BIT_STREAM_MODE_WRITE) {
        /* バッファに残っている、ファイルポインタが示すファイルに書き込まれていない値を無条件に書き込む */
        bit_stream_write_buffer(stream);
    }

    /* 後始末 */
    stream->buffer_position = 0;
    stream->buffer = 0;
}
