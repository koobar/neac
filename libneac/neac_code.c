#include "./include/macro.h"
#include "./include/neac_code.h"
#include "./include/neac_error.h"
#include "./include/neac_sub_block.h"
#include "./include/signal.h"
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define U8_MAX  0xff
#define U16_MAX 0xffff
#define U24_MAX 0xffffff
#define U32_MAX 0xffffffff

/* 整数の表現に必要となるビット数の高速計算に要するテーブル 
 * 0から255までの整数の表現に必要となる最小のビット数を格納しています。*/
static uint32_t nbits_table[] = {
    1, 1, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8
};

/*!
 * @brief           指定された値を表現するために必要なビット数を求めます。
 * @param value     値
 * @return          valueに指定された値を表現するために必要なビット数
 */
static inline uint32_t count_bits(const uint32_t value) {
    if (value <= U8_MAX) {
        return nbits_table[value];
    }

    if (value <= U16_MAX) {
        return nbits_table[RSHIFT(value, 8)] + 8;
    }

    if (value <= U24_MAX) {
        return nbits_table[RSHIFT(value, 16)] + 16;
    }

    return nbits_table[RSHIFT(value, 24)] + 24;
}

#pragma region unary符号の実装

/*!
 * @brief           ビットストリームに、指定された値をunary符号化して書き込みます。
 * @param *stream   ビットストリームのハンドル
 * @param value     書き込む値
 */
static inline void write_unary_code(bit_stream* stream, const uint32_t value) {
    register uint32_t i;

    for (i = 0; i < value; ++i) {
        bit_stream_write_bit(stream, 1);
    }
    bit_stream_write_bit(stream, 0);
}

/*!
 * @brief           ビットストリームから、unary符号化された値を読み込みます。
 * @param *stream   ビットストリームのハンドル
 * @return          読み込まれた値
 */
static inline uint32_t read_unary_code(bit_stream* stream) {
    register uint32_t result = 0;

    while (bit_stream_read_bit(stream)) {
        ++result;
    }

    return result;
}

#pragma endregion

#pragma region rice符号の実装

/*!
 * @brief           指定されたビットストリームに、指定された値を、指定されたパラメータでライス符号化して書き込みます。
 * @param *stream   ビットストリームのハンドル
 * @param parameter ライス符号化のパラメータ
 * @param value     値
 */
static void write_rice_code(bit_stream* stream, const uint32_t parameter, const int32_t value) {
    register uint32_t val;
    register uint32_t quotient;
    register uint32_t remainder;

    if (parameter < ENTROPY_PARAMETER_MIN || parameter > ENTROPY_RICE_PARAMETER_MAX) {
        report_error(NEAC_ERROR_RICE_CODING_INVALID_PARAMETER);
        return;
    }

    val = CONVERT_INT32_TO_UINT32(value);
    quotient = RSHIFT(val, parameter);
    remainder = val - LSHIFT(quotient, parameter);

    /* 商をunary符号として書き込む */
    write_unary_code(stream, quotient);

    /* 剰余を書き込む */
    bit_stream_write_uint(stream, remainder, parameter);
}

static inline void write_rice_values(bit_stream* stream, const signal* data, uint16_t start, uint16_t data_size, uint32_t parameter) {
    uint16_t offset;

    for (offset = 0; offset < data_size; ++offset) {
        write_rice_code(stream, parameter, data[offset + start]);
    }
}

/*!
 * @brief           指定されたビットストリームから、指定されたパラメータでエントロピー符号化された値を1つ読み込みます。
 * @param *stream   ビットストリームのハンドル
 * @param parameter エントロピー符号化のパラメータ
 * @return          読み込まれた値
 */
static int32_t read_rice_code(bit_stream* stream, const uint32_t parameter) {
    register uint32_t quotient;
    register uint32_t remainder;

    if (parameter < ENTROPY_PARAMETER_MIN || parameter > ENTROPY_RICE_PARAMETER_MAX) {
        report_error(NEAC_ERROR_RICE_CODING_INVALID_PARAMETER);
        return 0;
    }

    /* unary符号化された商を読み込む */
    quotient = read_unary_code(stream);

    /* 剰余を読み込む */
    remainder = bit_stream_read_uint(stream, parameter);

    /* 値を復元 */
    return CONVERT_UINT32_TO_INT32(LSHIFT(quotient, parameter) + remainder);
}

