#ifndef NEAC_ENCODER_H
#define NEAC_ENCODER_H

#include "bit_stream.h"
#include "file_access.h"
#include "lms.h"
#include "neac.h"
#include "neac_block.h"
#include "neac_code.h"
#include "neac_error.h"
#include "neac_sub_block.h"
#include "signal.h"
#include "simple_predictor.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

/*!
 * @brief エンコーダ
 */
typedef struct neac_encoder {
	FILE* output_file;
	bit_stream* output_bit_stream;

	uint32_t sample_rate;
	uint8_t bits_per_sample;
	uint8_t num_channels;
	uint32_t num_samples;

	uint8_t filter_taps;
	uint16_t block_size;
	bool use_mid_side_stereo;
	bool disable_simple_predictor;
	uint32_t num_blocks;

	lms** lms_filters;
	simple_predictor** simple_predictors;

	neac_code* coder;
	neac_block* current_block;
	uint8_t current_sub_block_channel;
	uint16_t current_sub_block_offset;
} neac_encoder;

/*!
 * @brief							指定された設定で、エンコーダのハンドルを生成します。
 * @param file						出力先のファイルハンドル
 * @param sample_rate				サンプリング周波数
 * @param bits_per_sample			量子化ビット数
 * @param num_channels				チャンネル数
 * @param num_samples				合計サンプル数
 * @param block_size				ブロックサイズ
 * @param use_mid_side_stereo		ミッドサイドステレオを使用するかどうかを示すフラグ
 * @param disable_simple_predictor	シンプル予測器を無効化するかどうかを示すフラグ
 * @param filter_taps				LMSフィルタのタップ数
 */
neac_encoder* neac_encoder_create(
	FILE* file,
	uint32_t sample_rate,
	uint8_t bits_per_sample,
	uint8_t num_channels,
	uint32_t num_samples,
	uint16_t block_size,
	bool use_mid_side_stereo,
	bool disable_simple_predictor,
	uint8_t filter_taps);

/*!
 * @brief							指定された設定で、エンコーダのハンドルを生成します。
 * @param path						出力先のパス
 * @param sample_rate				サンプリング周波数
 * @param bits_per_sample			量子化ビット数
 * @param num_channels				チャンネル数
 * @param num_samples				合計サンプル数
 * @param block_size				ブロックサイズ
 * @param use_mid_side_stereo		ミッドサイドステレオを使用するかどうかを示すフラグ
 * @param disable_simple_predictor	シンプル予測器を無効化するかどうかを示すフラグ
 * @param filter_taps				LMSフィルタのタップ数
 */
neac_encoder* neac_encoder_create_from_path(
	const char* path,
	uint32_t sample_rate,
	uint8_t bits_per_sample,
	uint8_t num_channels,
	uint32_t num_samples,
	uint16_t block_size,
	bool use_mid_side_stereo,
	bool disable_simple_predictor,
	uint8_t filter_taps);

/*!
 * @brief			エンコーダを解放します。
 * @param encoder	エンコーダのハンドル
 */
void neac_encoder_free(neac_encoder* encoder);

/*!
 * @brief			指定されたハンドルのエンコーダで、指定されたサンプルをエンコードします。
 * @param *encoder	エンコーダのハンドル
 * @param sample	サンプル
 */
void neac_encoder_write_sample(neac_encoder* encoder, int32_t sample);

/*!
 * @brief			指定されたハンドルのエンコーダでのエンコードの終了処理を行います。すべてのサンプルのエンコードが終了した後に、必ず呼び出してください。
 * @param *encoder	エンコーダのハンドル
 */
void neac_encoder_end_write(neac_encoder* encoder);

#endif