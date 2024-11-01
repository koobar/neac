#include "./include/path.h"
#include <stdlib.h>
#include <string.h>

/*!
 * @brief           指定されたパスのファイルのバイト数を取得します。
 * @param *path     パス
 * @return          指定されたパスのファイルのバイト数
 */
fpos_t get_file_size(const char* file_name){
    fpos_t fsize = 0;
    FILE* fp;
    errno_t err;

    err = fopen_s(&fp, file_name, "rb");
    if (err != 0) {
        return 0;
    }

    /* ファイルサイズを取得 */
    fseek(fp, 0, SEEK_END);
    fgetpos(fp, &fsize);

    /* ファイルサイズを取得するために開いただけなので、ここで閉じてしまってもよい */
    fclose(fp);

    return fsize;
}

/*!
 * @brief           指定されたパスからファイル名部分を取得します。
 * @param *path     パス
 * @return          ファイル名
 */
char* get_file_name(const char* path) {
    char* file_name_ptr;
    char* result;
    size_t file_name_size;

    file_name_ptr = strrchr(path, PATH_SEPARATOR) + 1;
    file_name_size = strlen(file_name_ptr) + 1;

    result = (char*)malloc(sizeof(char) * file_name_size);
    snprintf(result, file_name_size, "%s", file_name_ptr);

    return result;
}

/*!
 * @brief           指定されたパスから、拡張子を除くファイル名部分を取得します。
 * @param *path     パス
 * @return          拡張子を除くファイル名
 */
char* get_file_name_without_extension(const char* path) {
    char* name = get_file_name(path);
    size_t i;
    size_t len = strlen(name);

    for (i = len - 1; i != 0; --i) {
        if (name[i] == '.') {
            name[i] = '\0';
            break;
        }
    }

    return name;
}

/*!
 * @brief           指定されたパスから拡張子部分を取得します。
 * @param *path     パス
 * @return          拡張子
 */
char* get_extension(const char* path) {
    const char* dot = NULL;
    char* result;
    size_t i, extension_size;
    size_t len = strlen(path);

    /* 引数に与えられたパスのうち、拡張子の開始位置のポインタを取得する */
    for (i = len - 1; i != 0; --i) {
        if (path[i] == (char)'.') {
            dot = &path[i];
            break;
        }
    }

    /* ポインタの取得失敗（拡張子がみつからない）ならNULLを返す */
    if (dot == NULL) {
        return NULL;
    }

    /* 拡張子の領域を確保 */
    extension_size = strlen(dot);
    result = (char*)malloc(sizeof(char) * extension_size);

    /* 拡張子領域の確保に失敗した場合はNULLを返す */
    if (result == NULL) {
        return NULL;
    }

    /* 拡張子部分を拡張子の領域にコピー */
    for (i = 0; i < extension_size; ++i) {
        result[i] = dot[i];
    }
    result[extension_size] = '\0';

    return result;
}

/*!
 * @brief           指定されたパスからディレクトリ名部分を取得します。
 * @param *path     パス
 * @return          ディレクトリ名部分
 */
char* get_directory_name(const char* path){
    size_t size = strlen(path), directory_name_end_offset = 0, directory_name_size, i;
    char* result;

    /* ディレクトリ名部分の終了位置を取得 */
    for (i = size - 1; i != 0; --i) {
        if (path[i] == PATH_SEPARATOR) {
            directory_name_end_offset = i;
            break;
        }
    }

    /* ディレクトリ名部分を格納する領域を確保 */
    directory_name_size = directory_name_end_offset + 1;
    result = (char*)malloc(directory_name_size);

    /* 確保できていなければNULLを返す */
    if (result == NULL) {
        return NULL;
    }

    /* ディレクトリ名部分をディレクトリ名領域にコピー */
    for (i = 0; i < directory_name_size - 1; ++i) {
        result[i] = path[i];
    }
    result[directory_name_size - 1] = '\0';

    return result;
}

/*!
 * @brief               指定されたパスのファイルの拡張子を変更したパスを取得します。
 * @param path          パス
 * @param new_extension 新しい拡張子
 */
void change_extension(char* path, const char* new_extension) {
    char* dot = NULL;
    size_t len = strlen(path);
    size_t i;

    for (i = len; i > 0; --i) {
        if (path[i - 1] == '.') {
            dot = &path[i - 1];
            break;
        }
    }

    if (dot != NULL) {
        // 既存の拡張子を置き換える
        strncpy(dot, new_extension, strlen(new_extension) + 1);
    }
    else {
        // 新しい拡張子を追加
        strncat(path, new_extension, strlen(new_extension));
    }
}