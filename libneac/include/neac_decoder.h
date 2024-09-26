#ifndef NEAC_DECODER_H
#define NEAC_DECODER_H

#include "bit_stream.h"
#include "file_access.h"
#include "lms.h"
#include "neac.h"
#include "neac_block.h"
#include "neac_code.h"
#include "neac_sub_block.h"
#include "simple_predictor.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/*!
 * @brief NEACデコーダ
 */
typedef struct neac_decoder {
    FILE* file;
    bit_stream* bit_stream;

    uint8_t format_version;
    uint32_t sample_rate;
    uint8_t bits_per_sample;
    uint8_t num_channels;
    uint32_t num_total_samples;
    uint8_t filter_taps;
    uint16_t block_size;
    bool use_mid_side_stereo;
    bool disable_simple_predictor;
    uint32_t num_blocks;

    lms** lms_filters;
    simple_predictor** simple_predictors;

    neac_code* coder;
    neac_block* current_block;
    uint8_t current_read_sub_block_channel;
    uint16_t current_read_sub_block_offset;
} neac_decoder;

/*!
 * @brief       デコーダのハンドルを生成します。
 * @param path  デコードするファイルのパス
 * @return      デコーダのハンドル
 */
neac_decoder* neac_decoder_create(const char* path);

/*!
 * @brief           デコーダを解放します。
 * @param decoder   デコーダのハンドル
 */
void neac_decoder_free(neac_decoder* decoder);

/*!
 * @brief           ファイルを閉じます。
 * @param decoder   デコーダのハンドル
 */
void neac_decoder_close(neac_decoder* decoder);

/*!
 * @brief           次の1サンプルを読み込み、PCMサンプルとして返します。
 * @param decoder   デコーダのハンドル
 * @return          デコードされたPCMサンプル
 */
int32_t neac_decoder_read_sample(neac_decoder* decoder);

#endif