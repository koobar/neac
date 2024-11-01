#ifndef NEAC_DECODER_HEADER_INCLUDED
#define NEAC_DECODER_HEADER_INCLUDED

#include "bit_stream.h"
#include "lms.h"
#include "neac_block.h"
#include "neac_code.h"
#include "neac_tag.h"
#include "polynomial_predictor.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

/*!
 * @brief NEACデコーダ
 */
typedef struct {
    FILE* file;                                     /* ファイルハンドル */
    bit_stream* bit_stream;                         /* ビットストリームのハンドル */

    uint8_t format_version;                         /* フォーマットのバージョン */
    uint32_t sample_rate;                           /* サンプリング周波数 */
    uint8_t bits_per_sample;                        /* PCMの量子化ビット数 */
    uint8_t num_channels;                           /* チャンネル数 */
    uint32_t num_total_samples;                     /* ファイルに含まれるサンプルの総数 */
    uint8_t filter_taps;                            /* SSLMSフィルタのタップ数 */
    uint16_t block_size;                            /* ブロック（厳密にはサブブロック）に含まれるサンプル数*/
    bool use_mid_side_stereo;                       /* ミッドサイドステレオが使用されているかどうかを示すフラグ */
    uint32_t num_blocks;                            /* ファイルに含まれるブロックの総数 */

    lms** lms_filters;                              /* チャンネル毎のSSLMSフィルタのハンドルを格納する領域 */
    polynomial_predictor** polynomial_predictors;   /* チャンネル毎の多項式予測器のハンドルを格納する領域 */

    neac_tag* tag;                                  /* タグ情報のハンドル */
    neac_code* coder;                               /* ブロック読み書きAPIのハンドル */
    neac_block* current_block;                      /* デコード中のブロックのハンドル */
    uint8_t current_read_sub_block_channel;         /* 次にサンプルを読み込む場合のチャンネルのオフセット */
    uint16_t current_read_sub_block_offset;         /* 次にサンプルを読み込む場合に参照するサブブロックのオフセット */

    uint32_t num_samples_read;                      /* 読み込み済みサンプル数 */
    bool is_seeking;                                /* シーク処理中であるかどうかを示すフラグ */
} neac_decoder;

/*!
 * @brief                   デコーダのハンドルを生成します。
 * @param path              デコードするファイルのパス
 * @return                  デコーダのハンドル
 */
neac_decoder* neac_decoder_create(const char* path);

/*!
 * @brief                   デコーダを解放します。
 * @param decoder           デコーダのハンドル
 */
void neac_decoder_free(neac_decoder* decoder);

/*!
 * @brief                   ファイルを閉じます。
 * @param decoder           デコーダのハンドル
 */
void neac_decoder_close(neac_decoder* decoder);

/*!
 * @brief                   次の1サンプルを読み込み、PCMサンプルとして返します。
 * @param decoder           デコーダのハンドル
 * @return                  デコードされたPCMサンプル
 */
int32_t neac_decoder_read_sample(neac_decoder* decoder);

/*!
 * @brief                   指定されたオフセットのサンプルまでシークします。
 * @param *decoder          デコーダのハンドル
 * @param sample_offset     シーク先のサンプルのオフセット
 */
void neac_decoder_seek_sample_to(neac_decoder* decoder, uint32_t sample_offset);

/*!
 * @brief                   ミリ秒単位で指定された時間までシークします。
 * @param *decoder          デコーダのハンドル
 * @param sample_offset     シーク先時間（ミリ秒）
 */
void neac_decoder_seek_milliseconds_to(neac_decoder* decoder, uint32_t ms);

/*!
 * @brief                   デコーダが開いているファイルの演奏時間をミリ秒単位で取得します。
 * @param *decoder          デコーダのハンドル
 * @return                  演奏時間（ミリ秒単位）
 */
uint32_t neac_decoder_get_duration_ms(neac_decoder* decoder);

#endif