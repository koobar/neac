#include "./include/path.h"
#include "neac.h"
#include "neac_decoder.h"
#include "neac_encoder.h"
#include "wave_file_reader.h"
#include "wave_file_writer.h"
#include <stdbool.h>
#include <time.h>

static char* input_file_path = NULL;
static char* output_file_path = NULL;
static bool use_mid_side_stereo = false;
static bool is_silent_mode = false;
static bool is_help_mode = false;
static uint16_t block_size = 1024;
static uint8_t filter_taps = 4;

static const char* tag_title;
static const char* tag_album;
static const char* tag_artist;
static const char* tag_album_artist;
static const char* tag_subtitle;
static const char* tag_publisher;
static const char* tag_composer;
static const char* tag_songwriter;
static const char* tag_conductor;
static const char* tag_copyright;
static const char* tag_genre;
static const char* tag_comment;
static uint16_t tag_year;
static uint16_t tag_track_number;
static uint16_t tag_track_count;
static uint16_t tag_disc;
static uint16_t tag_rate;
static const char* tag_picture_path;

static bool has_tag = false;

/*!
 * @brief               コマンドライン引数を解析します。
 * @param argc          引数の数
 * @param argv          引数
 */
static void parse_commandline_args(int argc, char* argv[]) {
    int i;

    for (i = 0; i < argc; ++i) {
        if (strcmp(argv[i], "-ms") == 0 || strcmp(argv[i], "-midside") == 0) {
            use_mid_side_stereo = true;
        }
        else if (strcmp(argv[i], "--bs") == 0 || strcmp(argv[i], "--blocksize") == 0) {
            block_size = atoi(argv[++i]);
        }
        else if (strcmp(argv[i], "--taps") == 0 || strcmp(argv[i], "--filter-taps") == 0) {
            filter_taps = atoi(argv[i + 1]);
            ++i;
        }
        else if (strcmp(argv[i], "--in") == 0 || strcmp(argv[i], "--input") == 0) {
            input_file_path = argv[++i];
        }
        else if (strcmp(argv[i], "--out") == 0 || strcmp(argv[i], "--output") == 0) {
            output_file_path = argv[++i];
        }
        else if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "-silent") == 0) {
            is_silent_mode = true;
        }
        else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "-help") == 0) {
            is_help_mode = true;
        }
        else if (strcmp(argv[i], "--title") == 0) {
            tag_title = argv[i + 1];
            ++i;
            has_tag = true;
        }
        else if (strcmp(argv[i], "--album") == 0) {
            tag_album = argv[i + 1];
            ++i;
            has_tag = true;
        }
        else if (strcmp(argv[i], "--artist") == 0) {
            tag_artist = argv[i + 1];
            ++i;
            has_tag = true;
        }
        else if (strcmp(argv[i], "--album-artist") == 0) {
            tag_album_artist = argv[i + 1];
            ++i;
            has_tag = true;
        }
        else if (strcmp(argv[i], "--subtitle") == 0) {
            tag_subtitle = argv[i + 1];
            ++i;
            has_tag = true;
        }
        else if (strcmp(argv[i], "--publisher") == 0) {
            tag_publisher = argv[i + 1];
            ++i;
            has_tag = true;
        }
        else if (strcmp(argv[i], "--composer") == 0) {
            tag_composer = argv[i + 1];
            ++i;
            has_tag = true;
        }
        else if (strcmp(argv[i], "--songwriter") == 0) {
            tag_songwriter = argv[i + 1];
            ++i;
            has_tag = true;
        }
        else if (strcmp(argv[i], "--conductor") == 0) {
            tag_conductor = argv[i + 1];
            ++i;
            has_tag = true;
        }
        else if (strcmp(argv[i], "--copyright") == 0) {
            tag_copyright = argv[i + 1];
            ++i;
            has_tag = true;
        }
        else if (strcmp(argv[i], "--genre") == 0) {
            tag_genre = argv[i + 1];
            ++i;
            has_tag = true;
        }
        else if (strcmp(argv[i], "--year") == 0) {
            tag_year = (uint16_t)atoi(argv[i + 1]);
            ++i;
            has_tag = true;
        }
        else if (strcmp(argv[i], "--track-number") == 0) {
            tag_track_number = (uint16_t)atoi(argv[i + 1]);
            ++i;
            has_tag = true;
        }
        else if (strcmp(argv[i], "--track-count") == 0) {
            tag_track_count = (uint16_t)atoi(argv[i + 1]);
            ++i;
            has_tag = true;
        }
        else if (strcmp(argv[i], "--disc") == 0) {
            tag_disc = (uint16_t)atoi(argv[i + 1]);
            ++i;
            has_tag = true;
        }
        else if (strcmp(argv[i], "--rate") == 0) {
            tag_rate = (uint16_t)atoi(argv[i + 1]);
            ++i;
            has_tag = true;
        }
        else if (strcmp(argv[i], "--picture") == 0) {
            tag_picture_path = argv[i + 1];
            ++i;
            has_tag = true;
        }
    }
}

