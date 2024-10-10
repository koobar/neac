#include "./include/neac_decoder.h"

const static uint8_t supported_format_versions[1] = { 0x02 };

#pragma region データ読み込み

/*! 
 * @brief           指定されたバージョン番号が、このデコーダでサポートされているフォーマットバージョンであるかを判定します。
 * @param version   バージョン番号
 * @return          サポートされているバージョンなら true を、そうでなければ false を返します
 */
static bool is_supported_version(uint8_t version) {
    size_t n = sizeof(supported_format_versions) / sizeof(supported_format_versions[0]);
    size_t i;

    for (i = 0; i < n; ++i) {
        if (supported_format_versions[i] == version) {
            return true;
        }
    }

    return false;
}

/*!
 * @brief           指定されたハンドルのデコーダで開かれたファイルから、NEACファイルのヘッダ部を読み込みます。
 * @param *decoder  デコーダのハンドル
 */
static void read_header(neac_decoder* decoder) {
    if (read_char(decoder->file) == 'N' &&
        read_char(decoder->file) == 'E' &&
        read_char(decoder->file) == 'A' &&
        read_char(decoder->file) == 'C') {
        /* フォーマットバージョンを読み込む */
        decoder->format_version = read_uint8(decoder->file);

        if (!is_supported_version(decoder->format_version)) {
            report_error(NEAC_ERROR_DECODER_UNSUPPORTED_FORMAT_VERSION);
            return;
        }

        /* PCMフォーマット情報を読み込む */
        decoder->sample_rate = read_uint32(decoder->file);
        decoder->bits_per_sample = read_uint8(decoder->file);
        decoder->num_channels = read_uint8(decoder->file);
        decoder->num_total_samples = read_uint32(decoder->file);

        /* NEACエンコード情報を読み込む */
        decoder->filter_taps = read_uint8(decoder->file);
        decoder->block_size = read_uint16(decoder->file);
        decoder->use_mid_side_stereo = read_bool(decoder->file);
        decoder->disable_simple_predictor = read_bool(decoder->file);
        decoder->num_blocks = read_uint32(decoder->file);

        /* タグ情報を読み込む */
        neac_tag_read(decoder->file, &decoder->tag);
    }
    else {
        /* マジックナンバーが不正な値である旨のエラーをレポートする */
        report_error(NEAC_ERROR_DECODER_INVALID_MAGIC_NUMBER);
    }
}

#pragma endregion

#pragma region 信号処理

/*!
 * @brief               適応フィルタにより予測符号化されたサブブロックをデコードします。
 * @param *filter       適応フィルタのハンドル
 * @param *sub_block    サブブロックのハンドル
 * @param block_size    ブロックサイズ
 */
static void pass_lms(lms* filter, neac_sub_block* sub_block, uint16_t block_size) {
    uint16_t offset;
    signal residual, prediction, sample;

    for (offset = 0; offset < block_size; ++offset) {
        residual = sub_block->samples[offset];
        prediction = lms_predict(filter);
        sample = residual + prediction;

        sub_block->samples[offset] = sample;
        lms_update(filter, sample, residual);
    }
}

/*!
 * @brief               シンプル予測器により予測符号化されたサブブロックをデコードします。
 * @param *filter       シンプル予測器のハンドル
 * @param *sub_block    サブブロックのハンドル
 * @param block_size    ブロックサイズ
 */
static void pass_simple_predictor(simple_predictor* predictor, neac_sub_block* sub_block, uint16_t block_size) {
    uint16_t offset;
    signal residual, prediction, sample;

    for (offset = 0; offset < block_size; ++offset) {
        residual = sub_block->samples[offset];
        prediction = simple_predictor_predict(predictor);
        sample = residual + prediction;

        sub_block->samples[offset] = sample;
        simple_predictor_update(predictor, sample);
    }
}

/*!
 * @brief               ブロックがステレオである場合に限り、ブロックに格納されたミッドサイドステレオの音声信号を、シンプルステレオに変換します。
 * @param *block        処理するブロックのハンドル
 * @param block_size    処理するブロックのサイズ
 */
static void ms_to_lr_conversion(neac_block* block, uint16_t block_size) {
    neac_sub_block* ch0 = NULL;
    neac_sub_block* ch1 = NULL;
    uint16_t offset;
    signal m, s;

    if (block->num_channels != 2) {
        return;
    }

    ch0 = block->sub_blocks[0];
    ch1 = block->sub_blocks[1];

    for (offset = 0; offset < block_size; ++offset) {
        m = ch0->samples[offset];
        s = ch1->samples[offset];
        m = m << 1;                             /* m = m * 2; */
        m |= s & 1;                             /* m += (s % 2) ? 1 : 0; */

        ch0->samples[offset] = (m + s) >> 1;    /* (m + s) / 2; */
        ch1->samples[offset] = (m - s) >> 1;    /* (m - s) / 2; */
    }
}

/*!
 * @brief           指定されたデコーダで読み込み済みのブロックのデコードを行います。
 * @param *decoder  デコーダのハンドル
 */
