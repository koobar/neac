#include "neac_decoder.h"
#include "wave_file_writer.h"
#include <Windows.h>
#include <mmsystem.h>

#define PATH_SEPARATOR '\\'
#define PATH_SEPARATOR_STR "\\"

static void parse_command_line_args(int argc, char* argv[], char** input) {
    if (argc >= 2) {
        *input = argv[1];
    }
}

/*!
 * @brief           指定されたパスからディレクトリ名部分を取得します。
 * @param *path     パス
 * @return          ディレクトリ名部分
 */
static char* get_directory_name(const char* path) {
    size_t size = strlen(path), directory_name_end_offset = 0, directory_name_size, i;
    char* result;

    /* ディレクトリ名部分の終了位置を取得 */
    for (i = size - 1; i != 0; --i) {
        if (path[i] == PATH_SEPARATOR) {
            directory_name_end_offset = i;
            break;
        }
    }

    /* ディレクトリ名部分を格納する領域を確保 */
    directory_name_size = directory_name_end_offset + 1;
    result = (char*)malloc(directory_name_size);

    /* 確保できていなければNULLを返す */
    if (result == NULL) {
        return NULL;
    }

    /* ディレクトリ名部分をディレクトリ名領域にコピー */
    for (i = 0; i < directory_name_size - 1; ++i) {
        result[i] = path[i];
    }
    result[directory_name_size - 1] = '\0';

    return result;
}

/*!
 * @brief           指定されたパスからファイル名部分を取得します。
 * @param *path     パス
 * @return          ファイル名
 */
static char* get_file_name(const char* path) {
    char* file_name_ptr;
    char* result;
    size_t file_name_size;

    file_name_ptr = strrchr(path, PATH_SEPARATOR) + 1;
    file_name_size = strlen(file_name_ptr) + 1;

    result = (char*)malloc(sizeof(char) * file_name_size);
    snprintf(result, file_name_size, "%s", file_name_ptr);

    return result;
}
/*!
 * @brief           指定されたパスから、拡張子を除くファイル名部分を取得します。
 * @param *path     パス
 * @return          拡張子を除くファイル名
 */
static char* get_file_name_without_extension(const char* path) {
    char* name = get_file_name(path);
    size_t i;
    size_t len = strlen(name);

    for (i = len - 1; i != 0; --i) {
        if (name[i] == '.') {
            name[i] = '\0';
            break;
        }
    }

    return name;
}

/*!
 * @brief           デコードを行います。
 * @param input     入力ファイルのパス
 * @param output    出力ファイルのパス
 */
static void decode(const char* input, const char* output) {
    wave_file_writer* writer = NULL;
    neac_decoder* decoder = NULL;
    uint32_t i;

    /* 古いファイルを削除 */
    remove(output);

    /* デコードハンドルを作成して初期化 */
    decoder = neac_decoder_create(input);

    /* WAVEファイルを作成し、フォーマットとサンプル数を設定 */
    writer = wave_file_writer_create(output);
    wave_file_writer_set_pcm_format(writer, decoder->sample_rate, decoder->bits_per_sample, decoder->num_channels);
    wave_file_writer_set_num_samples(writer, decoder->num_total_samples);

    /* サンプル数を設定 */
    wave_file_writer_begin_write(writer);

    /* デコード処理 */
    for (i = 0; i < decoder->num_total_samples; ++i) {
        wave_file_writer_write_sample(writer, neac_decoder_read_sample(decoder));
    }
    wave_file_writer_end_write(writer);
    wave_file_writer_close(writer);
}

static void play(const char* input) {
    size_t buffer_size = sizeof(char) * MAX_PATH;
    char* output = (char*)malloc(buffer_size);

    if (output == NULL) {
        return;
    }

    printf("INPUT: %s \n", input);

    memset(output, '\0', buffer_size);

    char* dir = get_directory_name(input);
    char* name = get_file_name_without_extension(input);

    /* 出力先パスを作成 */
    strcat_s(output, buffer_size, dir);
    strcat_s(output, buffer_size, PATH_SEPARATOR_STR);
    strcat_s(output, buffer_size, name);
    strcat_s(output, buffer_size, "_decoded.wav");

    printf("Now Decoding...");

    /* デコード */
    decode(input, output);

    printf("Complete!\n");

    printf("PLAYING: %s\n", output);
    printf("To stop, press Ctrl+C.\n");

    if (!PlaySoundA(output, NULL, SND_SYNC)) {
        printf("[FATAL ERROR] Cannot play.\n");
    }

    system("pause");

    /* 後始末 */
    free(output);
}

int main(int argc, char* argv[]) {
    size_t buffer_size = sizeof(char) * MAX_PATH;
    char* input = (char*)malloc(buffer_size);

    if (argc >= 1) {
        parse_command_line_args(argc, argv, &input);
        play(input);
    }

    free(input);
}