/*!
 * @brief                   メッセージを出力します。
 * @param *str              出力するメッセージ
 * @param is_silent_mode    サイレントモード指定
 */
static void print(const char* str, bool is_silent_mode) {
    if (!is_silent_mode) {
        printf("%s", str);
        printf("\n");
    }
}

/*!
 * @brief                   行セパレータを出力します。
 * @param is_silent_mode    サイレントモード指定
 */
static void print_separator(bool is_silent_mode) {
    print("================================================================================", is_silent_mode);
}

/*!
 * @brief                   改行を出力します。
 * @param is_silent_mode    サイレントモード指定
 */
static void print_return(bool is_silent_mode) {
    print("", is_silent_mode);
}

/*!
 * @brief                   著作権表示等を出力します。
 * @param is_silent_mode    サイレントモード指定
 */
static void print_logo(bool is_silent_mode) {
    print("NEAC: NEko Audio Codec - Reference codec implementation.", is_silent_mode);
    print("Copyright (c) 2024 koobar. Released under WTFPL version 2.", is_silent_mode);
    print_return(is_silent_mode);
    print("Codec version:   1.0", is_silent_mode);
    print("Build:           2024/11/01", is_silent_mode);
    print_separator(is_silent_mode);
}

/*!
 * @brief       ヘルプメッセージを表示します。
 */
static void print_usage() {
    printf("Usage:      neac [options]\n");
    printf("Example:    neac --bs 1024 -ms --in <input> --out <output>\n");
    printf("\n");
    printf("Options:\n");
    printf("    --bs|--blocksize            Specify the number of samples per block. (default = 1024)\n");
    printf("    --taps|--filter-taps        Specify the LMS adaptive filter taps between 1 and 32.\n");
    printf("    --in|--input                Specify the input file path.\n");
    printf("    --out|--output              Specify the output file path.\n");
    printf("    -ms|-midside                Uses mid-side stereo. Compression rates are often improved.\n");
    printf("    -silent|-s                  Don't display any text.\n");
    printf("    --title                     Set the title in tag information.\n");
    printf("    --album                     Set the album in tag information.\n");
    printf("    --artist                    Set the artist in tag information.\n");
    printf("    --album-artist              Set the album artist in tag information.\n");
    printf("    --subtitle                  Set the subtitle in tag information.\n");
    printf("    --publisher                 Set the publisher in tag information.\n");
    printf("    --composer                  Set the composer in tag information.\n");
    printf("    --songwriter                Set the songwriter in tag information.\n");
    printf("    --conductor                 Set the conductor in tag information.\n");
    printf("    --copyright                 Set the copyright notices in tag information.\n");
    printf("    --comment                   Set the comment in tag information.\n");
    printf("    --genre                     Set the genre in tag information.\n");
    printf("    --year                      Set the year in tag information.\n");
    printf("    --track-number              Set the track number in tag information.\n");
    printf("    --track-count               Set the track count in tag information.\n");
    printf("    --disc                      Set the disc number in tag information.\n");
    printf("    --rate                      Set the rate in tag information.\n");
    printf("    --picture                   Set the picture such as cover, album art, thumbnail in tag information.\n");
    printf("    -h|-help                    Display this text.\n");
}

