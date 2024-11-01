#include "./include/wave_file_writer.h"
#include <stdlib.h>

/*!
 * @brief       指定されたファイルポインタが示すファイルにASCII文字を書き込む。
 * @param file  ファイルポインタ
 * @param value 書き込む値
 */
static inline void write_char(FILE* file, const char value) {
    fwrite(&value, sizeof(char), 1, file);
}

/*!
 * @brief       指定されたファイルポインタが示すファイルに16ビット整数を書き込む。
 * @param file  ファイルポインタ
 * @param value 書き込む値
 */
static inline void write_uint16(FILE* file, const uint16_t value) {
    fwrite(&value, sizeof(uint16_t), 1, file);
}

/*!
 * @brief       指定されたファイルポインタが示すファイルに32ビット整数を書き込む。
 * @param file  ファイルポインタ
 * @param value 書き込む値
 */
static inline void write_uint32(FILE* file, const uint32_t value) {
    fwrite(&value, sizeof(uint32_t), 1, file);
}

/*!
 * @brief       指定されたファイルポインタが示すファイルに16符号付きビット整数を書き込む。
 * @param file  ファイルポインタ
 * @param value 書き込む値
 */
static inline void write_int16(FILE* file, const int16_t value) {
    fwrite(&value, sizeof(int16_t), 1, file);
}

/*!
 * @brief           指定されたハンドルのwave_file_writerが保持するフォーマット情報に基づいたチャンクサイズを計算します。
 * @param *writer   wave_file_readerのハンドル
 * @return          チャンクサイズ
 */
static uint32_t compute_chunk_size(const wave_file_writer* writer) {
    uint32_t size_of_fmt_chunk = 24;
    uint32_t bytes_per_sample = writer->bits_per_sample / 8;
    uint32_t size_of_data_chunk = 4 + writer->num_samples * bytes_per_sample;
    uint32_t chunk_size = size_of_data_chunk + size_of_fmt_chunk;

    return chunk_size;
}

/*!
 * @brief           指定されたハンドルのwave_file_writerで、WAVEヘッダ部の書き込みを行います。
 * @param *writer   wave_file_readerのハンドル
 */
static void write_wave_header(const wave_file_writer* writer) {
    /* RIFFのマジックナンバーを書き込む。 */
    write_char(writer->wave_file, 0x52);
    write_char(writer->wave_file, 0x49);
    write_char(writer->wave_file, 0x46);
    write_char(writer->wave_file, 0x46);

    /* チャンクサイズを求めて書き込む。 */
    write_uint32(writer->wave_file, compute_chunk_size(writer));

    /* 'WAVE' をASCIIコードで書き込む。 */
    write_char(writer->wave_file, 0x57);
    write_char(writer->wave_file, 0x41);
    write_char(writer->wave_file, 0x56);
    write_char(writer->wave_file, 0x45);
}

/*!
 * @brief           指定されたハンドルのwave_file_writerで、fmtチャンクの書き込みを行います。
 * @param *writer   wave_file_readerのハンドル
 */
static void write_fmt_chunk(const wave_file_writer* writer) {
    uint32_t avr_bytes_per_sec;
    uint16_t block_size;

    /* fmt チャンクのヘッダを書き込む。 */
    write_char(writer->wave_file, 0x66);
    write_char(writer->wave_file, 0x6d);
    write_char(writer->wave_file, 0x74);
    write_char(writer->wave_file, 0x20);

    /* チャンクのバイト数を書き込む。 */
    write_uint32(writer->wave_file, 16);

    /* オーディオフォーマットを書き込む。 */
    write_uint16(writer->wave_file, 0x0001);

    /* チャンネル数を書き込む。 */
    write_uint16(writer->wave_file, writer->num_channels);

    /* サンプルレートを書き込む。 */
    write_uint32(writer->wave_file, writer->sample_rate);

    /* 1秒あたりの平均バイト数を書き込む。 */
    avr_bytes_per_sec = writer->sample_rate * (writer->bits_per_sample / 8) * writer->num_channels;
    write_uint32(writer->wave_file, avr_bytes_per_sec);

    /* ブロックのサイズを書き込む。 */
    block_size = writer->num_channels * writer->bits_per_sample / 8;
    write_uint16(writer->wave_file, block_size);

    /* 量子化ビット数を書き込む。 */
    write_uint16(writer->wave_file, writer->bits_per_sample);
}

/*!
 * @brief           指定されたハンドルのwave_file_writerで、dataチャンクの書き込みを行います。
 * @param *writer   wave_file_readerのハンドル
 */
