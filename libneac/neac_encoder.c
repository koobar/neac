#include "./include/neac_encoder.h"

#pragma region データの書き込み

/*!
 * @brief			NEACのヘッダ部の書き込みを行います。
 * @param *encoder	エンコーダのハンドル
 */
static void write_header(neac_encoder* encoder) {
	/* マジックナンバーを書き込む。 */
	write_char(encoder->output_file, 'N');
	write_char(encoder->output_file, 'E');
	write_char(encoder->output_file, 'A');
	write_char(encoder->output_file, 'C');

	/* フォーマットのバージョンを書き込む。 */
	write_uint8(encoder->output_file, NEAC_ENCODER_FORMAT_VERSION);

	/* PCMフォーマット情報を書き込む。 */
	write_uint32(encoder->output_file, encoder->sample_rate);
	write_uint8(encoder->output_file, encoder->bits_per_sample);
	write_uint8(encoder->output_file, encoder->num_channels);
	write_uint32(encoder->output_file, encoder->num_samples);

	/* NEACエンコード情報 */
	write_uint8(encoder->output_file, encoder->filter_taps);
	write_uint16(encoder->output_file, encoder->block_size);
	write_bool(encoder->output_file, encoder->use_mid_side_stereo);
	write_bool(encoder->output_file, encoder->disable_simple_predictor);
	write_uint32(encoder->output_file, encoder->num_blocks);
}

#pragma endregion

#pragma region 信号処理

/*!
 * @brief				指定されたシンプル予測器を用いて、指定されたサブブロックを予測符号化します。
 * @param *predictor	シンプル予測器のハンドル
 * @param *sub_block	サブブロックのハンドル
 * @param block_size	ブロックサイズ
 */
static void pass_simple_predictor(simple_predictor* predictor, neac_sub_block* sub_block, uint16_t block_size) {
	uint16_t offset;
	signal sample, prediction, residual;

	for (offset = 0; offset < block_size; ++offset) {
		sample = sub_block->samples[offset];
		prediction = simple_predictor_predict(predictor);
		residual = sample - prediction;

		simple_predictor_update(predictor, sample);
		sub_block->samples[offset] = residual;
	}
}

/*!
 * @brief				指定されたLMSフィルタを用いて、指定されたサブブロックを予測符号化します。
 * @param *filter		LMSフィルタのハンドル
 * @param *sub_block	サブブロックのハンドル
 * @param block_size	ブロックサイズ
 */
static void pass_lms_filter(lms* filter, neac_sub_block* sub_block, uint16_t block_size) {
	uint16_t offset;
	signal sample, prediction, residual;

	for (offset = 0; offset < block_size; ++offset) {
		sample = sub_block->samples[offset];
		prediction = lms_predict(filter);
		residual = sample - prediction;

		lms_update(filter, sample, residual);
		sub_block->samples[offset] = residual;
	}
}

/*!
 * @brief				指定されたブロックがステレオである場合に限り、ミッドサイドステレオに変換します。
 * @param *block		ブロックのハンドル
 * @param block_size	ブロックサイズ
 */
static void lr_to_ms_conversion(neac_block* block, uint16_t block_size) {
	neac_sub_block* ch0 = NULL;
	neac_sub_block* ch1 = NULL;
	uint16_t offset;
	signal left, right, mid, side;

	if (block->num_channels != 2) {
		return;
	}

	ch0 = block->sub_blocks[0];
	ch1 = block->sub_blocks[1];

	for (offset = 0; offset < block_size; ++offset) {
		left = ch0->samples[offset];
		right = ch1->samples[offset];
		mid = (left + right) >> 1;		/* mid = (int)((left + right) / 2); */
		side = left - right;

		ch0->samples[offset] = mid;
		ch1->samples[offset] = side;
	}
}

#pragma endregion

#pragma region エンコード処理

/*!
 * @brief			指定されたハンドルのエンコーダでエンコード中のブロックにおける、指定されたチャンネルに対応するサブブロックのエンコードを行います。
 * @param *encoder	エンコーダのハンドル
 * @param channel	サブブロックのチャンネル
 */
static void encode_sub_block(neac_encoder* encoder, uint8_t channel) {
	if (!encoder->disable_simple_predictor) {
		/* シンプル予測器を適用 */
		pass_simple_predictor(encoder->simple_predictors[channel], encoder->current_block->sub_blocks[channel], encoder->block_size);
	}

	/* 適応フィルタを適用 */
	pass_lms_filter(encoder->lms_filters[channel], encoder->current_block->sub_blocks[channel], encoder->block_size);
}