/*!
 * @brief                   エンコード結果を出力します。
 * @param src               エンコード前のファイルのパス
 * @param output            エンコード後のファイルのパス
 * @param encode_start      エンコード開始時間
 * @param encode_end        エンコード終了時間
 * @param is_silent_mode    サイレントモード指定
 */
static void print_encode_result(const char* src, const char* output, clock_t encode_start, clock_t encode_end, bool is_silent_mode) {
    size_t buffer_size = sizeof(char) * 1024;
    char* buffer = (char*)malloc(buffer_size);
    fpos_t src_file_size, dst_file_size;

    if (buffer == NULL) {
        return;
    }

    print("Encode completed.", is_silent_mode);

    sprintf_s(buffer, buffer_size, "Output: %s\0", output);
    print(buffer, is_silent_mode);

    print_return(is_silent_mode);

    sprintf_s(buffer, buffer_size, "[RESULT]\0");
    print(buffer, is_silent_mode);

    sprintf_s(buffer, buffer_size, "Elapsed time: %d msec.\0", encode_end - encode_start);
    print(buffer, is_silent_mode);

    src_file_size = get_file_size(src);
    dst_file_size = get_file_size(output);

    sprintf_s(buffer, buffer_size, "Source file size: %.2f MiB\0", (src_file_size / 1048576.0));
    print(buffer, is_silent_mode);

    sprintf_s(buffer, buffer_size, "Destination file size: %.2f MiB\0", (dst_file_size / 1048576.0));
    print(buffer, is_silent_mode);

    sprintf_s(buffer, buffer_size, "Compression rate: %.2f%% of the original file size.\0", ((double)dst_file_size / src_file_size) * 100.0);
    print(buffer, is_silent_mode);

    free(buffer);
}

/*!
 * @brief                   フォーマット情報を出力します。
 * @param *decoder          デコーダのハンドル
 * @param is_silent_mode    サイレントモード指定
 */
