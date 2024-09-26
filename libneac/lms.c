#include "./include/lms.h"

#define LMS_SUM_SHIFT_FOR_PCM16 9
#define LMS_SUM_SHIFT_FOR_PCM24 11
#define SIGN(value) ((value > 0) - (value < 0))

/* 24ビット符号付きPCM信号の処理時における重み更新処理において使用する、予測残差の右シフトビット数のテーブル */
static const int32_t residual_shift_table_for_pcm24[LMS_MAX_TAPS] = {
    20, 20, 20, 20,
    20, 20, 20, 20,
    21, 21, 21, 21,
    21, 21, 21, 21,
    22, 22, 22, 22,
    22, 22, 22, 22,
    22, 22, 22, 22,
    22, 22, 22, 22,
};

/*!
 * @brief           LMSフィルタを初期化します。
 * @param filter    LMSフィルタのハンドル
 * @param pcm_bits  処理するPCM信号の量子化ビット数
 * @param taps      LMSフィルタのタップ数
 */
static void lms_init(lms* filter, uint8_t pcm_bits, uint8_t taps) {
    uint32_t i;

    filter->pcm_bits = pcm_bits;

    /* タップ数を反映 */
    if (taps > LMS_MAX_TAPS) {
        report_error(NEAC_ERROR_LMS_INVALID_FILTER_TAPS);
        return;
    }
    filter->taps = taps;

    /* PCMの量子化ビット数に応じたシフト数を選択する */
    if (pcm_bits == 16) {
        filter->shift = LMS_SUM_SHIFT_FOR_PCM16;
    }
    else if (pcm_bits == 24) {
        filter->shift = LMS_SUM_SHIFT_FOR_PCM24;
    }
    else {
        report_error(NEAC_ERROR_LMS_UNSUPPORTED_PCM_BITS);
        return;
    }

    /* 過去サンプルと重み係数の領域を確保 */
    filter->history = (signal*)malloc(sizeof(signal) * filter->taps);
    filter->weights = (signal*)malloc(sizeof(signal) * filter->taps);

    if (filter->history == NULL || filter->weights == NULL) {
        report_error(NEAC_ERROR_LMS_CANNOT_ALLOCATE_MEMORY);
        return;
    }

    /* 過去サンプルと重み係数の領域をゼロで初期化 */
    for (i = 0; i < filter->taps; ++i) {
        filter->history[i] = 0;
        filter->weights[i] = 0;
    }
}

/*!
 * @brief           LMSフィルタのハンドルを生成します。
 * @param pcm_bits  処理するPCM信号の量子化ビット数
 * @param taps      LMSフィルタのタップ数
 * @return			LMSフィルタのハンドル
 */
lms* lms_create(uint8_t pcm_bits, uint8_t taps) {
    lms* result = (lms*)malloc(sizeof(lms));

    if (result == NULL){
        report_error(NEAC_ERROR_LMS_CANNOT_ALLOCATE_MEMORY);
        return NULL;
    }

    lms_init(result, pcm_bits, taps);
    return result;
}

/*!
 * @brief           LMSフィルタを解放します。
 * @param filter    LMSフィルタのハンドル
 */
void lms_free(lms* filter) {
    free(filter->history);
    free(filter->weights);
}

/*!
 * @brief           PCMサンプルを予測します。
 * @param filter    LMSフィルタのハンドル
 * @return          予測されたPCMサンプル
 */
signal lms_predict(lms* filter) {
    register signal sum = 0;
    register uint32_t i;

    for (i = 0; i < filter->taps; ++i) {
        sum += filter->weights[i] * filter->history[i];
    }

    return sum >> filter->shift;
}

/*!
 * @brief           Sign-Sign LMSアルゴリズムを用いて、重みの更新を行います。(16ビットPCM処理時専用)
 * @param *lms      LMSフィルタのハンドル
 * @param sample    サンプル
 * @param residual  予測残差
*/
static void lms_update_pcm16(lms* filter, signal sample, signal residual) {
    register uint32_t i;

    /* 重みの更新 (Sign-Sign LMS) */
    for (i = 0; i < filter->taps; ++i) {
        filter->weights[i] += SIGN(residual) * SIGN(filter->history[i]);
    }
}

/*!
 * @brief           Sign-Sign LMSアルゴリズムを用いて、重みの更新を行います。(24ビットPCM処理時専用)
 * @param *lms      LMSフィルタのハンドル
 * @param sample    サンプル
 * @param residual  予測残差
*/
static void lms_update_pcm24(lms* filter, signal sample, signal residual) {
    register uint32_t i;

    /* 重みの更新 (Sign-Sign LMS) */
    for (i = 0; i < filter->taps; ++i) {
        filter->weights[i] += (residual >> residual_shift_table_for_pcm24[i]) * SIGN(filter->history[i]);
    }
}

/*!
 * @brief           LMSフィルタを更新します。
 * @param filter    適応フィルタ
 * @param sample    実際のPCMサンプル
 * @param residual  実際のPCMサンプルと、予測されたPCMサンプルの差分
 */
void lms_update(lms* filter, signal sample, signal residual) {
    register int32_t cnt;

    switch (filter->pcm_bits) {
    case 16:
        lms_update_pcm16(filter, sample, residual);
        break;
    case 24:
        lms_update_pcm24(filter, sample, residual);
        break;
    }

    /* 過去サンプルの更新 */
    for (cnt = filter->taps - 1; cnt >= 1; --cnt) {
        filter->history[cnt] = filter->history[cnt - 1];
    }
    filter->history[0] = sample;
}