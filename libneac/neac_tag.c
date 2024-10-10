#include "./include/neac_tag.h"
#include <string.h>

void neac_tag_init(neac_tag* tag) {
    const char* unknown = "Unknown";
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
}

void neac_tag_read(FILE* file, neac_tag** tag) {
    uint8_t num_tag_info, i, id, size;

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
            size = read_uint8(file);            /* タグのデータのバイト数を読み込む */

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
            }
        }
    }
    else {
        (*tag) = NULL;
    }
}

void neac_tag_write(FILE* file, neac_tag* tag) {
    uint8_t size;
    /* タグの存在判定用フラグを書き込む */
    write_bool(file, tag != NULL);

    if (tag != NULL) {
        /* タグ情報の総数を書き込む */
        write_uint8(file, NEAC_TAG_INFO_COUNT);

        /*  タグ情報のデータ構造 
         *  ------------------------------------
         *  | ID (1Byte) | Size (1Byte) | Data |
         *  ------------------------------------
         * 
         *  IDはタグの種類を示す。例えば、0x01なら曲のタイトルを、0x02ならアルバム名を示す。
         *  Size は、Data のバイト数を示す。文字列データの場合はこの内容が可変であるが、
         *  トラック番号や発行年など、固定長の整数データが格納される場合、Sizeを読み込まなくても
         *  Data のサイズを変数の型から推定することも可能である。しかし、データ構造上、固定長の
         *  データであっても、必ず Size は書き込まなければならない。
         */

        /* タイトル */
        size = (uint8_t)(strlen(tag->title) + 1);
        write_uint8(file, NEAC_TAG_ID_TITLE);
        write_uint8(file, size);
        write_string(file, tag->title, size);

        /* アルバム名 */
        size = (uint8_t)(strlen(tag->album) + 1);
        write_uint8(file, NEAC_TAG_ID_ALBUM);
        write_uint8(file, size);
        write_string(file, tag->album, size);

        /* アーティスト名 */
        size = (uint8_t)(strlen(tag->artist) + 1);
        write_uint8(file, NEAC_TAG_ID_ARTIST);
        write_uint8(file, size);
        write_string(file, tag->artist, size);

        /* アルバムアーティスト名 */
        size = (uint8_t)(strlen(tag->album_artist) + 1);
        write_uint8(file, NEAC_TAG_ID_ALBUM_ARTIST);
        write_uint8(file, size);
        write_string(file, tag->album_artist, size);

        /* サブタイトル */
        size = (uint8_t)(strlen(tag->subtitle) + 1);
        write_uint8(file, NEAC_TAG_ID_SUBTITLE);
        write_uint8(file, size);
        write_string(file, tag->subtitle, size);

        /* 発行者 */
        size = (uint8_t)(strlen(tag->publisher) + 1);
        write_uint8(file, NEAC_TAG_ID_PUBLISHER);
        write_uint8(file, size);
        write_string(file, tag->publisher, size);

        /* 作曲者 */
        size = (uint8_t)(strlen(tag->composer) + 1);
        write_uint8(file, NEAC_TAG_ID_COMPOSER);
        write_uint8(file, size);
        write_string(file, tag->composer, size);

        /* 作詞者 */
        size = (uint8_t)(strlen(tag->songwriter) + 1);
        write_uint8(file, NEAC_TAG_ID_SONGWRITER);
        write_uint8(file, size);
        write_string(file, tag->songwriter, size);

        /* 指揮者 */
        size = (uint8_t)(strlen(tag->conductor) + 1);
        write_uint8(file, NEAC_TAG_ID_CONDUCTOR);
        write_uint8(file, size);
        write_string(file, tag->conductor, size);

        /* 著作権表示 */
        size = (uint8_t)(strlen(tag->copyright) + 1);
        write_uint8(file, NEAC_TAG_ID_COPYRIGHT);
        write_uint8(file, size);
        write_string(file, tag->copyright, size);

        /* ジャンル */
        size = (uint8_t)(strlen(tag->genre) + 1);
        write_uint8(file, NEAC_TAG_ID_GENRE);
        write_uint8(file, size);
        write_string(file, tag->genre, size);

        /* 発行年 */
        size = 2;
        write_uint8(file, NEAC_TAG_ID_YEAR);
        write_uint8(file, size);
        write_uint16(file, tag->year);

        /* トラック番号 */
        size = 2;
        write_uint8(file, NEAC_TAG_ID_TRACK_NUM);
        write_uint8(file, size);
        write_uint16(file, tag->track_number);

        /* トラック数 */
        size = 2;
        write_uint8(file, NEAC_TAG_ID_TRACK_CNT);
        write_uint8(file, size);
        write_uint16(file, tag->track_count);

        /* ディスク番号 */
        size = 2;
        write_uint8(file, NEAC_TAG_ID_DISC);
        write_uint8(file, size);
        write_uint16(file, tag->disc);

        /* 評価 */
        size = 2;
        write_uint8(file, NEAC_TAG_ID_RATE);
        write_uint8(file, size);
        write_uint16(file, tag->rate);

        /* コメント */
        size = (uint8_t)(strlen(tag->comment) + 1);
        write_uint8(file, NEAC_TAG_ID_COMMENT);
        write_uint8(file, size);
        write_string(file, tag->comment, size);
    }
}