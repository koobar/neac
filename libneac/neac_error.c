#include "./include/neac_error.h"

/*!
 * @brief			指定されたエラーコードでエラーをレポートします。
 * @param error		エラーコード
 */
void report_error(error_code error) {
	fprintf(stderr, "[Error] ERROR CODE = %#06x\n", error);
	exit((int)error);
}