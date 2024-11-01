#include "./include/file_access.h"
#include "./include/neac_error.h"
#include <stdlib.h>
#include <string.h>

/*!
 * @brief       指定されたファイルポインタが示すファイルから、真偽値を読み込みます。
 * @param file  ファイルポインタ
 * @return      真偽値
 */
bool read_bool(FILE* file) {
    uint8_t value;
    size_t actual_read = fread(&value, sizeof(uint8_t), 1, file);

    if (actual_read < 1) {
        report_error(NEAC_ERROR_FILE_ACCESS_FAILED_TO_READ_BOOL);
    }

    return value == 1;
}

/*!
 * @brief       指定されたファイルポインタが示すファイルから16ビット符号付き整数を読み込みます。
 * @param file  ファイルポインタ
 * @return      読み込まれた値
 */
int16_t read_int16(FILE* file) {
    int16_t value;
    size_t actual_read = fread(&value, sizeof(int16_t), 1, file);

    if (actual_read < 1) {
        report_error(NEAC_ERROR_FILE_ACCESS_FAILED_TO_READ_INT16);
    }

    return value;
}

/*!
 * @brief       指定されたファイルポインタが示すファイルから32ビット符号付き整数を読み込みます。
 * @param file  ファイルポインタ
 * @return      読み込まれた値
 */
int32_t read_int32(FILE* file) {
    int32_t value;
    size_t actual_read = fread(&value, sizeof(int32_t), 1, file);

    if (actual_read < 1) {
        report_error(NEAC_ERROR_FILE_ACCESS_FAILED_TO_READ_INT32);
    }

    return value;
}

/*!
 * @brief       指定されたファイルポインタが示すファイルからASCII文字を読み込みます。
 * @param file  ファイルポインタ
 * @return      読み込まれた値
 */
char read_char(FILE* file) {
    char value;
    size_t actual_read = fread(&value, sizeof(char), 1, file);

    if (actual_read < 1) {
        report_error(NEAC_ERROR_FILE_ACCESS_FAILED_TO_READ_CHAR);
    }

    return value;
}

/*!
 * @brief       指定されたファイルポインタが示すファイルからASCII文字を読み込みます。
 * @param file  ファイルポインタ
 * @return      読み込まれた値
 */
const char* read_string(FILE* file, uint32_t bytes) {
    uint32_t i;
    char* result = (char*)malloc(bytes);

    if (result == NULL) {
        return NULL;
    }

    for (i = 0; i < bytes; ++i) {
        result[i] = read_char(file);
    }

    return result;
}

/*!
 * @brief       指定されたファイルポインタが示すファイルから8ビット整数を読み込みます。
 * @param file  ファイルポインタ
 * @return      読み込まれた値
 */
uint8_t read_uint8(FILE* file) {
    uint8_t value;
    size_t actual_read = fread(&value, sizeof(uint8_t), 1, file);

    if (actual_read < 1) {
        report_error(NEAC_ERROR_FILE_ACCESS_FAILED_TO_READ_UINT8);
    }

    return value;
}

/*!
 * @brief       指定されたファイルポインタが示すファイルから16ビット整数を読み込みます。
 * @param file  ファイルポインタ
 * @return      読み込まれた値
 */
uint16_t read_uint16(FILE* file) {
    uint16_t value;
    size_t actual_read = fread(&value, sizeof(uint16_t), 1, file);

    if (actual_read < 1) {
        report_error(NEAC_ERROR_FILE_ACCESS_FAILED_TO_READ_UINT16);
    }

    return value;
}

/*!
 * @brief       指定されたファイルポインタが示すファイルから32ビット整数を読み込みます。
 * @param file  ファイルポインタ
 * @return      読み込まれた値
 */
uint32_t read_uint32(FILE* file) {
    uint32_t value;
    size_t actual_read = fread(&value, sizeof(uint32_t), 1, file);

    if (actual_read < 1) {
        report_error(NEAC_ERROR_FILE_ACCESS_FAILED_TO_READ_UINT32);
    }

    return value;
}