static void write_data_chunk_header(const wave_file_writer* writer) {
    uint32_t bytes_per_sample, data_chunk_size;

    /* 'data' をASCIIコードで書き込む。 */
    write_char(writer->wave_file, 0x64);
    write_char(writer->wave_file, 0x61);
    write_char(writer->wave_file, 0x74);
    write_char(writer->wave_file, 0x61);

    /* チャンクサイズを書き込む。 */
    bytes_per_sample = writer->bits_per_sample / 8;
    data_chunk_size = writer->num_samples * bytes_per_sample;
    write_uint32(writer->wave_file, data_chunk_size);
}

/*!
 * @brief           wave_file_writerのハンドルを生成します。
 * @param *path     出力するWAVファイルのパス
 * @return          wave_file_writerのハンドル
 */
wave_file_writer* wave_file_writer_create(const char* path) {
    wave_file_writer* result = (wave_file_writer*)malloc(sizeof(wave_file_writer));

    if (result == NULL) {
        return NULL;
    }

    wave_file_writer_open(result, path);
    return result;
}

/*!
 * @brief           WAVファイルを作成し、指定されたハンドルで開きます。
 * @param *writer   wave_file_writerのハンドル
 * @param *path     WAVファイルを作成するパス
 */
void wave_file_writer_open(wave_file_writer* writer, const char* path) {
    errno_t err;
    
    err = fopen_s(&writer->wave_file, path, "wb");

    /* ファイルを開けなかった場合、エラーを報告して何もしない */
    if (err != 0) {
        return;
    }

    writer->sample_rate = 0;
    writer->bits_per_sample = 0;
    writer->num_channels = 0;
    writer->num_samples = 0;
    writer->flag_file_header_write = false;
    writer->flag_fmt_chunk_write = false;
    writer->flag_data_chunk_header_write = false;
}

/*!
 * @brief           指定されたハンドルで作成したWAVファイルを閉じます。
 * @param *writer   wave_file_writerのハンドル
 */
void wave_file_writer_close(const wave_file_writer* writer) {
    fclose(writer->wave_file);
}

/*!
 * @brief                   指定されたハンドルで作成したWAVファイルのPCMのフォーマットを設定します。
 * @param *writer           wave_file_writerのハンドル
 * @param sample_rate       サンプリング周波数
 * @param bits_per_sample   量子化ビット数
 * @param num_channels      チャンネル数
 */
void wave_file_writer_set_pcm_format(wave_file_writer* writer, uint32_t sample_rate, uint16_t bits_per_sample, uint16_t num_channels) {
    writer->sample_rate = sample_rate;
    writer->bits_per_sample = bits_per_sample;
    writer->num_channels = num_channels;
}

/*!
 * @brief                   指定されたハンドルで作成したWAVファイルの総サンプル数を設定します。
 * @param *writer           wave_file_writerのハンドル
 * @param num_samples       総サンプル数
 */
void wave_file_writer_set_num_samples(wave_file_writer* writer, uint32_t num_samples) {
    writer->num_samples = num_samples;
}

/*!
 * @brief                   指定されたハンドルで作成したWAVEファイルへのサンプルの書き込みを開始します。最初のサンプルの書き込み前に、必ず、この関数を呼び出してください。
 * @param *writer           wave_file_writerのハンドル
 */
void wave_file_writer_begin_write(const wave_file_writer* writer) {
    write_wave_header(writer);
    write_fmt_chunk(writer);
    write_data_chunk_header(writer);
}

/*!
 * @brief                   指定されたハンドルで作成したWAVファイルへサンプルを書き込みます。最初にこの関数を呼び出す前に、必ず、wave_file_writer_begin_write 関数を呼び出してください。
 * @param *writer           wave_file_writerのハンドル
 * @param sample            書き込むサンプル
 */
void wave_file_writer_write_sample(const wave_file_writer* writer, int32_t sample) {
    switch (writer->bits_per_sample)
    {
    case 16:
        write_int16(writer->wave_file, (int16_t)sample);
        break;
    case 24:
        write_char(writer->wave_file, (char)(sample >> 0) & 0xFF);
        write_char(writer->wave_file, (char)(sample >> 8) & 0xFF);
        write_char(writer->wave_file, (char)(sample >> 16) & 0xFF);
        break;
    }
}

/*!
 * @brief                   指定されたハンドルで作成したWAVファイルへのサンプルの書き込みを終了します。すべてのサンプルを書き込んだ後に、必ずこの関数を呼び出してください。
 * @param *writer           wave_file_readerのハンドル
 */
void wave_file_writer_end_write(const wave_file_writer* writer) {
    fflush(writer->wave_file);
    fclose(writer->wave_file);
}