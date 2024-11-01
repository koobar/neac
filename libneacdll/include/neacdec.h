#ifndef NEACDEC_HEADER_INCLUDED
#define NEACDEC_HEADER_INCLUDED

#include "neac_decoder.h"
#include <Windows.h>

typedef neac_decoder *HDECODER;
typedef uint8_t* BYTEPTR;

/*!
* @brief            指定されたパスのファイルをデコードするデコーダのハンドルを返します。
* @param path       開くファイルのパス
* @return           デコーダのハンドル
*/
HDECODER __declspec(dllexport) CreateDecoder(LPCSTR path);

/*!
* @brief            指定されたハンドルのデコーダを解放します。
* @param decoder    デコーダのハンドル
*/
void __declspec(dllexport) FreeDecoder(HDECODER decoder);


/*!
* @brief            指定されたハンドルのデコーダで開かれているファイルを閉じます。
* @param decoder    デコーダのハンドル
*/
void __declspec(dllexport) DecoderCloseFile(HDECODER decoder);

/*!
* @brief            指定されたハンドルのデコーダで開かれているファイルのサンプリング周波数を取得します。
* @param decoder    デコーダのハンドル
* @return           サンプリング周波数
*/
uint32_t __declspec(dllexport) DecoderGetSampleRate(HDECODER decoder);

/*!
* @brief            指定されたハンドルのデコーダで開かれているファイルのチャンネル数を取得します。
* @param decoder    デコーダのハンドル
* @return           チャンネル数
*/
uint32_t __declspec(dllexport) DecoderGetChannels(HDECODER decoder);

/*!
* @brief            指定されたハンドルのデコーダで開かれているファイルの量子化ビット数を取得します。
* @param decoder    デコーダのハンドル
*/
uint32_t __declspec(dllexport) DecoderGetBitsPerSample(HDECODER decoder);

/*!
* @brief            指定されたハンドルのデコーダで開かれているファイルのサンプル数を取得します。
* @param decoder    デコーダのハンドル
* @return           サンプル数
*/
uint32_t __declspec(dllexport) DecoderGetNumTotalSamples(HDECODER decoder);

/*!
* @brief            指定されたハンドルのデコーダで開かれているファイルのブロックのサイズを取得します。
* @param decoder    デコーダのハンドル
* @return           ブロックのサイズ
*/
uint16_t __declspec(dllexport) DecoderGetBlockSize(HDECODER decoder);

/*!
* @brief            指定されたハンドルのデコーダで開かれているファイルのブロック数を取得します。
* @param decoder    デコーダのハンドル
* @return           ブロック数
*/
uint32_t __declspec(dllexport) DecoderGetNumBlocks(HDECODER decoder);

/*!
* @brief            指定されたハンドルのデコーダで開かれているファイルで、ミッドサイドステレオが使用されているかどうかを示すフラグを取得します。
* @param decoder    デコーダのハンドル
* @return           ミッドサイドステレオが使用されていればtrueを、使用されていなければfalseを返します。
*/
bool __declspec(dllexport) DecoderGetUseMidSideStereo(HDECODER decoder);

/*!
* @brief            指定されたハンドルのデコーダで開かれているファイルから1サンプル読み込みます。
* @param decoder    デコーダのハンドル
* @return           読み込んだサンプル。戻り値の型はint32_tですが、実際のビット数は量子化ビット数と同じとなります。
*/
int32_t __declspec(dllexport) DecoderReadSample(HDECODER decoder);

/*!
* @brief            指定されたハンドルのデコーダで開かれているファイルで、デコードするサンプルのオフセットを設定します。
* @param decoder    デコーダのハンドル
* @param offset     サンプルのオフセット
*/
void __declspec(dllexport) DecoderSetSampleOffset(HDECODER decoder, uint32_t offset);

/*!
* @brief            指定されたハンドルのデコーダで開かれているファイルの、デコード中のサンプルのオフセットを取得します。
* @param decoder    デコーダのハンドル
* @return           サンプルのオフセット
*/
uint32_t __declspec(dllexport) DecoderGetSampleOffset(HDECODER decoder);

/*!
* @brief            指定されたハンドルのデコーダで開かれているファイルを、ミリ秒単位で指定された時間にシークします。
* @param decoder    デコーダのハンドル
* @param msec       シーク先時間（ミリ秒単位）
*/
void __declspec(dllexport) DecoderSeekTo(HDECODER decoder, uint32_t msec);

