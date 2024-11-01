#ifndef NEACENC_HEADER_INCLUDED
#define NEACENC_HEADER_INCLUDED

#include "neac_encoder.h"
#include <Windows.h>

typedef neac_encoder* HENCODER;

/*!
 * @brief                           指定された設定で、エンコーダのハンドルを生成します。
 * @param output                    出力先のファイルのパス
 * @param sample_rate               サンプリング周波数
 * @param bits_per_sample           量子化ビット数
 * @param num_channels              チャンネル数
 * @param num_samples               合計サンプル数
 * @param block_size                ブロックサイズ
 * @param use_mid_side_stereo       ミッドサイドステレオを使用するかどうかを示すフラグ
 * @param filter_taps               LMSフィルタのタップ数
 * @param *tag                      タグ情報
 */
HENCODER __declspec(dllexport) CreateEncoderFromPath(
    LPCSTR output,
    uint32_t sample_rate,
    uint8_t bits_per_sample,
    uint8_t num_channels,
    uint32_t num_samples,
    uint16_t block_size,
    bool use_mid_side_stereo,
    uint8_t filter_taps,
    neac_tag* tag);

/*!
 * @brief                           指定された設定で、エンコーダのハンドルを生成します。
 * @param *file                     出力先のファイルハンドル
 * @param sample_rate               サンプリング周波数
 * @param bits_per_sample           量子化ビット数
 * @param num_channels              チャンネル数
 * @param num_samples               合計サンプル数
 * @param block_size                ブロックサイズ
 * @param use_mid_side_stereo       ミッドサイドステレオを使用するかどうかを示すフラグ
 * @param filter_taps               LMSフィルタのタップ数
 * @param *tag                      タグ情報
 */
HENCODER __declspec(dllexport) CreateEncoderFromFile(
    FILE* file,
    uint32_t sample_rate,
    uint8_t bits_per_sample,
    uint8_t num_channels,
    uint32_t num_samples,
    uint16_t block_size,
    bool use_mid_side_stereo,
    uint8_t filter_taps,
    neac_tag* tag);

/*!
 * @brief           エンコーダを解放します。
 * @param encoder   エンコーダのハンドル
 */
void __declspec(dllexport) FreeEncoder(HENCODER encoder);

/*!
 * @brief           指定されたハンドルのエンコーダで、指定されたサンプルをエンコードします。
 * @param encoder   エンコーダのハンドル
 * @param sample    サンプル
 */
void __declspec(dllexport) EncoderWriteSample(HENCODER encoder, int32_t sample);

/*!
 * @brief           指定されたハンドルのエンコーダでのエンコードの終了処理を行います。すべてのサンプルのエンコードが終了した後に、必ず呼び出してください。
 * @param encoder   エンコーダのハンドル
 */
void __declspec(dllexport) EncoderEndWrite(HENCODER encoder);

#endif