static void print_format_info(neac_decoder* decoder, bool is_silent_mode) {
    size_t buffer_size = sizeof(char) * 1024;
    char* buffer = (char*)malloc(buffer_size);

    if (buffer == NULL) {
        return;
    }

    sprintf_s(buffer, buffer_size, "[FORMAT]\0");
    print(buffer, is_silent_mode);

    sprintf_s(buffer, buffer_size, "Sample Rate:       %d Hz\0", decoder->sample_rate);
    print(buffer, is_silent_mode);

    sprintf_s(buffer, buffer_size, "Bits Per Sample:   %d Bits\0", decoder->bits_per_sample);
    print(buffer, is_silent_mode);

    sprintf_s(buffer, buffer_size, "Channels:          %d Channels\0", decoder->num_channels);
    print(buffer, is_silent_mode);

    sprintf_s(buffer, buffer_size, "Block Size:        %d Samples\0", decoder->block_size);
    print(buffer, is_silent_mode);

    sprintf_s(buffer, buffer_size, "Total Samples:     %d Samples\0", decoder->num_total_samples);
    print(buffer, is_silent_mode);

    sprintf_s(buffer, buffer_size, "Total Blocks:      %d Blocks\0", decoder->num_blocks);
    print(buffer, is_silent_mode);

    free(buffer);
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
 * @brief                   デコード結果を出力します。
 * @param *decoder          デコーダのハンドル
 * @param output            出力先パス
 * @param decode_start      デコード開始時間
 * @param decode_end        デコード終了時間
 * @param is_silent_mode    サイレントモード指定
 */
static void print_decode_result(neac_decoder* decoder, const char* output, clock_t decode_start, clock_t decode_end, bool is_silent_mode) {
    size_t buffer_size = sizeof(char) * 1024;
    char* buffer = (char*)malloc(buffer_size);

    if (buffer == NULL) {
        return;
    }

    print("Decode completed.", is_silent_mode);
    print_return(is_silent_mode);

    sprintf_s(buffer, buffer_size, "[RESULT]\0");
    print(buffer, is_silent_mode);

    sprintf_s(buffer, buffer_size, "Output: %s\0", output);
    print(buffer, is_silent_mode);

    sprintf_s(buffer, buffer_size, "Elapsed time: %d msec.\0", decode_end - decode_start);
    print(buffer, is_silent_mode);

    print_return(is_silent_mode);
    
    /* フォーマット情報を出力 */
    print_format_info(decoder, is_silent_mode);

    /* タグ情報を出力 */
    print_tag(decoder->tag, is_silent_mode);
    
    free(buffer);
}

/*!
 * @brief                           エンコード処理を行います
 * @param input                     入力ファイル
 * @param output                    出力ファイル
 * @param block_size                ブロックサイズ
 * @param use_mid_side_stereo       ミッドサイドステレオを使うかどうか
 * @param filter_taps               LMSフィルタのタップ数
 * @param is_silent_mode            サイレントモード指定
 */
static void encode(
    const char* input, 
    const char* output, 
    uint16_t block_size, 
    bool use_mid_side_stereo, 
    uint8_t filter_taps, 
    bool is_silent_mode) {
    wave_file_reader* reader = NULL;
    neac_encoder* encoder = NULL;
    register uint32_t n, i;
    clock_t start, end;
    neac_tag* tag;

    /* 古いファイルがあれば削除 */
    remove(output);

    /* WAVファイルデコーダを作成 */
    reader = wave_file_reader_create(input);

    /* WAVファイルに含まれるサンプル数を取得 */
    n = wave_file_reader_get_num_samples(reader);

    if (has_tag) {
        tag = (neac_tag*)malloc(sizeof(neac_tag));
        neac_tag_init(tag);
        if (tag_title != NULL) tag->title = tag_title;
        if (tag_album != NULL) tag->album = tag_album;
        if (tag_artist != NULL) tag->artist = tag_artist;
        if (tag_album_artist != NULL) tag->album_artist = tag_album_artist;
        if (tag_subtitle != NULL) tag->subtitle = tag_subtitle;
        if (tag_publisher != NULL) tag->publisher = tag_publisher;
        if (tag_composer != NULL) tag->composer = tag_composer;
        if (tag_songwriter != NULL) tag->songwriter = tag_songwriter;
        if (tag_conductor != NULL) tag->conductor = tag_conductor;
        if (tag_copyright != NULL) tag->copyright = tag_copyright;
        if (tag_genre != NULL) tag->genre = tag_genre;
        if (tag_comment != NULL) tag->comment = tag_comment;
        tag->year = tag_year;
        tag->track_number = tag_track_number;
        tag->track_count = tag_track_count;
        tag->disc = tag_disc;
        tag->rate = tag_rate;

        if (tag_picture_path != NULL) {
            neac_tag_set_picture_from_path(tag, tag_picture_path);
        }
    }
    else {
        tag = NULL;
    }

    /* NEACエンコーダを作成 */
    encoder = neac_encoder_create_from_path(
        output,
        wave_file_reader_get_sample_rate(reader),
        (uint8_t)wave_file_reader_get_bits_per_sample(reader),
        (uint8_t)wave_file_reader_get_num_channels(reader),
        n,
        block_size,
        use_mid_side_stereo,
        filter_taps,
        tag);

    /* エンコード開始時間を記録 */
    start = clock();

    /* すべてのサンプルをエンコード */
    for (i = 0; i < n; ++i) {
        neac_encoder_write_sample(encoder, wave_file_reader_read_sample(reader));
    }
    neac_encoder_end_write(encoder);
    wave_file_reader_close(reader);

    /* エンコード終了時間を記録 */
    end = clock();

    /* エンコード結果を出力 */
    print_encode_result(input, output, start, end, is_silent_mode);

    /* 後始末 */
    neac_encoder_free(encoder);
}

/*!
 * @brief                   デコード処理を行います。
 * @param input             入力ファイル
 * @param output            出力ファイル
 * @param is_silent_mode    サイレントモード指定
 */
static void decode(const char* input, const char* output, bool is_silent_mode) {
    wave_file_writer* writer = NULL;
    neac_decoder* decoder = NULL;
    clock_t start, end;
    register uint32_t i;

    /* 古いファイルがあれば削除 */
    remove(output);

    /* デコーダのハンドルを作成 */
    decoder = neac_decoder_create(input);

    /* WAVEファイルエンコーダを作成 */
    writer = wave_file_writer_create(output);
    wave_file_writer_set_pcm_format(writer, decoder->sample_rate, decoder->bits_per_sample, decoder->num_channels);
    wave_file_writer_set_num_samples(writer, decoder->num_total_samples);

    /* WAVEファイルへの書き込み開始 */
    wave_file_writer_begin_write(writer);

    /* デコード開始時間を記録 */
    start = clock();

    /* すべてのサンプルをデコード */
    for (i = 0; i < decoder->num_total_samples; ++i) {
        wave_file_writer_write_sample(writer, neac_decoder_read_sample(decoder));
    }
    wave_file_writer_end_write(writer);
    wave_file_writer_close(writer);

    /* デコード終了時間を記録 */
    end = clock();

    /* デコード結果を出力 */
    print_decode_result(decoder, output, start, end, is_silent_mode);

    /* 後始末 */
    neac_decoder_free(decoder);
}

int main(int argc, char* argv[]) {
    size_t buffer_size, i;
    char* extension = NULL;
    char* dir = NULL;
    char* name = NULL;

    /* コマンドライン引数を解析 */
    parse_commandline_args(argc, argv);

    /* ロゴを表示 */
    print_logo(is_silent_mode);

    if (is_help_mode) {
        print_usage();
    }
    else {
        extension = get_extension(input_file_path);

        if (strcmp(extension, ".wav") == 0) {
            if (output_file_path == NULL) {
                buffer_size = sizeof(char) * MAX_PATH;
                output_file_path = (char*)malloc(buffer_size);

                if (output_file_path != NULL) {
                    for (i = 0; i < buffer_size; ++i) {
                        output_file_path[i] = '\0';
                    }

                    strcpy_s(output_file_path, buffer_size, input_file_path);
                    change_extension(output_file_path, ".neac");
                }
            }

            /* エンコード */
            encode(input_file_path, output_file_path, block_size, use_mid_side_stereo, filter_taps, is_silent_mode);
        }
        else if (strcmp(extension, ".neac") == 0) {
            if (output_file_path == NULL) {
                buffer_size = sizeof(char) * MAX_PATH;
                output_file_path = (char*)malloc(buffer_size);

                if (output_file_path != NULL) {
                    for (i = 0; i < buffer_size; ++i) {
                        output_file_path[i] = '\0';
                    }

                    dir = get_directory_name(input_file_path);
                    name = get_file_name_without_extension(input_file_path);

                    /* 保存先パスを作成 */
                    strcat_s(output_file_path, buffer_size, dir);
                    strcat_s(output_file_path, buffer_size, PATH_SEPARATOR_STR);
                    strcat_s(output_file_path, buffer_size, name);
                    strcat_s(output_file_path, buffer_size, "_decoded.wav\0");

                    free(dir);
                    free(name);
                }
            }

            /* デコード */
            decode(input_file_path, output_file_path, is_silent_mode);
        }
    }

    free(input_file_path);
    free(output_file_path);
}