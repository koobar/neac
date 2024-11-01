#ifndef NEAC_CODE_HEADER_INCLUDED
#define NEAC_CODE_HEADER_INCLUDED

#define RESTORE_PARTITION_COUNT(partition_parameter) (LSHIFT(1, partition_parameter))
#define RESTORE_PARTITION_SIZE(data_size, partition_parameter) ((data_size) / RESTORE_PARTITION_COUNT(partition_parameter))

#define ENTROPY_PARAMETER_NEED_BITS                 5
#define ENTROPY_PARAMETER_MIN                       0
#define ENTROPY_RICE_PARAMETER_MIN                  ENTROPY_PARAMETER_MIN
#define ENTROPY_RICE_PARAMETER_MAX                  ENTROPY_RICE_PARAMETER_MIN + 30
#define ENTROPY_PARAMETER_BLANK_PARTITION           ENTROPY_RICE_PARAMETER_MAX + 1
#define ENTROPY_PARAMETER_MAX                       LSHIFT(1, ENTROPY_PARAMETER_NEED_BITS)

#define ENTROPY_PARTITION_PARAMETER_MIN             1
#define ENTROPY_PARTITION_PARAMETER_MAX             ENTROPY_PARTITION_PARAMETER_MIN + 3
#define ENTROPY_PARTITION_PARAMETER_NEED_BITS       2
#define ENTROPY_PARTITION_COUNT_MAX                 RESTORE_PARTITION_COUNT(ENTROPY_PARTITION_PARAMETER_MAX)

#include "bit_stream.h"
#include "neac_block.h"

typedef struct {
    bit_stream* bitstream;      /* 入出力用ビットストリームのハンドル */
    uint32_t* workA;            /* 作業領域A */
    uint32_t* workB;            /* 作業領域B */
} neac_code;

/*!
 * @brief           ブロックを読み書きするAPIのハンドルを生成します。
 * @param *stream   ビットストリームのハンドル
 * @return          ブロック読み書きAPIのハンドル
 */
neac_code* neac_code_create(bit_stream* stream);

/*!
 * @brief           ブロックを読み書きするAPIのハンドルを解放します。
 * @param *coder    ブロック読み書きAPIのハンドル
 */
void neac_code_free(neac_code* coder);

/*!
 * @brief           ブロックを書き込みます。
 * @param *coder    ブロック読み書きAPIのハンドル
 * @param *block    書き込むブロックのハンドル
 */
void neac_code_write_block(neac_code* coder, neac_block* block);

/*!
 * @brief           ブロックを読み込みます。
 * @param *coder    ブロック読み書きAPIのハンドル
 * @param *block    読み込んだデータを格納するブロックのハンドル
 */
void neac_code_read_block(neac_code* coder, neac_block* block);

#endif