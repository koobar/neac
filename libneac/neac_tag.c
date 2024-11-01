#include "./include/file_access.h"
#include "./include/neac_tag.h"
#include <stdlib.h>

/*!
 * @brief           タグ構造体のメンバ変数pictureに格納された画像のバイナリデータを、タグとしてファイルに出力します。
 * @param *file     ファイルハンドル
 * @param *tag      タグ構造体のハンドル
 */
static void neac_tag_write_picture(FILE* file, neac_tag* tag) {
    uint32_t offset;

    if (tag == NULL || tag->picture_size == 0 || tag->picture == NULL) {
        return;
    }

    /* 画像データを書き込む */
    write_uint8(file, NEAC_TAG_ID_PICTURE);
    write_uint32(file, tag->picture_size);
    write_uint8(file, tag->picture_format);
    for (offset = 0; offset < tag->picture_size; ++offset) {
        write_uint8(file, tag->picture[offset]);
    }
}

/*!
 * @brief           ファイルから画像データを読み込みます。
 * @param *file     ファイルハンドル
 * @param **tag     タグ構造体のポインタのポインタ
 * @param size      画像データのサイズ
 */
static void neac_tag_read_picture(FILE* file, neac_tag** tag, uint32_t size) {
    uint32_t offset;

    /* 画像データを格納する領域を確保 */
    (*tag)->picture_size = (uint32_t)size;
    (*tag)->picture = (uint8_t*)malloc(sizeof(uint8_t) * size);
    if ((*tag)->picture == NULL) {
        return;
    }

    /* 画像データを読み込む */
    (*tag)->picture_format = read_uint8(file);
    for (offset = 0; offset < size; ++offset) {
        (*tag)->picture[offset] = read_uint8(file);
    }
}

/*!
 * @brief       1個目の引数に指定された文字列が、2個目の引数に指定された文字列で終了しているかどうかを判定します。但し、大文字と小文字を区別しません。
 * @param *str  文字列
 * @param *end  引数1が、この文字列で終了しているかどうかを判定します。
 * @return      引数1が引数2の文字列で終了していればtrueを、そうでなければfalseを返します。
 */
static int str_ends_with(const char* str, const char* end) {
    size_t len = strlen(str);
    size_t endlen = strlen(end);

    if (len < endlen) {
        return 0;
    }

    return _stricmp(str + len - endlen, end) == 0;
}

/*!
 * @brief                   タグ構造体を初期化します。
 * @param *tag              タグ構造体のハンドル
 */
void neac_tag_init(neac_tag* tag) {
    const char* unknown = "\0";
    const uint16_t default_integer = 0;
    tag->title = unknown;
    tag->album = unknown;
    tag->artist = unknown;
    tag->subtitle = unknown;
    tag->publisher = unknown;
    tag->composer = unknown;
    tag->songwriter = unknown;
    tag->conductor = unknown;
    tag->copyright = unknown;
    tag->comment = unknown;
    tag->album_artist = unknown;
    tag->genre = unknown;
    tag->year = default_integer;
    tag->track_number = default_integer;
    tag->track_count = default_integer;
    tag->disc = default_integer;
    tag->picture_size = 0;
    tag->picture_format = NEAC_TAG_PICTURE_FORMAT_NONE;
    tag->picture = NULL;
}

/*!
 * @brief                   タグ構造体を解放します。
 * @param *tag              タグ構造体のハンドル
 */
void neac_tag_free(neac_tag* tag) {
    if (tag == NULL) {
        return;
    }

    if (tag->picture != NULL) {
        free(tag->picture);
    }
}

/*!
 * @brief                   タグ構造体に画像ファイルを設定します。
 * @param *tag              タグ構造体のハンドル
 * @param *picture_file     画像ファイルのハンドル
 */
void neac_tag_set_picture(neac_tag* tag, FILE* picture_file) {
    fpos_t size, offset;
    if (picture_file == NULL) {
        return;
    }

    /* 画像ファイルのサイズを取得 */
    fseek(picture_file, 0, SEEK_END);
    fgetpos(picture_file, &size);

    /* 画像ファイルの先頭に戻る */
    rewind(picture_file);

    /* 画像ファイルのデータを格納する領域を確保 */
    tag->picture_size = (uint32_t)size;
    tag->picture = (uint8_t*)malloc(sizeof(uint8_t) * (uint32_t)size);
    if (tag->picture == NULL) {
        return;
    }

    /* 画像ファイルのデータを読み込む */
    for (offset = 0; offset < size; ++offset) {
        tag->picture[offset] = read_uint8(picture_file);
    }

    fclose(picture_file);
}

