#ifndef WAVE_FILE_WRITER_HEADER_INCLUDED
#define WAVE_FILE_WRITER_HEADER_INCLUDED

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

typedef struct {
    FILE* wave_file;
    uint32_t sample_rate;
    uint16_t bits_per_sample;
    uint16_t num_channels;
    uint32_t num_samples;
    bool flag_file_header_write;
    bool flag_fmt_chunk_write;
    bool flag_data_chunk_header_write;
} wave_file_writer;


/*!
 * @brief           wave_file_writerのハンドルを生成します。
 * @param *path     出力するWAVファイルのパス
 * @return          wave_file_writerのハンドル
 */
wave_file_writer* wave_file_writer_create(const char* path);

/*!
 * @brief           WAVファイルを作成し、指定されたハンドルで開きます。
 * @param *writer   wave_file_writerのハンドル
 * @param *path     WAVファイルを作成するパス
 */
void wave_file_writer_open(wave_file_writer* writer, const char* path);

/*!
 * @brief           指定されたハンドルで作成したWAVファイルを閉じます。
 * @param *writer   wave_file_writerのハンドル
 */
void wave_file_writer_close(const wave_file_writer* writer);

/*!
 * @brief                   指定されたハンドルで作成したWAVファイルのPCMのフォーマットを設定します。
 * @param *writer           wave_file_writerのハンドル
 * @param sample_rate       サンプリング周波数
 * @param bits_per_sample   量子化ビット数
 * @param num_channels      チャンネル数
 */
void wave_file_writer_set_pcm_format(wave_file_writer* writer, uint32_t sample_rate, uint16_t bits_per_sample, uint16_t num_channels);

/*!
 * @brief               指定されたハンドルで作成したWAVファイルの総サンプル数を設定します。
 * @param *writer       wave_file_writerのハンドル
 * @param num_samples   総サンプル数
 */
void wave_file_writer_set_num_samples(wave_file_writer* writer, uint32_t num_samples);

/*!
 * @brief           指定されたハンドルで作成したWAVEファイルへのサンプルの書き込みを開始します。最初のサンプルの書き込み前に、必ず、この関数を呼び出してください。
 * @param *writer   wave_file_writerのハンドル
 */
void wave_file_writer_begin_write(const wave_file_writer* writer);

/*!
 * @brief           指定されたハンドルで作成したWAVファイルへサンプルを書き込みます。最初にこの関数を呼び出す前に、必ず、wave_file_writer_begin_write 関数を呼び出してください。
 * @param *writer   wave_file_writerのハンドル
 * @param sample    書き込むサンプル
 */
void wave_file_writer_write_sample(const wave_file_writer* writer, int32_t sample);

/*!
 * @brief           指定されたハンドルで作成したWAVファイルへのサンプルの書き込みを終了します。すべてのサンプルを書き込んだ後に、必ずこの関数を呼び出してください。
 * @param *writer   wave_file_readerのハンドル
 */
void wave_file_writer_end_write(const wave_file_writer* writer);

#endif