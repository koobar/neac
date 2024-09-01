#ifndef SIMPLE_PREDICTOR_H
#define SIMPLE_PREDICTOR_H

#include "neac_error.h"
#include "signal.h"
#include <stdint.h>

typedef struct simple_predictor {
	signal p1;				/* 1つ前のサンプル */
	signal p2;				/* 2つ前のサンプル */
} simple_predictor;

/*!
 * @brief			シンプル予測器のハンドルを生成します。
 * @return			シンプル予測器のハンドル
 */
simple_predictor* simple_predictor_create();

/*!
 * @brief			シンプル予測器を初期化します。
 * @param *filter	シンプル予測器のハンドル
 */
void simple_predictor_reset(simple_predictor* predictor);

/*!
 * @brief			指定されたハンドルのシンプル予測器で、次に続くPCMサンプルを予測します。
 * @param *filter	シンプル予測器のハンドル
 * @return			予測されたPCMサンプル
 */
signal simple_predictor_predict(simple_predictor* predictor);

/*!
 * @brief			シンプル予測器を更新します。
 * @param *filter	シンプル予測器のハンドル
 * @param sample	PCMサンプル
 */
void simple_predictor_update(simple_predictor* predictor, signal sample);

#endif