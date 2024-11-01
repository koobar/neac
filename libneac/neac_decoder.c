#include "./include/file_access.h"
#include "./include/macro.h"
#include "./include/neac_decoder.h"
#include "./include/neac_error.h"
#include "./include/neac_sub_block.h"

const static uint8_t supported_format_versions[1] = { 0x01 };

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
    if (read_uint8(decoder->file) == 0x94 && 
        read_uint8(decoder->file) == 0x4C &&
        read_uint8(decoder->file) == 0x89 &&
        read_uint8(decoder->file) == 0xB9){
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

#pragma region デコード処理

/*!
 * @brief               ブロックがステレオである場合に限り、ブロックに格納されたミッドサイドステレオの音声信号を、シンプルステレオに変換します。
 * @param *block        処理するブロックのハンドル
 */
static inline void ms_to_lr_conversion(neac_block* block) {
    neac_sub_block* ch0 = NULL;
    neac_sub_block* ch1 = NULL;
    uint16_t offset;
    signal m, s;

    if (block->num_channels != 2) {
        return;
    }

    ch0 = block->sub_blocks[0];
    ch1 = block->sub_blocks[1];

    for (offset = 0; offset < block->size; ++offset) {
        m = ch0->samples[offset];
        s = ch1->samples[offset];
        m = LSHIFT(m, 1);
        m |= s & 1;

        ch0->samples[offset] = RSHIFT(m + s, 1);
        ch1->samples[offset] = RSHIFT(m - s, 1);
    }
}

/*!
 * @brief           指定されたデコーダで読み込み済みのブロックのデコードを行います。
 * @param *decoder  デコーダのハンドル
 */
static void decode_current_block(neac_decoder* decoder) {
    uint8_t ch;
    neac_sub_block* sb = NULL;
    lms* lms = NULL;
    polynomial_predictor* poly = NULL;
    register uint16_t offset;
    register signal residual;
    register signal sample;

    for (ch = 0; ch < decoder->num_channels; ++ch) {
        sb = decoder->current_block->sub_blocks[ch];
        lms = decoder->lms_filters[ch];
        poly = decoder->polynomial_predictors[ch];

        for (offset = 0; offset < sb->size; ++offset) {
            /* STEP 1. SSLMSフィルタを適用し、多項式予測器の予測残差を復元 */
            residual = sb->samples[offset];
            sample = residual + lms_predict(lms);
            lms_update(lms, sample, residual);

            /* STEP 2. 多項式予測器で予測された信号に予測残差を加算し、元の信号を復元*/
            sample += polynomial_predictor_predict(poly);
            polynomial_predictor_update(poly, sample);

            /* STEP 3. 復元された信号をサブブロックに格納 */
            sb->samples[offset] = sample;
        }
    }

    /* STEP 4. ミッドサイドステレオに変換されていれば、シンプルステレオに戻す */
    if (decoder->use_mid_side_stereo) {
        ms_to_lr_conversion(decoder->current_block);
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
    decoder->polynomial_predictors = (polynomial_predictor**)malloc(sizeof(polynomial_predictor*) * decoder->num_channels);
    decoder->coder = neac_code_create(decoder->bit_stream);
    decoder->current_block = (neac_block*)malloc(sizeof(neac_block));
    decoder->current_read_sub_block_channel = 0;
    decoder->current_read_sub_block_offset = 0;
    decoder->num_samples_read = 0;
    decoder->is_seeking = false;

    /* ブロックを初期化 */
    neac_block_init(decoder->current_block, decoder->block_size, decoder->num_channels);

    /* 領域の確保に成功していれば、初期化を行う */
    if (decoder->lms_filters != NULL && decoder->polynomial_predictors != NULL && decoder->current_block != NULL) {
        for (ch = 0; ch < decoder->num_channels; ++ch) {
            decoder->lms_filters[ch] = lms_create(decoder->filter_taps, decoder->bits_per_sample);
            decoder->polynomial_predictors[ch] = polynomial_predictor_create();
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
        lms_free(decoder->lms_filters[ch]);
        polynomial_predictor_free(decoder->polynomial_predictors[ch]);
    }
    free(decoder->lms_filters);
    free(decoder->polynomial_predictors);

    free(decoder->coder);
    free(decoder->current_block);
}

/*!
 * @brief           ファイルを閉じます。
 * @param decoder   デコーダのハンドル
 */
void neac_decoder_close(neac_decoder* decoder) {
    neac_tag_free(decoder->tag);
    fclose(decoder->file);
}

/*!
 * @brief           強制的に次の1サンプルを読み込み、PCMサンプルとして返します。
 * @param decoder   デコーダのハンドル
 * @return          デコードされたPCMサンプル
 */
static int32_t force_read_sample(neac_decoder* decoder) {
    int32_t sample;

    if (decoder->current_read_sub_block_offset == 0 && decoder->current_read_sub_block_channel == 0) {
        neac_code_read_block(decoder->coder, decoder->current_block);
        decode_current_block(decoder);
    }

    sample = (int32_t)decoder->current_block->sub_blocks[decoder->current_read_sub_block_channel++]->samples[decoder->current_read_sub_block_offset];

    if (decoder->current_read_sub_block_channel == decoder->num_channels) {
        decoder->current_read_sub_block_channel = 0;
        ++decoder->current_read_sub_block_offset;

        if (decoder->current_read_sub_block_offset == decoder->block_size) {
            decoder->current_read_sub_block_offset = 0;
        }
    }
    decoder->num_samples_read += 1;

    return sample;
}

/*!
 * @brief           次の1サンプルを読み込み、PCMサンプルとして返します。
 * @param decoder   デコーダのハンドル
 * @return          デコードされたPCMサンプル
 */
int32_t neac_decoder_read_sample(neac_decoder* decoder) {
    if (decoder->is_seeking) {
        return 0;
    }

    if (decoder->num_samples_read >= decoder->num_total_samples) {
        return 0;
    }

    return force_read_sample(decoder);
}

/*!
 * @brief                   指定されたオフセットのサンプルまでシークします。
 * @param *decoder          デコーダのハンドル
 * @param sample_offset     シーク先のサンプルのオフセット
 */
void neac_decoder_seek_sample_to(neac_decoder* decoder, uint32_t sample_offset) {
    uint8_t ch;
    uint32_t offset;

    /* シーク中フラグを立てる */
    decoder->is_seeking = true;

    /* 後方シーク（再生位置を過去に戻す）の場合、ファイルの最初からデコードをやり直す。*/
    if (sample_offset < decoder->num_samples_read) {
        rewind(decoder->file);
        bit_stream_init(decoder->bit_stream);

        /* ヘッダ部を読み込む */
        read_header(decoder);

        /* ブロックを初期化 */
        decoder->current_read_sub_block_channel = 0;
        decoder->current_read_sub_block_offset = 0;
        decoder->num_samples_read = 0;

        /* 領域の確保に成功していれば、初期化を行う */
        if (decoder->lms_filters != NULL && decoder->polynomial_predictors != NULL) {
            for (ch = 0; ch < decoder->num_channels; ++ch) {
                lms_clear(decoder->lms_filters[ch]);
                polynomial_predictor_clear(decoder->polynomial_predictors[ch]);
            }
        }
        offset = 0;
    }
    else {
        offset = decoder->num_samples_read;
    }

    /* 指定されたサンプルのインデックスまで読み飛ばす */
    for (; offset < sample_offset; ++offset) {
        force_read_sample(decoder);
    }

    /* シーク中フラグを折る */
    decoder->is_seeking = false;
}

/*!
 * @brief                   ミリ秒単位で指定された時間までシークします。
 * @param *decoder          デコーダのハンドル
 * @param sample_offset     シーク先時間（ミリ秒）
 */
void neac_decoder_seek_milliseconds_to(neac_decoder* decoder, uint32_t ms) {
    uint32_t samples_per_ms = (decoder->sample_rate * decoder->num_channels) / 1000;
    uint32_t offset = ms * samples_per_ms;

    neac_decoder_seek_sample_to(decoder, offset);
}

/*!
 * @brief                   デコーダが開いているファイルの演奏時間をミリ秒単位で取得します。
 * @param *decoder          デコーダのハンドル
 * @return                  演奏時間（ミリ秒単位）
 */
uint32_t neac_decoder_get_duration_ms(neac_decoder* decoder) {
    uint32_t samples_per_ms = (decoder->sample_rate * decoder->num_channels) / 1000;
    uint32_t total_ms = decoder->num_total_samples / samples_per_ms;

    return total_ms;
}