static inline void read_rice_values(bit_stream* stream, signal* data, uint16_t start, uint16_t data_size, uint32_t parameter) {
    uint16_t offset;

    for (offset = 0; offset < data_size; ++offset) {
        data[start + offset] = read_rice_code(stream, parameter);
    }
}

/*!
 * @brief                   指定されたデータを、指定されたパラメータでライス符号化した場合のビット数を計算します。
 * @param *data             データ
 * @param data_size         データのサイズ
 * @param parameter         ライス符号化のパラメータ
 * @return                  指定されたデータを、指定されたパラメータでライス符号化した場合のビット数
 */
static uint32_t compute_rice_total_bits(const signal* data, const uint16_t start, const uint16_t data_size, const uint32_t parameter) {
    register uint32_t estimated_size = 0;
    register uint32_t val;
    register uint32_t quotient;
    register uint16_t offset;

    for (offset = 0; offset < data_size; ++offset) {
        val = CONVERT_INT32_TO_UINT32(data[start + offset]);
        quotient = RSHIFT(val, parameter);

        estimated_size += quotient;                 /* 商のビット数 */
    }

    estimated_size += (parameter * data_size);      /* 剰余のビット数 */

    return estimated_size;
}

/*!
 * @brief               指定されたデータに対して最適となるライス符号化のパラメータを計算します。
 * @param *data         データのポインタ
 * @param start         データの開始オフセット
 * @param data_size     データのサイズ
 * @param *total_bits   [出力]求められたパラメータでライス符号化した場合のデータの合計ビット数
 * @return              指定されたデータに対して最適となるライス符号化のパラメータ
 */
static uint32_t compute_optimal_rice_parameter(const signal* data, const uint16_t start, const uint16_t data_size, uint32_t* total_bits) {
    register double sum = 0;
    register double mean = 0;
    register uint32_t i;
    register uint32_t parameter;

    /* 出現するデータの絶対値の平均を求める。*/
    for (i = 0; i < data_size; ++i) {
        /* sum += CONVERT_INT32_TO_UINT32(data[start + i]);     // こっちを使う場合、パラメータの最終出力を-1にすると良い */
        sum += abs(data[start + i]);
    }
    mean = sum / data_size;

    /* 平均を整数に丸め込んだ値の表現に必要なビット数をパラメータとする。*/
    parameter = count_bits((uint32_t)round(mean));
    if (parameter > ENTROPY_RICE_PARAMETER_MAX) {
        parameter = ENTROPY_RICE_PARAMETER_MAX;
    }
    *(total_bits) = compute_rice_total_bits(data, start, data_size, parameter);

    return parameter;
}

#pragma endregion

/*!
 * @brief                   パーティションで使用すべきエントロピー符号化のパラメータを計算します。
 * @param *data             データ全体のポインタ
 * @param start             パーティションの開始位置
 * @param partition_size    パーティションのサイズ
 * @param *partition_bits   [出力]求められたエントロピー符号化のパラメータを使用してエントロピー符号化した場合のパーティション全体のビット数
 * @return                  最適なパーティションパラメータ
 */
static uint32_t compute_optimal_entropy_parameter(const signal* data, const uint16_t start, const uint16_t partition_size, uint32_t* partition_bits) {
    bool is_blank_partition = true;
    register uint16_t offset;

    /* ブランクパーティション（すべての値がゼロであるパーティション）であるか判定する。*/
    for (offset = 0; offset < partition_size; ++offset) {
        if (data[offset + start] != 0) {
            is_blank_partition = false;
            break;
        }
    }

    if (is_blank_partition) {
        *(partition_bits) = ENTROPY_PARAMETER_NEED_BITS;
        return ENTROPY_PARAMETER_BLANK_PARTITION;
    }

    return compute_optimal_rice_parameter(data, start, partition_size, partition_bits);
}

