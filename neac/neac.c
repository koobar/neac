#include <time.h>
#include <stdbool.h>
#include "neac.h"
#include "neac_decoder.h"
#include "neac_encoder.h"
#include "wave_file_reader.h"
#include "wave_file_writer.h"
#include "./include/path.h"

/*!
 * @brief							コマンドライン引数を解析します。
 * @param argc						コマンドライン引数の数
 * @param argv						コマンドライン引数
 * @param input_file_path			入力ファイルのパス
 * @param output_file_path			出力ファイルのパス
 * @param block_size				ブロックサイズ
 * @param filter_taps				LMSフィルタのタップ数
 * @param use_mid_side_stereo		ミッドサイドステレオを使用するかどうかを示すフラグ
 * @param disable_simple_predictor	シンプル予測器を無効化するかどうかを示すフラグ
 * @param is_silent_mode			サイレントモードを使用するかどうかを示すフラグ
 * @param is_help_mode				ヘルプモードを実行するかどうかを示すフラグ
 */
static void parse_commandline_args(
	int argc,
	char* argv[],
	char** input_file_path,
	char** output_file_path,
	uint16_t* block_size,
	uint8_t* filter_taps,
	bool* use_mid_side_stereo,
	bool* disable_simple_predictor,
	bool* is_silent_mode,
	bool* is_help_mode) {
	int i;
	/* 既定値を設定 */
	*block_size = 1024;
	*filter_taps = 4;
	*use_mid_side_stereo = false;
	*disable_simple_predictor = false;
	*is_silent_mode = false;
	*is_help_mode = false;
	
	if (argc <= 1) {
		*is_help_mode = 1;
		return;
	}

	for (i = 0; i < argc; ++i) {
		if (strcmp(argv[i], "-ms") == 0 || strcmp(argv[i], "-midside") == 0) {
			*use_mid_side_stereo = true;
		}
		else if (strcmp(argv[i], "-disable-simple-predictor") == 0) {
			*disable_simple_predictor = true;
		}
		else if (strcmp(argv[i], "--bs") == 0 || strcmp(argv[i], "--blocksize") == 0) {
			*block_size = atoi(argv[++i]);
		}
		else if (strcmp(argv[i], "--taps") == 0 || strcmp(argv[i], "--filter-taps") == 0) {
			*filter_taps = atoi(argv[i + 1]);
			++i;
		}
		else if (strcmp(argv[i], "--in") == 0 || strcmp(argv[i], "--input") == 0) {
			*input_file_path = argv[++i];
		}
		else if (strcmp(argv[i], "--out") == 0 || strcmp(argv[i], "--output") == 0) {
			*output_file_path = argv[++i];
		}
		else if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "-silent") == 0) {
			*is_silent_mode = true;
		}
		else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "-help") == 0) {
			*is_help_mode = true;
		}
	}
}

/*!
 * @brief					サイレントモードでない場合に限り、指定された文字列を出力します。
 * @param *str				出力する文字列
 * @param is_silent_mode	サイレントモードであるかどうかを示すフラグ
 */
static void print(const char* str, bool is_silent_mode) {
	if (!is_silent_mode) {
		printf("%s", str);
		printf("\n");
	}
}

/*!
 * @brief					サイレントモードでない場合に限り、区切り行を出力します。
 * @param is_silent_mode	サイレントモードであるかどうかを示すフラグ
 */
static void print_separator(bool is_silent_mode) {
	print("================================================================================", is_silent_mode);
}

/*!
 * @brief					サイレントモードでない場合に限り、改行を出力します。
 * @param is_silent_mode	サイレントモードであるかどうかを示すフラグ
 */
static void print_return(bool is_silent_mode) {
	print("", is_silent_mode);
}

/*!
 * @brief					サイレントモードでない場合に限り、ロゴを表示します。
 * @param is_silent_mode	サイレントモードであるかどうかを示すフラグ
 */
static void print_logo(bool is_silent_mode) {
	print("NEAC: NEko Audio Codec", is_silent_mode);
	print("Copyright (c) 2024 koobar.", is_silent_mode);
	print_return(is_silent_mode);
	print("Version: 0.1", is_silent_mode);
	print("Build:   2024/09/01", is_silent_mode);
	print_separator(is_silent_mode);
}

/*!
 * @brief 使い方の説明を表示します。
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
	printf("    -disable-simple-predictor   Disable 2-order simple predictor. Compression rates are often reduced.\n");
	printf("    -silent|-s                  Don't display any text.\n");
	printf("    -h|-help                    Display this text.\n");
}

/*!
 * @brief					サイレントモードでない場合に限り、エンコード結果の情報を表示します。
 * @param src				エンコード元ファイルのパス
 * @param output			エンコード先ファイルのパス
 * @param encode_start		エンコード開始時間
 * @param encode_end		エンコード終了時間
 * @param is_silent_mode	サイレントモードであるかどうかを示すフラグ
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

	sprintf_s(buffer, buffer_size, "Rate: %.2f\0", (double)dst_file_size / src_file_size);
	print(buffer, is_silent_mode);

	free(buffer);
}

/*!
 * @brief					サイレントモードでない場合に限り、NEACファイルのフォーマット情報を表示します。
 * @param *decoder			NEACデコーダのハンドル
 * @param is_silent_mode	サイレントモードであるかどうかを示すフラグ
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

	sprintf_s(buffer, buffer_size, "Simple Predictor:  %s\0", (decoder->disable_simple_predictor ? "False" : "True"));
	printf(buffer, is_silent_mode);

	free(buffer);
}

/*!
 * @brief					サイレントモードでない場合に限り、デコード結果の情報を表示します。
 * @param *decoder			デコーダのハンドル
 * @param output			出力先ファイルのパス
 * @param decode_start		デコード開始時間
 * @param decode_end		デコード終了時間
 * @param is_silent_mode	サイレントモードであるかどうかを示すフラグ
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
	
	/* フォーマット情報の表示 */
	print_format_info(decoder, is_silent_mode);
	
	free(buffer);
}

