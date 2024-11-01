#ifndef NEAC_BLOCK_HEADER_INCLUDED
#define NEAC_BLOCK_HEADER_INCLUDED

#include "neac_sub_block.h"
#include <stdint.h>

/*!
 * @brief ブロック
 */
typedef struct {
    neac_sub_block** sub_blocks;
    uint16_t size;
    uint8_t num_channels;
} neac_block;

/*!
 * @brief               ブロックを初期化します。
 * @param block         初期化するブロックのハンドル
 * @param size          ブロックに含まれるサブブロックのサンプル数 
 * @param num_channels  ブロックのチャンネル数（サブブロック数）
 */
void neac_block_init(neac_block* block, uint16_t size, uint8_t num_channels);

/*!
 * @brief       ブロックを解放します。
 * @param block ブロックのハンドル
 */
void neac_block_free(neac_block* block);

#endif