/*!
 * @brief                           指定されたデータに対して最適となるパーティションパラメータ、および、それぞれのパーティションで使用すべきエントロピー符号化のパラメータを計算します。
 * @param *data                     データのポインタ
 * @param data_size                 データのサイズ
 * @param *entropy_parameters       [出力]各パーティションで使用すべきエントロピー符号化のパラメータを出力する領域のポインタ。ENTROPY_PARTITION_COUNT_MAX個の要素を格納できる必要があります。
 * @param *work                     作業領域のポインタ。ENTROPY_PARTITION_COUNT_MAX個の要素を格納できる必要があります。
 * @return                          指定されたデータに対して最適となるパーティションパラメータ
 */
static uint32_t compute_optimal_partition_parameter(const signal* data, const uint16_t data_size, uint32_t* entropy_parameters, uint32_t* work) {
    uint32_t partition_size;
    uint32_t partition_index;
    uint32_t trial_pp;
    uint32_t partition_count;
    uint32_t optimal_partition_parameter;
    uint32_t size, min_size;
    uint16_t start;
    uint32_t partition_bits;

    min_size = U32_MAX;
    optimal_partition_parameter = 0;

    for (trial_pp = ENTROPY_PARTITION_PARAMETER_MIN; trial_pp <= ENTROPY_PARTITION_PARAMETER_MAX; ++trial_pp) {
        partition_count = RESTORE_PARTITION_COUNT(trial_pp);
        partition_size = data_size / partition_count;
        size = 0;

        /* それぞれのパーティションにとって最適となるエントロピー符号化のパラメータを求め、作業領域に格納する */
        for (partition_index = 0; partition_index < partition_count; ++partition_index) {
            start = partition_size * partition_index;

            /* このパーティションのエントロピー符号化のパラメータを作業領域に保存 */
            work[partition_index] = compute_optimal_entropy_parameter(data, start, partition_size, &partition_bits);
            
            /* パーティションのサイズを加算 */
            size += partition_bits;
        }

        /* パーティションのサイズに、エントロピー符号化のパラメータを保存するために要するビット数を加算 */
        size += ENTROPY_PARTITION_PARAMETER_NEED_BITS;

        /* すでに発見されたパーティションの最小サイズより、今回試したパラメータによる
         * パーティションサイズのほうが小さければ、最小サイズを置き換え、作業領域に格納された、
         * すべてのパーティションのエントロピー符号化のパラメータを、出力領域にコピーする。*/
        if (min_size > size) {
            min_size = size;
            optimal_partition_parameter = trial_pp;
            memcpy(entropy_parameters, work, sizeof(uint32_t) * ENTROPY_PARTITION_COUNT_MAX);
        }
    }

    return optimal_partition_parameter;
}

/*!
 * @brief               指定されたビットストリームに、指定されたサブブロックをライス符号化して書き込みます。
 * @param *stream       ビットストリームのハンドル
 * @param *sub_block    サブブロックのハンドル
 * @param block_size    サブブロックのサンプル数
 */
static void write_sub_block(bit_stream* stream, uint32_t* entropy_parameters, uint32_t* work, const neac_sub_block* sub_block) {
    uint32_t p, start;
    uint32_t parameter;
    uint16_t partition_size;
    uint32_t partition_parameter;
    uint32_t partition_count;

    /* ライス符号化のパーティションパラメータを計算 */
    partition_parameter = compute_optimal_partition_parameter(sub_block->samples, sub_block->size, entropy_parameters, work);

    /* パーティションパラメータからパーティション数とパーティションのサイズを計算 */
    partition_count = RESTORE_PARTITION_COUNT(partition_parameter);
    partition_size = sub_block->size / partition_count;

    /* パーティションパラメータを保存する */
    bit_stream_write_uint(stream, partition_parameter - ENTROPY_PARTITION_PARAMETER_MIN, ENTROPY_PARTITION_PARAMETER_NEED_BITS);

    /* サブブロックをエントロピー符号化して書き込む */
    for (p = 0; p < partition_count; ++p) {
        start = partition_size * p;
        parameter = entropy_parameters[p];

        /* エントロピー符号化のパラメータを書き込む */
        bit_stream_write_uint(stream, (uint32_t)(parameter - ENTROPY_PARAMETER_MIN), ENTROPY_PARAMETER_NEED_BITS);

        if (parameter >= ENTROPY_RICE_PARAMETER_MIN && parameter <= ENTROPY_RICE_PARAMETER_MAX) {
            write_rice_values(stream, sub_block->samples, start, partition_size, parameter);
        }
    }
}

