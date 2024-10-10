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
 * @brief                   サイレントモードでない場合に限り、指定された文字列を出力します。
 * @param *str              出力する文字列
 * @param is_silent_mode    サイレントモードであるかどうかを示すフラグ
 */
static void print(const char* str, bool is_silent_mode) {
    if (!is_silent_mode) {
        printf("%s", str);
        printf("\n");
    }
}

/*!
 * @brief                   サイレントモードでない場合に限り、改行を出力します。
 * @param is_silent_mode    サイレントモードであるかどうかを示すフラグ
 */
static void print_return(bool is_silent_mode) {
    print("", is_silent_mode);
}

static void print_tag(neac_tag* tag, bool is_silent_mode) {
    size_t buffer_size = sizeof(char) * 1024;
    char* buffer = (char*)malloc(buffer_size);

    if (buffer == NULL) {
        return;
    }

    if (tag == NULL) {
        free(buffer);
        return;
    }

    sprintf_s(buffer, buffer_size, "[TAG INFORMATION]\0");
    print(buffer, is_silent_mode);

    sprintf_s(buffer, buffer_size, "Title:        %s\0", tag->title);
    print(buffer, is_silent_mode);

    sprintf_s(buffer, buffer_size, "Album:        %s\0", tag->album);
    print(buffer, is_silent_mode);

    sprintf_s(buffer, buffer_size, "Artist:       %s\0", tag->artist);
    print(buffer, is_silent_mode);

    sprintf_s(buffer, buffer_size, "Album Artist: %s\0", tag->album_artist);
    print(buffer, is_silent_mode);

    sprintf_s(buffer, buffer_size, "Subtitle:     %s\0", tag->subtitle);
    print(buffer, is_silent_mode);

    sprintf_s(buffer, buffer_size, "Publisher:    %s\0", tag->publisher);
    print(buffer, is_silent_mode);

    sprintf_s(buffer, buffer_size, "Composer:     %s\0", tag->composer);
    print(buffer, is_silent_mode);

    sprintf_s(buffer, buffer_size, "Songwriter:   %s\0", tag->songwriter);
    print(buffer, is_silent_mode);

    sprintf_s(buffer, buffer_size, "Conductor:    %s\0", tag->conductor);
    print(buffer, is_silent_mode);

    sprintf_s(buffer, buffer_size, "Copyright:    %s\0", tag->copyright);
    print(buffer, is_silent_mode);

    sprintf_s(buffer, buffer_size, "Comment:      %s\0", tag->comment);
    print(buffer, is_silent_mode);

    sprintf_s(buffer, buffer_size, "Genre:        %s\0", tag->genre);
    print(buffer, is_silent_mode);

    sprintf_s(buffer, buffer_size, "Year:         %d\0", tag->year);
    print(buffer, is_silent_mode);

    sprintf_s(buffer, buffer_size, "Track number: %d\0", tag->track_number);
    print(buffer, is_silent_mode);

    sprintf_s(buffer, buffer_size, "Track count:  %d\0", tag->track_count);
    print(buffer, is_silent_mode);

    sprintf_s(buffer, buffer_size, "Disc number:  %d\0", tag->disc);
    print(buffer, is_silent_mode);

    sprintf_s(buffer, buffer_size, "Rate:         %d\0", tag->rate);
    print(buffer, is_silent_mode);

    free(buffer);
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
    
    printf("Now Decoding...");

    /* デコード処理 */
    for (i = 0; i < decoder->num_total_samples; ++i) {
        wave_file_writer_write_sample(writer, neac_decoder_read_sample(decoder));
    }
    wave_file_writer_end_write(writer);
    wave_file_writer_close(writer);

    printf("Decoding process completed successfully!\n");
    printf("\n");

    if (decoder->tag != NULL) {
        print_tag(decoder->tag, false);
        print_return(false);
        print_return(false);
    }

    neac_decoder_free(decoder);
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

    /* デコード */
    decode(input, output);

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