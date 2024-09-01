#include "./include/simple_predictor.h"

/*!
 * @brief		指定されたハンドルのシンプル予測器を初期化します。
 */
static void simple_predictor_init(simple_predictor* predictor) {
	predictor->p1 = 0;
	predictor->p2 = 0;
}

/*!
 * @brief		シンプル予測器のハンドルを生成します。
 * @return		シンプル予測器のハンドル
 */
simple_predictor* simple_predictor_create() {
	simple_predictor* result = (simple_predictor*)malloc(sizeof(simple_predictor));

	if (result == NULL) {
		report_error(NEAC_ERROR_SIMPLE_PREDICTOR_CANNOT_ALLOCATE_MEMORY);
	}

	simple_predictor_init(result);
	return result;
}

/*!
 * @brief				シンプルフ予測器を初期化します。
 * @param *predictor	シンプル予測器のハンドル
 */
void simple_predictor_reset(simple_predictor* predictor) {
	predictor->p1 = 0;
	predictor->p2 = 0;
}

/*!
 * @brief				指定されたハンドルのシンプル予測器で、次に続くPCMサンプルを予測します。
 * @param *predictor	シンプル予測器のハンドル
 * @return				予測されたPCMサンプル
 */
signal simple_predictor_predict(simple_predictor* predictor) {
	return (predictor->p1 << 1) - predictor->p2;
}

/*!
 * @brief				シンプル予測器を更新します。
 * @param *predictor	シンプル予測器のハンドル
 * @param sample		PCMサンプル
 */
void simple_predictor_update(simple_predictor* predictor, signal sample) {
	predictor->p2 = predictor->p1;
	predictor->p1 = sample;
}