/*!
 * @brief       指定されたファイルポインタが示すファイルに真偽値を書き込む。
 * @param file  ファイルポインタ
 * @param value 書き込む値
 */
void write_bool(FILE* file, const bool value) {
    size_t actual_write = 0;
    const uint8_t _true = 1;
    const uint8_t _false = 0;

    if (value) {
        actual_write = fwrite(&_true, sizeof(uint8_t), 1, file);
    }
    else {
        actual_write = fwrite(&_false, sizeof(uint8_t), 1, file);
    }

    if (actual_write < 1) {
        report_error(NEAC_ERROR_FILE_ACCESS_FAILED_TO_WRITE_BOOL);
    }
}

/*!
 * @brief       指定されたファイルポインタが示すファイルにASCII文字を書き込む。
 * @param file  ファイルポインタ
 * @param value 書き込む値
 */
void write_char(FILE* file, const char value) {
    size_t actual_write = fwrite(&value, sizeof(char), 1, file);

    if (actual_write < 1) {
        report_error(NEAC_ERROR_FILE_ACCESS_FAILED_TO_WRITE_CHAR);
    }
}

/*!
 * @brief       指定されたファイルポインタが示すファイルに文字列を書き込む。
 * @param file  ファイルポインタ
 * @param value 書き込む値
 */
void write_string(FILE* file, const char* value, uint32_t bytes) {
    uint8_t i;

    for (i = 0; i < bytes - 1; ++i) {
        write_char(file, value[i]);
    }
    write_char(file, '\0');         /* 末端文字 */
}

/*!
 * @brief       指定されたファイルポインタが示すファイルに8ビット整数を書き込む。
 * @param file  ファイルポインタ
 * @param value 書き込む値
 */
void write_uint8(FILE* file, const uint8_t value) {
    size_t actual_write = fwrite(&value, sizeof(uint8_t), 1, file);

    if (actual_write < 1) {
        report_error(NEAC_ERROR_FILE_ACCESS_FAILED_TO_WRITE_UINT8);
    }
}

/*!
 * @brief       指定されたファイルポインタが示すファイルに16ビット整数を書き込む。
 * @param file  ファイルポインタ
 * @param value 書き込む値
 */
void write_uint16(FILE* file, const uint16_t value) {
    size_t actual_write = fwrite(&value, sizeof(uint16_t), 1, file);

    if (actual_write < 1) {
        report_error(NEAC_ERROR_FILE_ACCESS_FAILED_TO_WRITE_UINT16);
    }
}

/*!
 * @brief       指定されたファイルポインタが示すファイルに32ビット整数を書き込む。
 * @param file  ファイルポインタ
 * @param value 書き込む値
 */
void write_uint32(FILE* file, const uint32_t value) {
    size_t actual_write = fwrite(&value, sizeof(uint32_t), 1, file);

    if (actual_write < 1) {
        report_error(NEAC_ERROR_FILE_ACCESS_FAILED_TO_WRITE_UINT32);
    }
}

/*!
 * @brief       指定されたファイルポインタが示すファイルに16符号付きビット整数を書き込む。
 * @param file  ファイルポインタ
 * @param value 書き込む値
 */
void write_int16(FILE* file, const int16_t value) {
    size_t actual_write = fwrite(&value, sizeof(int16_t), 1, file);

    if (actual_write < 1) {
        report_error(NEAC_ERROR_FILE_ACCESS_FAILED_TO_WRITE_INT16);
    }
}

/*!
 * @brief       指定されたファイルポインタが示すファイルに32ビット符号付整数を書き込む。
 * @param file  ファイルポインタ
 * @param value 書き込む値
 */
void write_int32(FILE* file, const int32_t value) {
    size_t actual_write = fwrite(&value, sizeof(int32_t), 1, file);

    if (actual_write < 1) {
        report_error(NEAC_ERROR_FILE_ACCESS_FAILED_TO_WRITE_INT32);
    }
}