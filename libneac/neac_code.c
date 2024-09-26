#include "./include/neac_code.h"

/*!
 * @brief       入力が正またはゼロの場合は入力の2倍を、入力が負の場合、入力の符号反転の2倍から1を引いた値を返します。
 * @param value 入力
 * @return      入力が正またはゼロの場合は入力の2倍を、入力が負の場合、入力の符号反転の2倍から1を引いた値（必ず非負整数が返されます）
 */
static inline uint32_t convert_int32_to_uint32(int32_t value) {
    uint32_t result = 0;

    if (value < 0) {
        result = (uint32_t)((-(value << 1)) - 1);       /* 入力が負の場合、入力を2倍し、1を引く */
    }
    else {
        result = (uint32_t)(value << 1);                /* 入力が正またはゼロの場合、入力を2倍する */
    }

    return result;
}

/*!
 * @brief           convert_int32_to_uint32 の出力が指定された値となる、convert_int32_to_uint32 の入力値を求めます。
 * @param value     値
 * @return          convert_int32_to_uint32 の出力が指定された値となる、convert_int32_to_uint32 の入力値
 */
static inline signal convert_uint32_to_int32(uint32_t value) {
    int32_t x = (int32_t)(value >> 1);  /* 1ビット右シフトし、上位ビットを取得 */
    int32_t y = -(int32_t)(value & 1);  /* 最下位ビットを取得。これが符号ビットとなる */

    return x ^ y;                       /* 上位ビットと符号ビットの排他的論理和は、convert_int32_to_uint32 の入力と等しい */
}

#pragma region ライス符号化の実装

/*!
 * @brief           指定されたデータを、指定されたパラメータでエントロピー符号化した場合のビット数を計算します。
 * @param *data     データ
 * @param data_size データのサイズ
 * @param parameter エントロピー符号化のパラメータ
 * @return          指定されたデータを、指定されたパラメータでエントロピー符号化した場合のビット数
 */
static uint32_t compute_rice_size(signal* data, uint16_t data_size, uint32_t parameter) {
    uint32_t estimated_size = 0, val, quotient;
    uint16_t offset;

    for (offset = 0; offset < data_size; ++offset) {
        val = convert_int32_to_uint32((int32_t)data[offset]);
        quotient = val >> parameter;

        estimated_size += quotient;     /* 商のビット数 */
        estimated_size += parameter;    /* 剰余のビット数 */
    }

    return estimated_size;
}

/*!
 * @brief           指定されたデータに対して最適となるエントロピー符号化のパラメータを計算します。
 * @param *data     データのポインタ
 * @param data_size データのサイズ
 * @return          指定されたデータに対して最適となるエントロピー符号化のパラメータ
 */
static uint32_t compute_optimal_parameter(signal* data, uint16_t data_size) {
    uint32_t min_size = UINT32_MAX, result, parameter, size;

    for (parameter = ENTROPY_PARAMETER_MIN; parameter <= ENTROPY_PARAMETER_MAX; ++parameter) {
        size = compute_rice_size(data, data_size, parameter);

        if (size < min_size) {
            min_size = size;
            result = parameter;
        }
    }

    return result;
}

/*!
 * @brief           指定されたビットストリームに、指定された値を、指定されたパラメータでエントロピー符号化して書き込みます。
 * @param *stream   ビットストリームのハンドル
 * @param parameter エントロピー符号化のパラメータ
 * @param value     値
 */
static void write_rice_code(bit_stream* stream, uint32_t parameter, signal value) {
    uint32_t val, quotient, remainder, i;

    if (parameter < ENTROPY_PARAMETER_MIN || parameter > ENTROPY_PARAMETER_MAX) {
        report_error(NEAC_ERROR_ENTROPY_CODING_INVALID_PARAMETER);
        return;
    }

    val = convert_int32_to_uint32((int32_t)value);
    quotient = val >> parameter;
    /* remainder = val - (val << parameter); */     /* これでも正常に動作するが原理は理解していない?? */
    remainder = val - (quotient << parameter);

    /* 商をunary符号として書き込む */
    for (i = 0; i < quotient; ++i) {
        bit_stream_write_bit(stream, 1);
    }
    bit_stream_write_bit(stream, 0);

    /* 剰余を書き込む */
    bit_stream_write_uint(stream, remainder, parameter);
}

