#include "./include/wave_file_reader.h"
#include <stdbool.h>
#include <stdlib.h>

/*!
 * @brief       指定されたファイルポインタが示すファイルから8ビット整数を読み込みます。
 * @param file  ファイルポインタ
 * @return      読み込まれた値
 */
static inline uint8_t read_uint8(FILE* file) {
    uint8_t value;
    fread(&value, sizeof(uint8_t), 1, file);

    return value;
}

/*!
 * @brief       指定されたファイルポインタが示すファイルから16ビット整数を読み込みます。
 * @param file  ファイルポインタ
 * @return      読み込まれた値
 */
static inline uint16_t read_uint16(FILE* file) {
    uint16_t value;
    fread(&value, sizeof(uint16_t), 1, file);

    return value;
}

/*!
 * @brief       指定されたファイルポインタが示すファイルから32ビット整数を読み込みます。
 * @param file  ファイルポインタ
 * @return      読み込まれた値
 */
static inline uint32_t read_uint32(FILE* file) {
    uint32_t value;
    fread(&value, sizeof(uint32_t), 1, file);

    return value;
}

/*!
 * @brief       指定されたファイルポインタが示すファイルから16ビット符号付き整数を読み込みます。
 * @param file  ファイルポインタ
 * @return      読み込まれた値
 */
static inline int16_t read_int16(FILE* file) {
    int16_t value;
    fread(&value, sizeof(int16_t), 1, file);

    return value;
}

/*!
 * @brief       指定されたファイルポインタが示すファイルからASCII文字を読み込みます。
 * @param file  ファイルポインタ
 * @return      読み込まれた値
 */
static inline char read_char(FILE* file) {
    char value;
    fread(&value, sizeof(char), 1, file);

    return value;
}

/*!
 * @brief           指定されたハンドルのwave_file_readerが開いているファイルの読み込み位置を、指定されたASCII文字に一致するバイト列が出現する箇所に移動します。
 * @param *reader   wave_file_readerのハンドル
 * @param *data     ASCII文字
 * @param n         ASCII文字の文字数
 * @return          指定されたASCII文字に対応するバイト列が出現したかどうかを示す値
 */
static bool check_match_next_bytes(const wave_file_reader* reader, const char* data, uint32_t n) {
    register uint32_t i;
    register char read;

    for (i = 0; i < n; ++i) {
        read = read_char(reader->wave_file);

        if (read != data[i]) {
            return false;
        }
    }

    return true;
}

/*!
 * @brief                   指定されたハンドルのwave_file_readerが開いているファイルの読み込み位置を、指定されたチャンク名のチャンクの開始位置に移動します。
 * @param *reader           wave_file_readerのハンドル
 * @param *chunk_name       チャンク名
 * @param n                 チャンク名の文字数
 * @param find_from_begin   ファイルの最初からチャンクの探索を行うかどうかを示すフラグ
 * @return                  指定された名前のチャンクが見つかったかどうかを示す値
 */
static bool go_to_chunk(const wave_file_reader* reader, const char* chunk_name, uint32_t n, bool find_from_begin) {
    if (find_from_begin) {
        fseek(reader->wave_file, 0, SEEK_SET);
    }

    while (true) {
        if (check_match_next_bytes(reader, chunk_name, n)) {
            return true;
        }
    }

    return false;
}

/*!
 * @brief                   指定されたハンドルのwave_file_readerが開いているファイルからfmtチャンクを読み込みます。
 */
static void read_fmt_chunk(wave_file_reader* reader) {
    uint16_t audio_format;

    if (go_to_chunk(reader, "fmt ", 4, true)) {
        read_uint32(reader->wave_file);                                     /* チャンクサイズを読み飛ばす */
        audio_format = read_uint16(reader->wave_file);

        /* オーディオフォーマットがPCMか？ */
        if (audio_format == 0x0001 || audio_format == 0xFFFE) {
            reader->num_channels = read_uint16(reader->wave_file);          /* チャンネル数 */
            reader->sample_rate = read_uint32(reader->wave_file);           /* サンプリング周波数 */
            reader->avr_bytes_per_sec = read_uint32(reader->wave_file);     /* 1秒あたりの平均バイト数 */
            reader->block_size = read_uint16(reader->wave_file);            /* ブロックサイズ */
            reader->bits_per_sample = read_uint16(reader->wave_file);       /* 量子化ビット数 */
        }
    }
}

