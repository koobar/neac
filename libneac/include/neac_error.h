#ifndef NEAC_ERROR_HEADER_INCLUDED
#define NEAC_ERROR_HEADER_INCLUDED

#include <stdbool.h>
#include <stdint.h>

#define NEAC_OK                                                 0x0000      /* エラーではないことを示す */
#define NEAC_ERROR_UNKNOWN                                      0xffff      /* 原因不明のエラーであることを示す */

/* ファイル読み込みエラー */
#define NEAC_ERROR_FILE_ACCESS_FAILED_TO_READ_UINT8             0x0010      /* 8ビット整数の読み込みに失敗した */
#define NEAC_ERROR_FILE_ACCESS_FAILED_TO_READ_UINT16            0x0011      /* 16ビット整数の読み込みに失敗した */
#define NEAC_ERROR_FILE_ACCESS_FAILED_TO_READ_UINT32            0x0012      /* 32ビット整数の読み込みに失敗した */
#define NEAC_ERROR_FILE_ACCESS_FAILED_TO_READ_INT16             0x0013      /* 符号付き16ビット整数の読み込みに失敗した */
#define NEAC_ERROR_FILE_ACCESS_FAILED_TO_READ_INT32             0x0014      /* 符号付き32ビット整数の読み込みに失敗した */
#define NEAC_ERROR_FILE_ACCESS_FAILED_TO_READ_CHAR              0x0015      /* ASCII文字の読み込みに失敗した */
#define NEAC_ERROR_FILE_ACCESS_FAILED_TO_READ_BOOL              0x0016      /* 真偽値の読み込みに失敗した */

/* ファイル書き込みエラー */
#define NEAC_ERROR_FILE_ACCESS_FAILED_TO_WRITE_UINT8            0x0017      /* 8ビット整数の書き込みに失敗した */
#define NEAC_ERROR_FILE_ACCESS_FAILED_TO_WRITE_UINT16           0x0018      /* 16ビット整数の書き込みに失敗した */
#define NEAC_ERROR_FILE_ACCESS_FAILED_TO_WRITE_UINT32           0x0019      /* 32ビット整数の書き込みに失敗した */
#define NEAC_ERROR_FILE_ACCESS_FAILED_TO_WRITE_INT16            0x001a      /* 符号付き16ビット整数の書き込みに失敗した */
#define NEAC_ERROR_FILE_ACCESS_FAILED_TO_WRITE_INT32            0x001b      /* 符号付き32ビット整数の書き込みに失敗した */
#define NEAC_ERROR_FILE_ACCESS_FAILED_TO_WRITE_CHAR             0x001c      /* ASCII文字の書き込みに失敗した */
#define NEAC_ERROR_FILE_ACCESS_FAILED_TO_WRITE_BOOL             0x001d      /* 真偽値の書き込みに失敗した */

/* ビットストリームのエラー */
#define NEAC_ERROR_BIT_STREAM_CANNOT_ALLOCATE_MEMORY            0x0020      /* ビットストリームで必要な領域のメモリアロケーションに失敗した */
#define NEAC_ERROR_BIT_STREAM_NOT_READ_MODE                     0x0021      /* ビットストリームが読み込みモードでないにもかかわらず、読み込みモード専用の関数が呼び出された */
#define NEAC_ERROR_BIT_STREAM_NOT_WRITE_MODE                    0x0022      /* ビットストリームが書き込みモードでないにもかかわらず、書き込みモード専用の関数が呼び出された */

/* LMSフィルタのエラー */
#define NEAC_ERROR_LMS_CANNOT_ALLOCATE_MEMORY                   0x0030      /* LMSフィルタで必要な領域のメモリアロケーションに失敗した */
#define NEAC_ERROR_LMS_INVALID_FILTER_TAPS                      0x0031      /* LMSフィルタのタップ数が有効範囲外であった */
#define NEAC_ERROR_LMS_UNSUPPORTED_PCM_BITS                     0x0032      /* LMSフィルタで処理しようとした信号が対応していない量子化ビット数であった */

/* 多項式予測器のエラー */
#define NEAC_ERROR_POLYNOMIAL_PREDICTOR_CANNOT_ALLOCATE_MEMORY  0x0040  /* 多項式予測器で必要な領域のメモリアロケーションに失敗した */

/* ブロックのエラー */
#define NEAC_ERROR_BLOCK_CANNOT_ALLOCATE_MEMORY                 0x0050      /* ブロックで必要な領域のメモリアロケーションに失敗した */

/* サブブロックのエラー */
#define NEAC_ERROR_SUB_BLOCK_CANNOT_ALLOCATE_MEMORY             0x0060      /* サブブロックで必要な領域のメモリアロケーションに失敗した */

/* エントロピー符号化のエラー */
#define NEAC_ERROR_RICE_CODING_INVALID_PARAMETER             0x0070      /* エントロピー符号化に用いられたパラメータが不正なパラメータであった */

/* デコードエラー */
#define NEAC_ERROR_DECODER_CANNOT_ALLOCATE_MEMORY               0x0080      /* デコーダで必要な領域のメモリアロケーションに失敗した */
#define NEAC_ERROR_DECODER_FAILED_TO_OPEN_FILE                  0x0081      /* デコードしようとしたファイルを開けなかった */
#define NEAC_ERROR_DECODER_INVALID_MAGIC_NUMBER                 0x0082      /* デコードしようとしたファイルのマジックナンバーがNEACのものではなかった */
#define NEAC_ERROR_DECODER_UNSUPPORTED_FORMAT_VERSION           0x0083      /* デコードしようとしたファイルに含まれているNEACデータのバージョンがサポート対象外のバージョンであった */

/* エンコードエラー */
#define NEAC_ERROR_ENCODER_CANNOT_ALLOCATE_MEMORY               0x0090      /* エンコーダーで必要な領域のメモリアロケーションに失敗した */
#define NEAC_ERROR_ENCODER_FAILED_TO_OPEN_FILE                  0x0091      /* エンコード先ファイルをバイナリ書き込み(wb)モードで開けなかった */

typedef uint16_t error_code;

/*!
 * @brief           指定されたエラーコードでエラーをレポートします。
 * @param error     エラーコード
 */
void report_error(error_code error);

/*!
 * @brief           最後に発生したエラーのエラーコードを取得します。
 * @return          エラーコード
 */
error_code get_last_error_code();

/*!
 * @brief           最後に発生したエラーのエラーコードを取得します。
 * @param value     trueならエラー発生時に即座にプログラムを終了します。falseなら、プログラムを終了しません。
 */
void set_on_error_exit(bool value);

#endif