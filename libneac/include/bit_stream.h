#ifndef BIT_STREAM_HEADER_INCLUDED
#define BIT_STREAM_HEADER_INCLUDED

#define BIT_STREAM_MODE_READ	0x00
#define BIT_STREAM_MODE_WRITE	0x01

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

typedef struct {
    FILE* file;
    uint32_t mode;
    uint8_t buffer;
    uint32_t buffer_position;
} bit_stream;

/*!
 * @brief           ビットストリームの領域を確保し、そのハンドルを返します。
 * @param *file     ビットストリームで扱うファイルのファイルハンドル
 * @param mode      ビットストリームのモード
 * @return          ビットストリームのハンドル
 */
bit_stream* bit_stream_create(FILE* file, uint32_t mode);

/*!
 * @brief           ビットストリームを初期化します。
 * @param *stream   ビットストリームのハンドル
 */
void bit_stream_init(bit_stream* stream);

/*!
 * @brief           ビットストリームから1ビット読み込みます。
 * @param *stream   ビットストリームのハンドル
 * @return          読み込まれたビット
 */
bool bit_stream_read_bit(bit_stream* stream);

/*!
 * @brief           ビットストリームから任意ビット数の整数を読み込みます。
 * @param *stream   ビットストリームのハンドル
 * @param bits      読み込む整数のビット数
 * @return          読み込まれた整数
 */
uint32_t bit_stream_read_uint(bit_stream* stream, uint32_t bits);

/*!
 * @brief           ビットストリームに1ビット書き込みます。
 * @param *stream   ビットストリームのハンドル
 * @param bit       書き込むビット
 */
void bit_stream_write_bit(bit_stream* stream, bool bit);

/*!
 * @brief           ビットストリームに任意ビット数の整数を書き込みます。
 * @param *stream   ビットストリームのハンドル
 * @param value     書き込む整数
 * @param num_bits  書き込む整数のビット数
 */
void bit_stream_write_uint(bit_stream* stream, uint32_t value, uint32_t num_bits);

/*!
 * @brief			バッファに残っている、ファイルポインタが示すファイルに書き込まれていない値を無条件に書き込みます。
 * @param *stream	ビットストリームのハンドル
 */
void bit_stream_close(bit_stream* stream);

#endif