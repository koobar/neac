#include "./include/neac_error.h"
#include <stdio.h>
#include <stdlib.h>

static bool on_error_exit = true;
static error_code last_error_code = NEAC_OK;

/*!
 * @brief           指定されたエラーコードでエラーをレポートします。
 * @param error     エラーコード
 */
void report_error(error_code error) {
    last_error_code = error;
    fprintf(stderr, "[Error] ERROR CODE = %#06x\n", error);

    if (on_error_exit) {    
        exit((int)error);
    }
}

/*!
 * @brief           最後に発生したエラーのエラーコードを取得します。
 * @return          エラーコード
 */
error_code get_last_error_code() {
    return last_error_code;
}

/*!
 * @brief           最後に発生したエラーのエラーコードを取得します。
 * @param value     trueならエラー発生時に即座にプログラムを終了します。falseなら、プログラムを終了しません。
 */
void set_on_error_exit(bool value) {
    on_error_exit = value;
}
