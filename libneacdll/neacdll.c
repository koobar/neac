#include "./include/neacdll.h"

/*!
 * @brief       最後に発生したエラーのエラーコードを取得します。
 * @return      最後に発生したエラーのエラーコード
 */
error_code GetLastErrorCode() {
    return get_last_error_code();
}