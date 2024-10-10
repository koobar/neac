#ifndef NEAC_TAG_H
#define NEAC_TAG_H

#define NEAC_TAG_ID_TITLE           0x01
#define NEAC_TAG_ID_ALBUM           0x02
#define NEAC_TAG_ID_ARTIST          0x03
#define NEAC_TAG_ID_ALBUM_ARTIST    0x04
#define NEAC_TAG_ID_SUBTITLE        0x05
#define NEAC_TAG_ID_PUBLISHER       0x06
#define NEAC_TAG_ID_COMPOSER        0x07
#define NEAC_TAG_ID_SONGWRITER      0x08
#define NEAC_TAG_ID_CONDUCTOR       0x09
#define NEAC_TAG_ID_COPYRIGHT       0x0a
#define NEAC_TAG_ID_GENRE           0x0b
#define NEAC_TAG_ID_YEAR            0x0d
#define NEAC_TAG_ID_TRACK_NUM       0x0e
#define NEAC_TAG_ID_TRACK_CNT       0x0f
#define NEAC_TAG_ID_DISC            0x10
#define NEAC_TAG_ID_RATE            0x11
#define NEAC_TAG_ID_COMMENT         0x12

#define NEAC_TAG_INFO_COUNT         17

#include <stdio.h>
#include <stdint.h>
#include "file_access.h"

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
    uint16_t year;
    uint16_t track_number;
    uint16_t track_count;
    uint16_t disc;
    uint16_t rate;
    const char* comment;
    
} neac_tag;

void neac_tag_init(neac_tag* tag);
void neac_tag_read(FILE* file, neac_tag** tag);
void neac_tag_write(FILE* file, neac_tag* tag);

#endif
