#ifndef PATH_H
#define PATH_H
#define MAX_PATH 260

#if defined(WIN32) || defined(_WIN32) 
#define PATH_SEPARATOR '\\'
#define PATH_SEPARATOR_STR "\\"
#else
#define PATH_SEPARATOR '/'
#define PATH_SEPARATOR_STR "/"
#endif

#include <stdio.h>

/*!
 * @brief           指定されたパスのファイルのバイト数を取得します。
 * @param *path     パス
 * @return          指定されたパスのファイルのバイト数
 */
fpos_t get_file_size(const char* file_name);

/*!
 * @brief           指定されたパスからファイル名部分を取得します。
 * @param *path     パス
 * @return          ファイル名
 */
char* get_file_name(const char* path);

/*!
 * @brief           指定されたパスから、拡張子を除くファイル名部分を取得します。
 * @param *path     パス
 * @return          拡張子を除くファイル名
 */
char* get_file_name_without_extension(const char* path);

/*!
 * @brief           指定されたパスから拡張子部分を取得します。
 * @param *path     パス
 * @return          拡張子
 */
char* get_extension(const char* path);

/*!
 * @brief           指定されたパスからディレクトリ名部分を取得します。
 * @param *path     パス
 * @return          ディレクトリ名部分
 */
char* get_directory_name(const char* path);

/*!
 * @brief               指定されたパスのファイルの拡張子を変更したパスを取得します。
 * @param path          パス
 * @param new_extension 新しい拡張子
 */
void change_extension(char* path, const char* new_extension);

#endif