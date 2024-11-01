#include "./include/neac_error.h"
#include "./include/neac_sub_block.h"
#include <stdbool.h>
#include <stdlib.h>

/*!
 * @brief               サブブロックを初期化します。
 * @param *sub_block    サブブロックのハンドル
 * @param size          サブブロックのサンプル数
 * @param channel       対応するチャンネル
 */
void neac_sub_block_init(neac_sub_block* sub_block, uint16_t size, uint8_t channel) {
    sub_block->size = size;
    sub_block->channel = channel;
    sub_block->samples = (signal*)calloc(size, sizeof(signal));

    if (sub_block->samples == NULL) {
        report_error(NEAC_ERROR_SUB_BLOCK_CANNOT_ALLOCATE_MEMORY);
    }
}

/*!
 * @brief               サブブロックを解放します。
 * @param *sub_block    サブブロックのハンドル
 */
void neac_sub_block_free(neac_sub_block* sub_block) {
    free(sub_block->samples);
}