static void decode_block(neac_decoder* decoder) {
    uint8_t ch;

    for (ch = 0; ch < decoder->num_channels; ++ch) {
        pass_lms(decoder->lms_filters[ch], decoder->current_block->sub_blocks[ch], decoder->block_size);

        if (!decoder->disable_simple_predictor) {
            pass_simple_predictor(decoder->simple_predictors[ch], decoder->current_block->sub_blocks[ch], decoder->block_size);
        }
    }

    /* ミッドサイドステレオに変換されていれば、シンプルステレオに戻す */
    if (decoder->use_mid_side_stereo) {
        ms_to_lr_conversion(decoder->current_block, decoder->block_size);
    }
}

#pragma endregion

/*!
 * @brief           指定されたデコーダを、指定されたパスのファイルをデコードできるように初期化します。
 * @param *decoder  デコーダのハンドル
 * @param *path     ファイルのパス
 */
static void neac_decoder_init(neac_decoder* decoder, const char* path) {
    uint8_t ch;
    errno_t err;

    /* ファイルを開く */
    err = fopen_s(&decoder->file, path, "rb");

    /* ファイルを開けなかった場合、エラーを報告して何もしない */
    if (err != 0) {
        report_error(NEAC_ERROR_DECODER_FAILED_TO_OPEN_FILE);
        return;
    }

    /* ビットストリームの初期化 */
    decoder->bit_stream = bit_stream_create(decoder->file, BIT_STREAM_MODE_READ);

    /* ヘッダ部を読み込む */
    read_header(decoder);

    decoder->lms_filters = (lms**)malloc(sizeof(lms*) * decoder->num_channels);
    decoder->simple_predictors = (simple_predictor**)malloc(sizeof(simple_predictor*) * decoder->num_channels);
    decoder->coder = neac_code_create(decoder->bit_stream);
    decoder->current_block = (neac_block*)malloc(sizeof(neac_block));
    decoder->current_read_sub_block_channel = 0;
    decoder->current_read_sub_block_offset = 0;

    /* ブロックを初期化 */
    neac_block_init(decoder->current_block, decoder->block_size, decoder->num_channels);

    /* 領域の確保に成功していれば、初期化を行う */
    if (decoder->lms_filters != NULL && decoder->simple_predictors != NULL) {
        for (ch = 0; ch < decoder->num_channels; ++ch) {
            decoder->lms_filters[ch] = lms_create(decoder->bits_per_sample, decoder->filter_taps);
            decoder->simple_predictors[ch] = simple_predictor_create();
        }
    }
    else {
        report_error(NEAC_ERROR_DECODER_CANNOT_ALLOCATE_MEMORY);
    }
}

/*!
 * @brief       デコーダのハンドルを生成します。
 * @param path  デコードするファイルのパス
 * @return      デコーダのハンドル
 */
neac_decoder* neac_decoder_create(const char* path) {
    neac_decoder* result = (neac_decoder*)malloc(sizeof(neac_decoder));

    if (result == NULL) {
        report_error(NEAC_ERROR_DECODER_CANNOT_ALLOCATE_MEMORY);
        return NULL;
    }

    neac_decoder_init(result, path);
    return result;
}

/*!
 * @brief           デコーダを解放します。
 * @param decoder   デコーダのハンドル
 */
void neac_decoder_free(neac_decoder* decoder) {
    uint8_t ch;

    free(decoder->bit_stream);

    /* 各チャンネル用のフィルタを解放 */
    for (ch = 0; ch < decoder->num_channels; ++ch) {
        free(decoder->lms_filters[ch]);
        free(decoder->simple_predictors[ch]);
    }
    free(decoder->lms_filters);
    free(decoder->simple_predictors);

    free(decoder->coder);
    free(decoder->current_block);
}

/*!
 * @brief           ファイルを閉じます。
 * @param decoder   デコーダのハンドル
 */
void neac_decoder_close(neac_decoder* decoder) {
    fclose(decoder->file);
}

/*!
 * @brief           次の1サンプルを読み込み、PCMサンプルとして返します。
 * @param decoder   デコーダのハンドル
 * @return          デコードされたPCMサンプル
 */
int32_t neac_decoder_read_sample(neac_decoder* decoder) {
    int32_t sample;

    if (decoder->current_read_sub_block_offset == 0 && decoder->current_read_sub_block_channel == 0) {
        neac_code_read_block(decoder->coder, decoder->current_block, decoder->block_size);
        decode_block(decoder);
    }

    sample = (int32_t)decoder->current_block->sub_blocks[decoder->current_read_sub_block_channel++]->samples[decoder->current_read_sub_block_offset];

    /* オフセット計算 */
    if (decoder->current_read_sub_block_channel == decoder->num_channels) {
        decoder->current_read_sub_block_channel = 0;
        ++decoder->current_read_sub_block_offset;

        if (decoder->current_read_sub_block_offset == decoder->block_size) {
            decoder->current_read_sub_block_offset = 0;
        }
    }

    return sample;
}