/*!
 * @brief               指定されたビットストリームに、指定されたサブブロックをライス符号化して書き込みます。
 * @param *stream       ビットストリームのハンドル
 * @param *sub_block    サブブロックのハンドル
 * @param block_size    サブブロックのサンプル数
 */
static void write_sub_block_rice_coding(bit_stream* stream, neac_sub_block* sub_block, uint16_t block_size) {
    uint16_t offset;
    uint32_t parameter;

    /* ライス符号化の最適なパラメータを計算する */
    parameter = compute_optimal_parameter(sub_block->samples, block_size);

    /* ライス符号化のパラメータを書き込む */
    bit_stream_write_uint(stream, (uint32_t)(parameter - ENTROPY_PARAMETER_MIN), ENTROPY_PARAMETER_NEED_BITS);

    /* サブブロックに含まれるすべてのデータを書き込む */
    for (offset = 0; offset < block_size; ++offset) {
        write_rice_code(stream, parameter, sub_block->samples[offset]);
    }
}

/*!
 * @brief           指定されたビットストリームから、指定されたパラメータでエントロピー符号化された値を1つ読み込みます。
 * @param *stream   ビットストリームのハンドル
 * @param parameter エントロピー符号化のパラメータ
 * @return          読み込まれた値
 */
static signal read_rice_code(bit_stream* stream, uint32_t parameter) {
    uint32_t quotient, remainder;

    if (parameter < ENTROPY_PARAMETER_MIN || parameter > ENTROPY_PARAMETER_MAX) {
        report_error(NEAC_ERROR_ENTROPY_CODING_INVALID_PARAMETER);
        return 0;
    }

    /* unary符号化された商を読み込む */
    quotient = 0;
    while (bit_stream_read_bit(stream)) {
        ++quotient;
    }

    /* 剰余を読み込む */
    remainder = bit_stream_read_uint(stream, parameter);

    /* 値を復元 */
    return convert_uint32_to_int32((quotient << parameter) + remainder);
}

/*!
 * @brief               指定されたビットストリームから、ライス符号化されて書き込まれたサブブロックのデータを読み込み、指定されたサブブロックに格納します。
 * @param *stream       ビットストリームのハンドル
 * @param *sub_block    サブブロックのハンドル
 * @param block_size    サブブロックのサンプル数
 */
static void read_sub_block_rice_coding(bit_stream* stream, neac_sub_block* sub_block, uint16_t block_size) {
    uint32_t parameter;
    uint16_t offset;

    /* ライス符号化のパラメータを取得 */
    parameter = bit_stream_read_uint(stream, ENTROPY_PARAMETER_NEED_BITS) + ENTROPY_PARAMETER_MIN;

    /* 予測残差の読み込み */
    for (offset = 0; offset < block_size; ++offset) {
        sub_block->samples[offset] = read_rice_code(stream, parameter);
    }
}

#pragma endregion

neac_code* neac_code_create(bit_stream* stream) {
    neac_code* result = (neac_code*)malloc(sizeof(neac_code));

    if (result == NULL) {
        return NULL;
    }

    result->bitstream = stream;
    return result;
}

void neac_code_write_block(neac_code* coder, neac_block* block, uint16_t block_size) {
    uint8_t ch;

    for (ch = 0; ch < block->num_channels; ++ch) {
        write_sub_block_rice_coding(coder->bitstream, block->sub_blocks[ch], block_size);
    }
}

void neac_code_read_block(neac_code* coder, neac_block* block, uint16_t block_size) {
    uint8_t ch;

    for (ch = 0; ch < block->num_channels; ++ch) {
        read_sub_block_rice_coding(coder->bitstream, block->sub_blocks[ch], block_size);
    }
}