/*!
 * @brief           wave_file_readerのハンドルを生成します。
 * @path            読み込むWAVファイルのパス
 * @return          wave_file_readerのハンドル
 */
wave_file_reader* wave_file_reader_create(const char* path) {
    wave_file_reader* result = (wave_file_reader*)malloc(sizeof(wave_file_reader));
    
    if (result == NULL) {
        return NULL;
    }

    wave_file_reader_open(result, path);
    return result;
}

/*!
 * @brief           WAVファイルを開きます。
 * @param *reader   wave_file_readerのハンドル
 * @param *path     ファイルパス
 */
void wave_file_reader_open(wave_file_reader* reader, const char* path) {
    uint32_t size;
    errno_t err;

    err = fopen_s(&reader->wave_file, path, "rb");

    /* ファイルを開けなかった場合、エラーを報告して何もしない */
    if (err != 0) {
        return;
    }

    /* fmt チャンクを読み込む */
    read_fmt_chunk(reader);

    /* dataチャンクに移動 */
    if (go_to_chunk(reader, "data", 4, true)) {
        /* 合計サンプル数を計算 */
        size = read_uint32(reader->wave_file);
        reader->num_samples = size / (reader->bits_per_sample / 8);
    }
    else {
        reader->num_samples = 0;
    }
}

/*!
 * @brief           指定されたハンドルで開かれたWAVファイルを閉じます。
 * @param *reader   wave_file_readerのハンドル
 */
void wave_file_reader_close(const wave_file_reader* reader) {
    fclose(reader->wave_file);
}

/*!
 * @brief           指定されたハンドルで開かれたWAVファイルのサンプリング周波数を取得します。
 * @param *reader   wave_file_readerのハンドル
 * @return          サンプリング周波数
 */
uint32_t wave_file_reader_get_sample_rate(const wave_file_reader* reader) {
    return reader->sample_rate;
}

/*!
 * @brief           指定されたハンドルで開かれたWAVファイルの量子化ビット数を取得します。
 * @param *reader   wave_file_readerのハンドル
 * @return          量子化ビット数
 */
uint16_t wave_file_reader_get_bits_per_sample(const wave_file_reader* reader) {
    return reader->bits_per_sample;
}

/*!
 * @brief           指定されたハンドルで開かれたWAVファイルのチャンネル数を取得します。
 * @param *reader   wave_file_readerのハンドル
 * @return          チャンネル数
 */
uint16_t wave_file_reader_get_num_channels(const wave_file_reader* reader) {
    return reader->num_channels;
}

/*!
 * @brief           指定されたハンドルで開かれたWAVファイルの総サンプル数を取得します。
 * @param *reader   wave_file_readerのハンドル
 * @return          総サンプル数
 */
uint32_t wave_file_reader_get_num_samples(const wave_file_reader* reader) {
    return reader->num_samples;
}

/*!
 * @brief           指定されたハンドルで開かれたWAVファイルから次のサンプルを読み込みます。
 * @param *reader   wave_file_readerのハンドル
 * @return          サンプル
 */
int32_t wave_file_reader_read_sample(wave_file_reader* reader) {
    register int32_t result = 0;
    register uint8_t sb1, sb2;
    register int8_t sb3;

    switch (reader->bits_per_sample) {
    case 8:
        result = (int32_t)read_uint8(reader->wave_file) - 128;
        break;
    case 16:
        result = read_int16(reader->wave_file);
        break;
    case 24:
        sb1 = read_uint8(reader->wave_file);
        sb2 = read_uint8(reader->wave_file);
        sb3 = read_uint8(reader->wave_file);
        result = (sb3 << 16) | (sb2 << 8) | sb1;
        break;
    default:
        break;
    }

    reader->num_samples_read++;
    return result;
}