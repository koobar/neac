#ifndef WAVE_FILE_READER_HEADER_INCLUDED
#define WAVE_FILE_READER_HEADER_INCLUDED

#include <stdint.h>
#include <stdio.h>

typedef struct {
    FILE* wave_file;
    uint32_t stream_size;
    uint32_t sample_rate;
    uint16_t bits_per_sample;
    uint16_t num_channels;
    uint32_t num_samples;
    uint32_t num_samples_read;
    uint32_t avr_bytes_per_sec;
    uint16_t block_size;
} wave_file_reader;

/*!
 * @brief           wave_file_readerのハンドルを生成します。
 * @param path      読み込むWAVファイルのパス
 * @return          wave_file_readerのハンドル
 */
wave_file_reader* wave_file_reader_create(const char* path);

/*!
 * @brief           WAVファイルを開きます。
 * @param *reader   wave_file_readerのハンドル
 * @param *path     ファイルパス
 */
void wave_file_reader_open(wave_file_reader* reader, const char* path);

/*!
 * @brief           指定されたハンドルで開かれたWAVファイルを閉じます。
 * @param *reader   wave_file_readerのハンドル
 */
void wave_file_reader_close(const wave_file_reader* reader);

/*!
 * @brief           指定されたハンドルで開かれたWAVファイルのサンプリング周波数を取得します。
 * @param *reader   wave_file_readerのハンドル
 * @return          サンプリング周波数
 */
uint32_t wave_file_reader_get_sample_rate(const wave_file_reader* reader);

/*!
 * @brief           指定されたハンドルで開かれたWAVファイルの量子化ビット数を取得します。
 * @param *reader   wave_file_readerのハンドル
 * @return          量子化ビット数
 */
uint16_t wave_file_reader_get_bits_per_sample(const wave_file_reader* reader);

/*!
 * @brief           指定されたハンドルで開かれたWAVファイルのチャンネル数を取得します。
 * @param *reader   wave_file_readerのハンドル
 * @return          チャンネル数
 */
uint16_t wave_file_reader_get_num_channels(const wave_file_reader* reader);

/*!
 * @brief           指定されたハンドルで開かれたWAVファイルの総サンプル数を取得します。
 * @param *reader   wave_file_readerのハンドル
 * @return          総サンプル数
 */
uint32_t wave_file_reader_get_num_samples(const wave_file_reader* reader);

/*!
 * @brief           指定されたハンドルで開かれたWAVファイルから次のサンプルを読み込みます。
 * @param *reader   wave_file_readerのハンドル
 * @return          サンプル
 */
int32_t wave_file_reader_read_sample(wave_file_reader* reader);

#endif