/*!
 * @brief			指定されたハンドルのエンコーダで読み込まれたブロックのエンコードを行います。
 * @param *encoder	エンコーダのハンドル
 */
static void encode_block(neac_encoder* encoder) {
	uint8_t ch;

	/* ミッドサイドステレオ変換が有効なら変換処理を行う */
	if (encoder->use_mid_side_stereo) {
		lr_to_ms_conversion(encoder->current_block, encoder->block_size);
	}

	/* すべてのチャンネルのサブブロックをエンコード */
	for (ch = 0; ch < encoder->num_channels; ++ch) {
		encode_sub_block(encoder, ch);
	}
}

#pragma endregion

/*!
 * @brief				ブロック数を計算します
 * @param num_samples	サンプル数
 * @param num_channels	チャンネル数
 * @param block_size	サブブロックのサンプル数
 * @return				ブロック数
 */
static uint32_t compute_block_count(uint32_t num_samples, uint8_t num_channels, uint16_t block_size) {
	uint32_t num_samples_per_ch = num_samples / num_channels;
	uint32_t num_blocks = num_samples_per_ch / block_size;

	if (num_samples_per_ch % block_size != 0) {
		++num_blocks;
	}

	return num_blocks;
}

/*!
 * @brief エンコーダを初期化します
 * @param *encoder				エンコーダのハンドル
 * @param *file					出力先のファイルハンドル
 * @param sample_rate			サンプリング周波数
 * @param bits_per_sample		量子化ビット数
 * @param num_channels			チャンネル数
 * @param num_samples			合計サンプル数
 * @param block_size			ブロックサイズ
 * @param use_mid_side_stereo	ミッドサイドステレオを使用するかどうかを示すフラグ
 * @param disable_simple_predictor	シンプル予測器を無効化するかどうかを示すフラグ
 * @param filter_taps			LMSフィルタのタップ数
 */
static void init(
	neac_encoder* encoder, 
	FILE* file, 
	uint32_t sample_rate, 
	uint8_t bits_per_sample, 
	uint8_t num_channels, 
	uint32_t num_samples, 
	uint16_t block_size, 
	bool use_mid_side_stereo, 
	bool disable_simple_predictor,
	uint8_t filter_taps) {
	uint8_t ch;

	/* ファイルを開いてビットストリームを初期化する */
	encoder->output_file = file;
	encoder->output_bit_stream = bit_stream_create(encoder->output_file, BIT_STREAM_MODE_WRITE);

	encoder->sample_rate = sample_rate;
	encoder->bits_per_sample = bits_per_sample;
	encoder->num_channels = num_channels;
	encoder->num_samples = num_samples;
	encoder->filter_taps = filter_taps;
	encoder->block_size = block_size;
	encoder->use_mid_side_stereo = use_mid_side_stereo;
	encoder->disable_simple_predictor = disable_simple_predictor;
	encoder->num_blocks = compute_block_count(num_samples, num_channels, block_size);
	encoder->lms_filters = (lms**)malloc(sizeof(lms*) * num_channels);
	encoder->simple_predictors = (simple_predictor**)malloc(sizeof(simple_predictor*) * num_channels);
	encoder->coder = neac_code_create(encoder->output_bit_stream);
	encoder->current_block = (neac_block*)malloc(sizeof(neac_block));
	encoder->current_sub_block_channel = 0;
	encoder->current_sub_block_offset = 0;

	/* ブロックを初期化 */
	neac_block_init(encoder->current_block, block_size, num_channels);

	/* 各チャンネル用のフィルタを初期化 */
	if (encoder->lms_filters != NULL && encoder->simple_predictors != NULL) {
		for (ch = 0; ch < num_channels; ++ch) {
			encoder->lms_filters[ch] = lms_create(bits_per_sample, filter_taps);
			encoder->simple_predictors[ch] = simple_predictor_create();
		}
	}
	else {
		report_error(NEAC_ERROR_ENCODER_CANNOT_ALLOCATE_MEMORY);
	}

	/* ヘッダ部を書き込む。 */
	write_header(encoder);
}

/*!
 * @brief エンコーダを初期化します
 * @param *encoder				エンコーダのハンドル
 * @param *path					出力先のパス
 * @param sample_rate			サンプリング周波数
 * @param bits_per_sample		量子化ビット数
 * @param num_channels			チャンネル数
 * @param num_samples			合計サンプル数
 * @param block_size			ブロックサイズ
 * @param use_mid_side_stereo	ミッドサイドステレオを使用するかどうかを示すフラグ
 * @param disable_simple_predictor	シンプル予測器を無効化するかどうかを示すフラグ
 * @param filter_taps			LMSフィルタのタップ数
 */
