#ifndef FILE_ACCESS_HEADER_INCLUDED
#define FILE_ACCESS_HEADER_INCLUDED

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

/*!
 * @brief       指定されたファイルポインタが示すファイルから、真偽値を読み込みます。
 * @param file  ファイルポインタ
 * @return      真偽値
 */
bool read_bool(FILE* file);

/*!
 * @brief       指定されたファイルポインタが示すファイルから16ビット符号付き整数を読み込みます。
 * @param file  ファイルポインタ
 * @return      読み込まれた値
 */
int16_t read_int16(FILE* file);

/*!
 * @brief       指定されたファイルポインタが示すファイルから32ビット符号付き整数を読み込みます。
 * @param file  ファイルポインタ
 * @return      読み込まれた値
 */
int32_t read_int32(FILE* file);

/*!
 * @brief       指定されたファイルポインタが示すファイルからASCII文字を読み込みます。
 * @param file  ファイルポインタ
 * @return      読み込まれた値
 */
char read_char(FILE* file);

/*!
 * @brief       指定されたファイルポインタが示すファイルから文字列を読み込みます。
 * @param file  ファイルポインタ
 * @return      読み込まれた値
 */
const char* read_string(FILE* file, uint32_t bytes);

/*!
 * @brief       指定されたファイルポインタが示すファイルから8ビット整数を読み込みます。
 * @param file  ファイルポインタ
 * @return      読み込まれた値
 */
uint8_t read_uint8(FILE* file);

/*!
 * @brief       指定されたファイルポインタが示すファイルから16ビット整数を読み込みます。
 * @param file  ファイルポインタ
 * @return      読み込まれた値
 */
uint16_t read_uint16(FILE* file);

/*!
 * @brief       指定されたファイルポインタが示すファイルから32ビット整数を読み込みます。
 * @param file  ファイルポインタ
 * @return      読み込まれた値
 */
uint32_t read_uint32(FILE* file);

/*!
 * @brief       指定されたファイルポインタが示すファイルに真偽値を書き込む。
 * @param file  ファイルポインタ
 * @param value 書き込む値
 */
void write_bool(FILE* file, const bool value);

/*!
 * @brief       指定されたファイルポインタが示すファイルにASCII文字を書き込む。
 * @param file  ファイルポインタ
 * @param value 書き込む値
 */
void write_char(FILE* file, const char value);

/*!
 * @brief       指定されたファイルポインタが示すファイルに文字列を書き込む。
 * @param file  ファイルポインタ
 * @param value 書き込む値
 */
void write_string(FILE* file, const char* value, uint32_t bytes);

/*!
 * @brief       指定されたファイルポインタが示すファイルに8ビット整数を書き込む。
 * @param file  ファイルポインタ
 * @param value 書き込む値
 */
void write_uint8(FILE* file, const uint8_t value);

/*!
 * @brief       指定されたファイルポインタが示すファイルに16ビット整数を書き込む。
 * @param file  ファイルポインタ
 * @param value 書き込む値
 */
void write_uint16(FILE* file, const uint16_t value);

/*!
 * @brief       指定されたファイルポインタが示すファイルに32ビット整数を書き込む。
 * @param file  ファイルポインタ
 * @param value 書き込む値
 */
void write_uint32(FILE* file, const uint32_t value);

/*!
 * @brief       指定されたファイルポインタが示すファイルに16符号付きビット整数を書き込む。
 * @param file  ファイルポインタ
 * @param value 書き込む値
 */
void write_int16(FILE* file, const int16_t value);

/*!
 * @brief       指定されたファイルポインタが示すファイルに32ビット符号付整数を書き込む。
 * @param file  ファイルポインタ
 * @param value 書き込む値
 */
void write_int32(FILE* file, const int32_t value);

#endif