/*!
* @brief            指定されたハンドルのデコーダで開かれているファイルの演奏時間をミリ秒単位で取得します。
* @param decoder    デコーダのハンドル
* @return           デコーダで開かれているファイルの演奏時間（ミリ秒単位）
*/
uint32_t __declspec(dllexport) DecoderGetDurationMsec(HDECODER decoder);

/*!
* @brief            指定されたハンドルのデコーダで開かれているファイルに、タグ情報が含まれているかどうかを判定します。
* @param decoder    デコーダのハンドル
* @return           タグ情報が含まれていればtrueを、含まれていなければfalseを返します。
*/
bool __declspec(dllexport) DecoderContainsTagInfo(HDECODER decoder);

/*!
* @brief            指定されたハンドルのデコーダで開かれているファイルに含まれるタグ情報から、タイトルを取得します。
* @param decoder    デコーダのハンドル
* @return           タグに設定されたタイトルを返します。タグが含まれていない場合や、設定されていない場合、\0を返します。
*/
LPCSTR __declspec(dllexport) DecoderGetTitle(HDECODER decoder);

/*!
* @brief            指定されたハンドルのデコーダで開かれているファイルに含まれるタグ情報から、アルバム名を取得します。
* @param decoder    デコーダのハンドル
* @return           タグに設定されたアルバム名を返します。タグが含まれていない場合や、設定されていない場合、\0を返します。
*/
LPCSTR __declspec(dllexport) DecoderGetAlbum(HDECODER decoder);

/*!
* @brief            指定されたハンドルのデコーダで開かれているファイルに含まれるタグ情報から、アーティスト名を取得します。
* @param decoder    デコーダのハンドル
* @return           タグに設定されたアーティスト名を返します。タグが含まれていない場合や、設定されていない場合、\0を返します。
*/
LPCSTR __declspec(dllexport) DecoderGetArtist(HDECODER decoder);

/*!
* @brief            指定されたハンドルのデコーダで開かれているファイルに含まれるタグ情報から、アルバムアーティストを取得します。
* @param decoder    デコーダのハンドル
* @return           タグに設定されたアルバムアーティストを返します。タグが含まれていない場合や、設定されていない場合、\0を返します。
*/
LPCSTR __declspec(dllexport) DecoderGetAlbumArtist(HDECODER decoder);

/*!
* @brief            指定されたハンドルのデコーダで開かれているファイルに含まれるタグ情報から、サブタイトルを取得します。
* @param decoder    デコーダのハンドル
* @return           タグに設定されたサブタイトルを返します。タグが含まれていない場合や、設定されていない場合、\0を返します。
*/
LPCSTR __declspec(dllexport) DecoderGetSubtitle(HDECODER decoder);

/*!
* @brief            指定されたハンドルのデコーダで開かれているファイルに含まれるタグ情報から、発行者を取得します。
* @param decoder    デコーダのハンドル
* @return           タグに設定された発行者を返します。タグが含まれていない場合や、設定されていない場合、\0を返します。
*/
LPCSTR __declspec(dllexport) DecoderGetPublisher(HDECODER decoder);

/*!
* @brief            指定されたハンドルのデコーダで開かれているファイルに含まれるタグ情報から、作曲者を取得します。
* @param decoder    デコーダのハンドル
* @return           タグに設定された作曲者を返します。タグが含まれていない場合や、設定されていない場合、\0を返します。
*/
LPCSTR __declspec(dllexport) DecoderGetComposer(HDECODER decoder);

/*!
* @brief            指定されたハンドルのデコーダで開かれているファイルに含まれるタグ情報から、作詞者を取得します。
* @param decoder    デコーダのハンドル
* @return           タグに設定された作詞者を返します。タグが含まれていない場合や、設定されていない場合、\0を返します。
*/
LPCSTR __declspec(dllexport) DecoderGetSongwriter(HDECODER decoder);

/*!
* @brief            指定されたハンドルのデコーダで開かれているファイルに含まれるタグ情報から、指揮者を取得します。
* @param decoder    デコーダのハンドル
* @return           タグに設定された指揮者を返します。タグが含まれていない場合や、設定されていない場合、\0を返します。
*/
LPCSTR __declspec(dllexport) DecoderGetConductor(HDECODER decoder);

