#include "./include/lms.h"
#include "./include/macro.h"
#include "./include/neac.h"
#include "./include/neac_error.h"
#include <stdlib.h>
#include <string.h>

#define SHIFT_FACTOR_PCM16  9
#define SHIFT_FACTOR_PCM24  8
#define SIGN(value)         ((value > 0) - (value < 0))

/*!
 * @brief           LMSフィルタを初期化します。
 * @param filter    LMSフィルタのハンドル
 */
static void lms_init(lms* filter, uint8_t taps, uint8_t pcm_bits) {
    /* 過去サンプルと重み係数の領域を確保 */
    filter->history = (signal*)calloc(taps, sizeof(signal));
    filter->weights = (signal*)calloc(taps, sizeof(signal));

    if (filter->history == NULL || filter->weights == NULL) {
        report_error(NEAC_ERROR_LMS_CANNOT_ALLOCATE_MEMORY);
        return;
    }

    /* タップ数を設定 */
    filter->taps = taps;

    /* PCMのビット数に応じたシフトファクタを設定 */
    if (pcm_bits == 16) {
        filter->shift = SHIFT_FACTOR_PCM16;
    }
    else if (pcm_bits == 24) {
        filter->shift = SHIFT_FACTOR_PCM24;
    }
}

/*!
 * @brief           LMSフィルタのハンドルを生成します。
 * @return			LMSフィルタのハンドル
 */
lms* lms_create(uint8_t taps, uint8_t pcm_bits) {
    lms* result = (lms*)malloc(sizeof(lms));

    if (result == NULL){
        report_error(NEAC_ERROR_LMS_CANNOT_ALLOCATE_MEMORY);
        return NULL;
    }

    lms_init(result, taps, pcm_bits);
    return result;
}

/*!
 * @brief           LMSフィルタを解放します。
 * @param *filter   LMSフィルタのハンドル
 */
void lms_free(lms* filter) {
    free(filter->history);
    free(filter->weights);
}

void lms_clear(lms* filter) {
    memset(filter->history, 0, sizeof(signal) * filter->taps);
    memset(filter->weights, 0, sizeof(signal) * filter->taps);
}

/*!
 * @brief           PCMサンプルを予測します。
 * @param *filter   LMSフィルタのハンドル
 * @return          予測されたPCMサンプル
 */
signal lms_predict(lms* filter) {
    register signal sum = 0;
    register uint32_t i;

    for (i = 0; i < filter->taps; ++i) {
        sum += RSHIFT(filter->weights[i] * filter->history[i], filter->shift);  
    }

    return sum;
}

/*!
 * @brief           LMSフィルタを更新します。
 * @param *filter   LMSフィルタのハンドル
 * @param sample    実際のPCMサンプル
 * @param residual  実際のPCMサンプルと、予測されたPCMサンプルの差分
 */
void lms_update(lms* filter, signal sample, signal residual) {
    register uint32_t i;
    register int32_t sgn = SIGN(residual);

    if (filter->taps == 0){
        return;
    }

    for (i = 0; i < filter->taps; ++i) {
        filter->weights[i] += sgn * SIGN(filter->history[i]);
    }

    memmove(&filter->history[1], &filter->history[0], (filter->taps - 1) * sizeof(int32_t));
    filter->history[0] = sample;

    /*for (i = filter->taps - 1; i >= 1; --i) {
        filter->history[i] = filter->history[i - 1];
    }
    filter->history[0] = sample;*/
}