/*!
 * @brief                   タグ構造体に画像ファイルを設定します。
 * @param *tag              タグ構造体のハンドル
 * @param *picture_file     画像ファイルのパス
 */
void neac_tag_set_picture_from_path(neac_tag* tag, const char* path) {
    FILE* file = NULL;
    errno_t err;

    err = fopen_s(&file, path, "rb");
    if (err != 0) {
        return;
    }

    if (str_ends_with(path, ".png")) {
        tag->picture_format = NEAC_TAG_PICTURE_FORMAT_PNG;
    }
    else if (str_ends_with(path, ".jpg") || str_ends_with(path, ".jpeg")) {
        tag->picture_format = NEAC_TAG_PICTURE_FORMAT_JPG;
    }
    else if (str_ends_with(path, ".bmp")) {
        tag->picture_format = NEAC_TAG_PICTURE_FORMAT_BMP;
    }

    neac_tag_set_picture(tag, file);
}

/*!
 * @brief                   タグを読み込み、タグ構造体に格納します。
 * @param *file             ファイルのハンドル
 * @param **tag             タグ構造体のポインタのポインタ
 */
void neac_tag_read(FILE* file, neac_tag** tag) {
    uint8_t num_tag_info, i, id;
    uint32_t size;

    if (read_bool(file)) {
        *tag = (neac_tag*)malloc(sizeof(neac_tag));

        if (*tag == NULL) {
            return;
        }

        /* タグ情報を初期化 */
        neac_tag_init(*tag);

        /* タグ数を取得 */
        num_tag_info = read_uint8(file);

        /* すべてのタグを読み込む */
        for (i = 0; i < num_tag_info; ++i) {
            id = read_uint8(file);              /* タグIDを読み込む */
            size = read_uint32(file);           /* タグのデータのバイト数を読み込む */

            switch (id) {
            case NEAC_TAG_ID_TITLE:
                (*tag)->title = read_string(file, size);
                break;
            case NEAC_TAG_ID_ALBUM:
                (*tag)->album = read_string(file, size);
                break;
            case NEAC_TAG_ID_ARTIST:
                (*tag)->artist = read_string(file, size);
                break;
            case NEAC_TAG_ID_ALBUM_ARTIST:
                (*tag)->album_artist = read_string(file, size);
                break;
            case NEAC_TAG_ID_SUBTITLE:
                (*tag)->subtitle = read_string(file, size);
                break;
            case NEAC_TAG_ID_PUBLISHER:
                (*tag)->publisher = read_string(file, size);
                break;
            case NEAC_TAG_ID_COMPOSER:
                (*tag)->composer = read_string(file, size);
                break;
            case NEAC_TAG_ID_SONGWRITER:
                (*tag)->songwriter = read_string(file, size);
                break;
            case NEAC_TAG_ID_CONDUCTOR:
                (*tag)->conductor = read_string(file, size);
                break;
            case NEAC_TAG_ID_COPYRIGHT:
                (*tag)->copyright = read_string(file, size);
                break;
            case NEAC_TAG_ID_GENRE:
                (*tag)->genre = read_string(file, size);
                break;
            case NEAC_TAG_ID_YEAR:
                (*tag)->year = read_uint16(file);
                break;
            case NEAC_TAG_ID_TRACK_NUM:
                (*tag)->track_number = read_uint16(file);
                break;
            case NEAC_TAG_ID_TRACK_CNT:
                (*tag)->track_count = read_uint16(file);
                break;
            case NEAC_TAG_ID_DISC:
                (*tag)->disc = read_uint16(file);
                break;
            case NEAC_TAG_ID_COMMENT:
                (*tag)->comment = read_string(file, size);
                break;
            case NEAC_TAG_ID_RATE:
                (*tag)->rate = read_uint16(file);
                break;
            case NEAC_TAG_ID_PICTURE:
                neac_tag_read_picture(file, tag, size);
                break;
            }
        }
    }
    else {
        (*tag) = NULL;
    }
}

/*!
 * @brief                   タグを書き込みます。
 * @param *file             ファイルのハンドル
 * @param *tag             タグ構造体のハンドル
 */