/*!
 * @brief               指定されたビットストリームから、ライス符号化されて書き込まれたサブブロックのデータを読み込み、指定されたサブブロックに格納します。
 * @param *stream       ビットストリームのハンドル
 * @param *sub_block    サブブロックのハンドル
 * @param block_size    サブブロックのサンプル数
 */
static void read_sub_block(bit_stream* stream, neac_sub_block* sub_block) {
    uint32_t parameter;
    uint16_t offset;
    uint32_t p, start;
    uint16_t partition_size;
    uint32_t partition_parameter;
    uint32_t partition_count;

    /* パーティションパラメータを取得 */
    partition_parameter = bit_stream_read_uint(stream, ENTROPY_PARTITION_PARAMETER_NEED_BITS) + ENTROPY_PARTITION_PARAMETER_MIN;

    /* ライス符号化のパーティション数とパーティションサイズを計算 */
    partition_count = RESTORE_PARTITION_COUNT(partition_parameter);
    partition_size = sub_block->size / partition_count;

    /* エントロピー符号化された整数を読み込む */
    for (p = 0; p < partition_count; ++p) {
        start = p * partition_size;

        /* エントロピー符号化のパラメータを取得 */
        parameter = bit_stream_read_uint(stream, ENTROPY_PARAMETER_NEED_BITS) + ENTROPY_PARAMETER_MIN;

        if (parameter >= ENTROPY_RICE_PARAMETER_MIN && parameter <= ENTROPY_RICE_PARAMETER_MAX) {
            read_rice_values(stream, sub_block->samples, start, partition_size, parameter);
        }
        else if (parameter == ENTROPY_PARAMETER_BLANK_PARTITION) {
            for (offset = 0; offset < partition_size; ++offset) {
                sub_block->samples[start + offset] = 0;
            }
        }
    }
}

/*!
 * @brief           ブロックを読み書きするAPIのハンドルを生成します。
 * @param *stream   ビットストリームのハンドル
 * @return          ブロック読み書きAPIのハンドル
 */
neac_code* neac_code_create(bit_stream* stream) {
    neac_code* result = (neac_code*)malloc(sizeof(neac_code));

    if (result == NULL) {
        return NULL;
    }

    result->bitstream = stream;
    result->workA = (uint32_t*)calloc(ENTROPY_PARTITION_COUNT_MAX, sizeof(uint32_t));
    result->workB = (uint32_t*)calloc(ENTROPY_PARTITION_COUNT_MAX, sizeof(uint32_t));

    return result;
}

/*!
 * @brief           ブロックを読み書きするAPIのハンドルを解放します。
 * @param *coder    ブロック読み書きAPIのハンドル
 */
void neac_code_free(neac_code* coder) {
    free(coder->workA);
    free(coder->workB);
}

/*!
 * @brief           ブロックを書き込みます。
 * @param *coder    ブロック読み書きAPIのハンドル
 * @param *block    書き込むブロックのハンドル
 */
void neac_code_write_block(neac_code* coder, neac_block* block) {
    uint8_t ch;

    for (ch = 0; ch < block->num_channels; ++ch) {
        write_sub_block(coder->bitstream, coder->workA, coder->workB, block->sub_blocks[ch]);
    }
}

/*!
 * @brief           ブロックを読み込みます。
 * @param *coder    ブロック読み書きAPIのハンドル
 * @param *block    読み込んだデータを格納するブロックのハンドル
 */
void neac_code_read_block(neac_code* coder, neac_block* block) {
    uint8_t ch;

    for (ch = 0; ch < block->num_channels; ++ch) {
        read_sub_block(coder->bitstream, block->sub_blocks[ch]);
    }
}