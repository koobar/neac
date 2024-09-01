#ifndef LMS_H
#define LMS_H

#define LMS_MAX_TAPS 32

#include "neac.h"
#include "neac_error.h"
#include "signal.h"
#include <stdint.h>
#include <stdlib.h>

typedef struct lms {
	uint32_t taps;							/* 予測次数 */
	int32_t shift;							/* 重み係数と過去サンプルを掛けた値の合計の右シフト数 */
	uint8_t pcm_bits;						/* 処理するPCMのビット数 */
	signal* history;						/* 過去サンプル領域のポインタ */
	signal* weights;						/* 重み係数領域のポインタ */
} lms;

/*!
 * @brief			LMSフィルタのハンドルを生成します。
 * @param pcm_bits	処理するPCM信号の量子化ビット数
 * @param taps		LMSフィルタのタップ数
 * @return			LMSフィルタのハンドル
 */
lms* lms_create(uint8_t pcm_bits, uint8_t taps);

/*!
 * @brief			LMSフィルタを解放します。
 * @param filter	LMSフィルタのハンドル
 */
void lms_free(lms* filter);

/*!
 * @brief			PCMサンプルを予測します。
 * @param filter	LMSフィルタのハンドル
 * @return			予測されたPCMサンプル
 */
signal lms_predict(lms* filter);

/*!
 * @brief			LMSフィルタを更新します。
 * @param filter	適応フィルタ
 * @param sample	実際のPCMサンプル
 * @param residual	実際のPCMサンプルと、予測されたPCMサンプルの差分
 */
void lms_update(lms* filter, signal sample, signal residual);

#endif