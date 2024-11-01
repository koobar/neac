#ifndef NEAC_ENCODER_HEADER_INCLUDED
#define NEAC_ENCODER_HEADER_INCLUDED

#include "bit_stream.h"
#include "lms.h"
#include "neac_block.h"
#include "neac_code.h"
#include "neac_tag.h"
#include "polynomial_predictor.h"
#include <stdbool.h>

/*!
 * @brief エンコーダ
 */
typedef struct {
    FILE* output_file;                                  /* 出力先ファイルのハンドル*/
    bit_stream* output_bit_stream;                      /* 出力先ビットストリームのハンドル */

    uint32_t sample_rate;                               /* サンプリング周波数 */
    uint8_t bits_per_sample;                            /* PCMの量子化ビット数 */
    uint8_t num_channels;                               /* チャンネル数 */
    uint32_t num_samples;                               /* ファイルに含まれるサンプルの総数 */

    uint8_t filter_taps;                                /* SSLMSフィルタのタップ数 */
    uint16_t block_size;                                /* ブロック（厳密にはサブブロック）に格納されるサンプル数 */
    bool use_mid_side_stereo;                           /* ミッドサイドステレオを使用するかどうかを示すフラグ */
    uint32_t num_blocks;                                /* ファイルに含まれるブロック数 */

    lms** lms_filters;                                  /* チャンネル毎のSSLMSフィルタのハンドルが格納される領域 */
    polynomial_predictor** polynomial_predictors;       /* チャンネル毎の多項式予測器のハンドルが格納される領域 */

    neac_tag* tag;                                      /* タグ情報のハンドル */
    neac_code* coder;                                   /* ブロック読み書きAPIのハンドル */
    neac_block* current_block;                          /* エンコード中のブロックのハンドル */
    uint8_t current_sub_block_channel;                  /* 次にブロックにサンプルを書き込む場合のチャンネルのオフセット */
    uint16_t current_sub_block_offset;                  /* 次にブロックにサンプルを書き込む場合のサブブロックのオフセット */
} neac_encoder;

/*!
 * @brief                           指定された設定で、エンコーダのハンドルを生成します。
 * @param *file                     出力先のファイルハンドル
 * @param sample_rate               サンプリング周波数
 * @param bits_per_sample           量子化ビット数
 * @param num_channels              チャンネル数
 * @param num_samples               合計サンプル数
 * @param block_size                ブロックサイズ
 * @param use_mid_side_stereo       ミッドサイドステレオを使用するかどうかを示すフラグ
 * @param filter_taps               LMSフィルタの最大タップ数
 * @param *tag                      タグ情報
 */
neac_encoder* neac_encoder_create(
    FILE* file,
    uint32_t sample_rate,
    uint8_t bits_per_sample,
    uint8_t num_channels,
    uint32_t num_samples,
    uint16_t block_size,
    bool use_mid_side_stereo,
    uint8_t filter_taps,
    neac_tag* tag);

/*!
 * @brief                           指定された設定で、エンコーダのハンドルを生成します。
 * @param *path                      出力先のパス
 * @param sample_rate               サンプリング周波数
 * @param bits_per_sample           量子化ビット数
 * @param num_channels              チャンネル数
 * @param num_samples               合計サンプル数
 * @param block_size                ブロックサイズ
 * @param use_mid_side_stereo       ミッドサイドステレオを使用するかどうかを示すフラグ
 * @param filter_taps               LMSフィルタの最大タップ数
 * @param *tag                      タグ情報
 */
neac_encoder* neac_encoder_create_from_path(
    const char* path,
    uint32_t sample_rate,
    uint8_t bits_per_sample,
    uint8_t num_channels,
    uint32_t num_samples,
    uint16_t block_size,
    bool use_mid_side_stereo,
    uint8_t filter_taps,
    neac_tag* tag);

/*!
 * @brief           エンコーダを解放します。
 * @param *encoder  エンコーダのハンドル
 */
void neac_encoder_free(neac_encoder* encoder);

/*!
 * @brief           指定されたハンドルのエンコーダで、指定されたサンプルをエンコードします。
 * @param *encoder  エンコーダのハンドル
 * @param sample    サンプル
 */
void neac_encoder_write_sample(neac_encoder* encoder, int32_t sample);

/*!
 * @brief           指定されたハンドルのエンコーダでのエンコードの終了処理を行います。すべてのサンプルのエンコードが終了した後に、必ず呼び出してください。
 * @param *encoder  エンコーダのハンドル
 */
void neac_encoder_end_write(neac_encoder* encoder);

#endif