void neac_tag_write(FILE* file, neac_tag* tag) {
    uint32_t size;

    /* タグの存在判定用フラグを書き込む */
    write_bool(file, tag != NULL);

    if (tag != NULL) {
        /* タグ情報の総数を書き込む */
        write_uint8(file, NEAC_TAG_INFO_COUNT);

        /*  タグ情報のデータ構造 
         *  -------------------------------------
         *  | ID (1Byte) | Size (4Bytes) | Data |
         *  -------------------------------------
         * 
         *  IDはタグの種類を示す。例えば、0x01なら曲のタイトルを、0x02ならアルバム名を示す。
         *  Size は、Data のバイト数を示す。文字列データの場合はこの内容が可変であるが、
         *  トラック番号や発行年など、固定長の整数データが格納される場合、Sizeを読み込まなくても
         *  Data のサイズを変数の型から推定することも可能である。しかし、データ構造上、固定長の
         *  データであっても、必ず Size は書き込まなければならない。
         */

        /* タイトル */
        size = (uint32_t)(strlen(tag->title) + 1);
        write_uint8(file, NEAC_TAG_ID_TITLE);
        write_uint32(file, size);
        write_string(file, tag->title, size);

        /* アルバム名 */
        size = (uint32_t)(strlen(tag->album) + 1);
        write_uint8(file, NEAC_TAG_ID_ALBUM);
        write_uint32(file, size);
        write_string(file, tag->album, size);

        /* アーティスト名 */
        size = (uint32_t)(strlen(tag->artist) + 1);
        write_uint8(file, NEAC_TAG_ID_ARTIST);
        write_uint32(file, size);
        write_string(file, tag->artist, size);

        /* アルバムアーティスト名 */
        size = (uint32_t)(strlen(tag->album_artist) + 1);
        write_uint8(file, NEAC_TAG_ID_ALBUM_ARTIST);
        write_uint32(file, size);
        write_string(file, tag->album_artist, size);

        /* サブタイトル */
        size = (uint32_t)(strlen(tag->subtitle) + 1);
        write_uint8(file, NEAC_TAG_ID_SUBTITLE);
        write_uint32(file, size);
        write_string(file, tag->subtitle, size);

        /* 発行者 */
        size = (uint32_t)(strlen(tag->publisher) + 1);
        write_uint8(file, NEAC_TAG_ID_PUBLISHER);
        write_uint32(file, size);
        write_string(file, tag->publisher, size);

        /* 作曲者 */
        size = (uint32_t)(strlen(tag->composer) + 1);
        write_uint8(file, NEAC_TAG_ID_COMPOSER);
        write_uint32(file, size);
        write_string(file, tag->composer, size);

        /* 作詞者 */
        size = (uint32_t)(strlen(tag->songwriter) + 1);
        write_uint8(file, NEAC_TAG_ID_SONGWRITER);
        write_uint32(file, size);
        write_string(file, tag->songwriter, size);

        /* 指揮者 */
        size = (uint32_t)(strlen(tag->conductor) + 1);
        write_uint8(file, NEAC_TAG_ID_CONDUCTOR);
        write_uint32(file, size);
        write_string(file, tag->conductor, size);

        /* 著作権表示 */
        size = (uint32_t)(strlen(tag->copyright) + 1);
        write_uint8(file, NEAC_TAG_ID_COPYRIGHT);
        write_uint32(file, size);
        write_string(file, tag->copyright, size);

        /* ジャンル */
        size = (uint32_t)(strlen(tag->genre) + 1);
        write_uint8(file, NEAC_TAG_ID_GENRE);
        write_uint32(file, size);
        write_string(file, tag->genre, size);

        /* コメント */
        size = (uint32_t)(strlen(tag->comment) + 1);
        write_uint8(file, NEAC_TAG_ID_COMMENT);
        write_uint32(file, size);
        write_string(file, tag->comment, size);

        /* 発行年 */
        size = 2;
        write_uint8(file, NEAC_TAG_ID_YEAR);
        write_uint32(file, size);
        write_uint16(file, tag->year);

        /* トラック番号 */
        size = 2;
        write_uint8(file, NEAC_TAG_ID_TRACK_NUM);
        write_uint32(file, size);
        write_uint16(file, tag->track_number);

        /* トラック数 */
        size = 2;
        write_uint8(file, NEAC_TAG_ID_TRACK_CNT);
        write_uint32(file, size);
        write_uint16(file, tag->track_count);

        /* ディスク番号 */
        size = 2;
        write_uint8(file, NEAC_TAG_ID_DISC);
        write_uint32(file, size);
        write_uint16(file, tag->disc);

        /* 評価 */
        size = 2;
        write_uint8(file, NEAC_TAG_ID_RATE);
        write_uint32(file, size);
        write_uint16(file, tag->rate);

        /* 画像の書き込み */
        neac_tag_write_picture(file, tag);
    }
}