/*!
* @brief            指定されたハンドルのデコーダで開かれているファイルに含まれるタグ情報から、著作権表示を取得します。
* @param decoder    デコーダのハンドル
* @return           タグに設定された著作権表示を返します。タグが含まれていない場合や、設定されていない場合、\0を返します。
*/
LPCSTR __declspec(dllexport) DecoderGetCopyright(HDECODER decoder);

/*!
* @brief            デコーダで開かれているファイルに含まれるタグ情報から、ジャンル名を取得します。
* @param decoder    デコーダのハンドル
* @return           タグに設定されたジャンル名を返します。タグが含まれていない場合や、設定されていない場合、\0を返します。
*/
LPCSTR __declspec(dllexport) DecoderGetGenre(HDECODER decoder);

/*!
* @brief            指定されたハンドルのデコーダで開かれているファイルに含まれるタグ情報から、コメントを取得します。
* @param decoder    デコーダのハンドル
* @return           タグに設定されたコメントを返します。タグが含まれていない場合や、設定されていない場合、\0を返します。
*/
LPCSTR __declspec(dllexport) DecoderGetComment(HDECODER decoder);

/*!
* @brief            指定されたハンドルのデコーダで開かれているファイルに含まれるタグ情報から、年を取得します。
* @param decoder    デコーダのハンドル
* @return           タグに設定された年を返します。タグが含まれていない場合や、設定されていない場合、0を返します。
*/
uint16_t __declspec(dllexport) DecoderGetYear(HDECODER decoder);

/*!
* @brief            指定されたハンドルのデコーダで開かれているファイルに含まれるタグ情報から、トラック番号を取得します。
* @param decoder    デコーダのハンドル
* @return           タグに設定されたトラック番号を返します。タグが含まれていない場合や、設定されていない場合、0を返します。
*/
uint16_t __declspec(dllexport) DecoderGetTrackNumber(HDECODER decoder);

/*!
* @brief            指定されたハンドルのデコーダで開かれているファイルに含まれるタグ情報から、トラック数を取得します。
* @param decoder    デコーダのハンドル
* @return           タグに設定されたトラック数を返します。タグが含まれていない場合や、設定されていない場合、0を返します。
*/
uint16_t __declspec(dllexport) DecoderGetTrackCount(HDECODER decoder);

/*!
* @brief            指定されたハンドルのデコーダで開かれているファイルに含まれるタグ情報から、ディスク番号を取得します。
* @param decoder    デコーダのハンドル
* @return           タグに設定されたディスク番号を返します。タグが含まれていない場合や、設定されていない場合、0を返します。
*/
uint16_t __declspec(dllexport) DecoderGetDisc(HDECODER decoder);

/*!
* @brief            指定されたハンドルのデコーダで開かれているファイルに含まれるタグ情報から、レーティングを取得します。
* @param decoder    デコーダのハンドル
* @return           タグに設定されたレーティングを返します。タグが含まれていない場合や、設定されていない場合、0を返します。
*/
uint16_t __declspec(dllexport) DecoderGetRating(HDECODER decoder);

/*!
* @brief            指定されたハンドルのデコーダで開かれているファイルに含まれるタグ情報から、カバー画像のサイズを取得します。
* @param decoder    デコーダのハンドル
* @return           タグに設定されたカバー画像データのサイズを返します。タグが含まれていない場合や、設定されていない場合、0を返します。
*/
uint32_t __declspec(dllexport) DecoderGetPictureSize(HDECODER decoder);

/*!
* @brief            指定されたハンドルのデコーダで開かれているファイルに含まれるタグ情報から、カバー画像のフォーマットコードを取得します。
* @param decoder    デコーダのハンドル
* @return           タグに設定されたカバー画像データのフォーマットコードを返します。タグが含まれていない場合や、設定されていない場合、0を返します。
*/
uint8_t __declspec(dllexport) DecoderGetPictureFormatCode(HDECODER decoder);

/*!
* @brief            指定されたハンドルのデコーダで開かれているファイルに含まれるタグ情報から、カバー画像を取得します。
* @param decoder    デコーダのハンドル
* @return           タグに設定されたカバー画像データが格納された領域へのポインタを返します。タグが含まれていない場合や、設定されていない場合、NULLを返します。
*/
BYTEPTR __declspec(dllexport) DecoderGetPicture(HDECODER decoder);

#endif