/*!
 * @brief							エンコードを行います。
 * @param input						入力ファイルのパス
 * @param output					出力ファイルのパス
 * @param block_size				ブロックサイズ
 * @param use_mid_side_stereo		ミッドサイドステレオを使用するかどうかを示すフラグ
 * @param disable_simple_predictor	シンプル予測器を無効化するかどうかを示すフラグ
 * @param filter_taps				LMSフィルタのタップ数
 * @param is_silent_mode			サイレントモードであるかどうかを示すフラグ
 */
static void encode(
	const char* input, 
	const char* output, 
	uint16_t block_size, 
	bool use_mid_side_stereo, 
	bool disable_simple_predictor, 
	uint8_t filter_taps, 
	bool is_silent_mode) {
	wave_file_reader* reader = NULL;
	neac_encoder* encoder = NULL;
	uint32_t n, i;
	clock_t start, end;

	/* 古いファイルを削除 */
	remove(output);

	/* エンコード元のWAVファイルを開く */
	reader = wave_file_reader_create(input);

	/* エンコードするファイルのサンプル数を取得 */
	n = wave_file_reader_get_num_samples(reader);

	/* エンコードハンドルを作成して初期化 */
	encoder = neac_encoder_create_from_path(
		output,
		wave_file_reader_get_sample_rate(reader),
		(uint8_t)wave_file_reader_get_bits_per_sample(reader),
		(uint8_t)wave_file_reader_get_num_channels(reader),
		n,
		block_size,
		use_mid_side_stereo,
		disable_simple_predictor,
		filter_taps);

	/* エンコード所要時間の計測開始 */
	start = clock();

	/* エンコード処理 */
	for (i = 0; i < n; ++i) {
		neac_encoder_write_sample(encoder, wave_file_reader_read_sample(reader));
	}
	neac_encoder_end_write(encoder);
	wave_file_reader_close(reader);

	/* エンコード所要時間の計測終了*/
	end = clock();

	/* エンコード結果の表示 */
	print_encode_result(input, output, start, end, is_silent_mode);
	print_return(is_silent_mode);

	/* 後始末 */
	neac_encoder_free(encoder);
}

/*!
 * @brief					デコードを行います。
 * @param input				入力ファイルのパス
 * @param output			出力ファイルのパス
 * @param is_silent_mode	サイレントモードであるかどうかを示すフラグ
 */
static void decode(const char* input, const char* output, bool is_silent_mode) {
	wave_file_writer* writer = NULL;
	neac_decoder* decoder = NULL;
	clock_t start, end;
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

	/* デコード所要時間の計測開始 */
	start = clock();

	/* デコード処理 */
	for (i = 0; i < decoder->num_total_samples; ++i) {
		wave_file_writer_write_sample(writer, neac_decoder_read_sample(decoder));
	}
	wave_file_writer_end_write(writer);
	wave_file_writer_close(writer);

	/* デコード所要時間の計測終了 */
	end = clock();

	/* デコード結果の表示 */
	print_decode_result(decoder, output, start, end, is_silent_mode);
	print_return(is_silent_mode);

	/* 後始末 */
	neac_decoder_free(decoder);
}

int main(int argc, char* argv[]) {
	char* input = NULL;
	char* output = NULL;
	uint16_t block_size;
	uint8_t filter_taps;
	size_t buffer_size;
	bool use_mid_side_stereo;
	bool disable_simple_predictor;
	bool is_silent_mode;
	bool is_help_mode;

	/* コマンドライン引数を解析 */
	parse_commandline_args(argc, argv, &input, &output, &block_size, &filter_taps, &use_mid_side_stereo, &disable_simple_predictor, &is_silent_mode, &is_help_mode);

	/* ロゴを表示 */
	print_logo(is_silent_mode);

	if (is_help_mode) {
		print_usage();
	}
	else {
		char* extension = get_extension(input);

		if (strcmp(extension, ".wav") == 0) {
			if (output == NULL) {
				buffer_size = sizeof(char) * MAX_PATH;
				output = (char*)malloc(buffer_size);

				if (output != NULL) {
					for (size_t i = 0; i < buffer_size; ++i) {
						output[i] = '\0';
					}

					strcpy_s(output, buffer_size, input);
					change_extension(output, ".neac");
				}
			}

			/* エンコード */
			encode(input, output, block_size, use_mid_side_stereo, disable_simple_predictor, filter_taps, is_silent_mode);
		}
		else if (strcmp(extension, ".neac") == 0) {
			if (output == NULL) {
				buffer_size = sizeof(char) * MAX_PATH;
				output = (char*)malloc(buffer_size);

				if (output != NULL) {
					for (size_t i = 0; i < buffer_size; ++i) {
						output[i] = '\0';
					}

					char* dir = get_directory_name(input);
					char* name = get_file_name_without_extension(input);

					/* 出力先パスを作成 */
					strcat_s(output, buffer_size, dir);
					strcat_s(output, buffer_size, PATH_SEPARATOR_STR);
					strcat_s(output, buffer_size, name);
					strcat_s(output, buffer_size, "_decoded.wav\0");

					free(dir);
					free(name);
				}
			}

			/* デコード */
			decode(input, output, is_silent_mode);
		}
	}

	free(input);
	free(output);
}