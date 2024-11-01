#ifndef NEAC_SUB_BLOCK_HEADER_INCLUDED
#define NEAC_SUB_BLOCK_HEADER_INCLUDED

#include "signal.h"
#include <stdint.h>

typedef struct {
    signal* samples;            /* サンプル領域のポインタ */
    uint16_t size;              /* サブブロックに格納されたサンプル数 */
    uint8_t channel;            /* サブブロックに対応するチャンネル */
} neac_sub_block;

/*!
 * @brief               サブブロックを初期化します。
 * @param *sub_block    サブブロックのハンドル
 * @param size          サブブロックのサンプル数
 * @param channel       対応するチャンネル
 */
void neac_sub_block_init(neac_sub_block* sub_block, uint16_t size, uint8_t channel);

/*!
 * @brief               サブブロックを解放します。
 * @param *sub_block    サブブロックのハンドル
 */
void neac_sub_block_free(neac_sub_block* sub_block);

#endif