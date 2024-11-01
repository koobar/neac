#ifndef NEAC_TAG_HEADER_INCLUDED
#define NEAC_TAG_HEADER_INCLUDED

#define NEAC_TAG_ID_TITLE               0x01
#define NEAC_TAG_ID_ALBUM               0x02
#define NEAC_TAG_ID_ARTIST              0x03
#define NEAC_TAG_ID_ALBUM_ARTIST        0x04
#define NEAC_TAG_ID_SUBTITLE            0x05
#define NEAC_TAG_ID_PUBLISHER           0x06
#define NEAC_TAG_ID_COMPOSER            0x07
#define NEAC_TAG_ID_SONGWRITER          0x08
#define NEAC_TAG_ID_CONDUCTOR           0x09
#define NEAC_TAG_ID_COPYRIGHT           0x0a
#define NEAC_TAG_ID_GENRE               0x0b
#define NEAC_TAG_ID_YEAR                0x0d
#define NEAC_TAG_ID_TRACK_NUM           0x0e
#define NEAC_TAG_ID_TRACK_CNT           0x0f
#define NEAC_TAG_ID_DISC                0x10
#define NEAC_TAG_ID_RATE                0x11
#define NEAC_TAG_ID_COMMENT             0x12
#define NEAC_TAG_ID_PICTURE             0x13

#define NEAC_TAG_PICTURE_FORMAT_NONE    0x00
#define NEAC_TAG_PICTURE_FORMAT_PNG     0x01
#define NEAC_TAG_PICTURE_FORMAT_JPG     0x01
#define NEAC_TAG_PICTURE_FORMAT_BMP     0x02

#define NEAC_TAG_INFO_COUNT             18

#include <stdint.h>
#include <string.h>
#include <stdio.h>

typedef struct {
    const char* title;
    const char* album;
    const char* artist;
    const char* album_artist;
    const char* subtitle;
    const char* publisher;
    const char* composer;
    const char* songwriter;
    const char* conductor;
    const char* copyright;
    const char* genre;
    const char* comment; 
    uint16_t year;
    uint16_t track_number;
    uint16_t track_count;
    uint16_t disc;
    uint16_t rate;
    uint32_t picture_size;
    uint8_t picture_format;
    uint8_t* picture;
    
} neac_tag;

/*!
 * @brief                   タグ構造体を初期化します。
 * @param *tag              タグ構造体のハンドル
 */
void neac_tag_init(neac_tag* tag);

/*!
 * @brief                   タグ構造体を解放します。
 * @param *tag              タグ構造体のハンドル
 */
void neac_tag_free(neac_tag* tag);

/*!
 * @brief                   タグ構造体に画像ファイルを設定します。
 * @param *tag              タグ構造体のハンドル
 * @param *picture_file     画像ファイルのハンドル
 */
void neac_tag_set_picture(neac_tag* tag, FILE* picture_file);

/*!
 * @brief                   タグ構造体に画像ファイルを設定します。
 * @param *tag              タグ構造体のハンドル
 * @param *picture_file     画像ファイルのパス
 */
void neac_tag_set_picture_from_path(neac_tag* tag, const char* path);

/*!
 * @brief                   タグを読み込み、タグ構造体に格納します。
 * @param *file             ファイルのハンドル
 * @param **tag             タグ構造体のポインタのポインタ
 */
void neac_tag_read(FILE* file, neac_tag** tag);

/*!
 * @brief                   タグを書き込みます。
 * @param *file             ファイルのハンドル
 * @param *tag              タグ構造体のハンドル
 */
void neac_tag_write(FILE* file, neac_tag* tag);

#endif