static void init_from_path(
	neac_encoder* encoder, 
	const char* path, 
	uint32_t sample_rate, 
	uint8_t bits_per_sample, 
	uint8_t num_channels, 
	uint32_t num_samples, 
	uint16_t block_size, 
	bool use_mid_side_stereo,
	bool disable_simple_predictor,
	uint8_t filter_taps) {
	FILE* fp;
	errno_t err;

	err = fopen_s(&fp, path, "wb");

	/* ファイルを開けなかった場合、エラーを報告して何もしない */
	if (err != 0) {
		report_error(NEAC_ERROR_ENCODER_FAILED_TO_OPEN_FILE);
		return;
	}

	init(
		encoder, 
		fp,
		sample_rate, 
		bits_per_sample, 
		num_channels, 
		num_samples, 
		block_size, 
		use_mid_side_stereo,
		disable_simple_predictor,
		filter_taps);
}

/*!
 * @brief 指定された設定で、エンコーダのハンドルを生成します。
 * @param *file						出力先のファイルハンドル
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
	uint8_t filter_taps) {
	neac_encoder* result = (neac_encoder*)malloc(sizeof(neac_encoder));

	if (result == NULL) {
		report_error(NEAC_ERROR_ENCODER_CANNOT_ALLOCATE_MEMORY);
		return NULL;
	}

	init(
		result,
		file,
		sample_rate,
		bits_per_sample,
		num_channels,
		num_samples,
		block_size,
		use_mid_side_stereo,
		disable_simple_predictor,
		filter_taps);

	return result;
}

/*!
 * @brief 指定された設定で、エンコーダのハンドルを生成します。
 * @param *path						出力先のパス
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
	uint8_t filter_taps) {
	neac_encoder* result = (neac_encoder*)malloc(sizeof(neac_encoder));

	if (result == NULL) {
		report_error(NEAC_ERROR_ENCODER_CANNOT_ALLOCATE_MEMORY);
		return NULL;
	}

	init_from_path(
		result,
		path,
		sample_rate,
		bits_per_sample,
		num_channels,
		num_samples,
		block_size,
		use_mid_side_stereo,
		disable_simple_predictor,
		filter_taps);

	return result;
}

/*!
 * @brief			指定されたハンドルのエンコーダを解放します。
 * @param *encoder	エンコーダのハンドル
 */
void neac_encoder_free(neac_encoder* encoder) {
	uint8_t ch;

	free(encoder->output_bit_stream);

	/* 各チャンネル用のフィルタを解放 */
	for (ch = 0; ch < encoder->num_channels; ++ch) {
		free(encoder->lms_filters[ch]);
		free(encoder->simple_predictors[ch]);
	}
	free(encoder->lms_filters);
	free(encoder->simple_predictors);

	free(encoder->coder);
	free(encoder->current_block);
}

/*!
 * @brief			指定されたハンドルのエンコーダで、指定されたサンプルをエンコードします。
 * @param *encoder	エンコーダのハンドル
 * @param sample	サンプル
 */
void neac_encoder_write_sample(neac_encoder* encoder, int32_t sample) {
	int32_t flg_encode_block = 0;

	encoder->current_block->sub_blocks[encoder->current_sub_block_channel++]->samples[encoder->current_sub_block_offset] = sample;

	/* オフセット更新 */
	if (encoder->current_sub_block_channel >= encoder->num_channels) {
		encoder->current_sub_block_channel = 0;
		++encoder->current_sub_block_offset;

		/* サブブロックのオフセットがブロックサイズ以上となった際は、サブブロックのオフセットをゼロに戻し、エンコードされたブロックを出力ストリームに書き込む */
		if (encoder->current_sub_block_offset >= encoder->block_size) {
			encoder->current_sub_block_offset = 0;
			flg_encode_block = 1;
		}
	}

	/* 出力ストリームへの書き込みフラグが立っていれば書き込む */
	if (flg_encode_block == 1) {
		encode_block(encoder);
		neac_code_write_block(encoder->coder, encoder->current_block, encoder->block_size);
	}
}

/*!
 * @brief			指定されたハンドルのエンコーダでのエンコードの終了処理を行います。すべてのサンプルのエンコードが終了した後に、必ず呼び出してください。
 * @param *encoder	エンコーダのハンドル
 */
void neac_encoder_end_write(neac_encoder* encoder) {
	if (encoder->current_sub_block_offset != 0) {
		encode_block(encoder);
		neac_code_write_block(encoder->coder, encoder->current_block, encoder->block_size);
	}

	bit_stream_close(encoder->output_bit_stream);
	fflush(encoder->output_file);
	fclose(encoder->output_file);
}