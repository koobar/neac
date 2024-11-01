#ifndef LMS_HEADER_INCLUDED
#define LMS_HEADER_INCLUDED

#define LMS_MAX_TAPS    32

#include "signal.h"
#include <stdint.h>

typedef struct {
    uint32_t taps;              /* 予測次数 */
    uint8_t shift;              /* シフト係数 */
    signal* history;            /* 過去サンプル領域のポインタ */
    signal* weights;            /* 重み係数領域のポインタ */
} lms;

/*!
 * @brief           LMSフィルタのハンドルを生成します。
 * @return          LMSフィルタのハンドル
 */
lms* lms_create(uint8_t taps, uint8_t pcm_bits);

/*!
 * @brief           LMSフィルタを解放します。
 * @param *filter   LMSフィルタのハンドル
 */
void lms_free(lms* filter);

/*!
 * @brief           LMSフィルタを初期化します。
 * @param *filter   LMSフィルタのハンドル
 */
void lms_clear(lms* filter);

/*!
 * @brief           PCMサンプルを予測します。
 * @param *filter   LMSフィルタのハンドル
 * @return          予測されたPCMサンプル
 */
signal lms_predict(lms* filter);

/*!
 * @brief           LMSフィルタを更新します。
 * @param *filter   LMSフィルタのハンドル
 * @param sample    実際のPCMサンプル
 * @param residual  実際のPCMサンプルと、予測されたPCMサンプルの差分
 */
void lms_update(lms* filter, signal sample, signal residual);

#endif