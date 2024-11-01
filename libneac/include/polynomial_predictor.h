#ifndef POLYNOMIAL_PREDICTOR_HEADER_INCLUDED
#define POLYNOMIAL_PREDICTOR_HEADER_INCLUDED

#include "signal.h"

typedef struct {
    signal* history;
} polynomial_predictor;

/*!
 * @brief               多項式予測器のハンドルを生成します。
 * @return              多項式予測器のハンドル
 */
polynomial_predictor* polynomial_predictor_create();

/*!
 * @brief               多項式予測器を解放します。
 * @param *predictor    多項式予測器のハンドル
 */
void polynomial_predictor_free(polynomial_predictor* predictor);

/*!
 * @brief               多項式予測器を初期化します。
 * @param *predictor    多項式予測器のハンドル
 */
void polynomial_predictor_clear(polynomial_predictor* predictor);

/*!
 * @brief               指定されたハンドルの多項式予測器で、次に続くPCMサンプルを予測します。
 * @param *predictor    多項式予測器のハンドル
 * @return              予測されたPCMサンプル
 */
signal polynomial_predictor_predict(polynomial_predictor* predictor);

/*!
 * @brief               多項式予測器を更新します。
 * @param *predictor    多項式予測器のハンドル
 * @param sample        PCMサンプル
 */
void polynomial_predictor_update(polynomial_predictor* predictor, signal sample);

#endif