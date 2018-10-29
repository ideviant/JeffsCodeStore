/*****************************************************************************
 * Copyright 2018 Jeff <ggjogh@gmail.com>
 *****************************************************************************
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include "box_reader.h"


#ifdef __MINGW32__
#undef fseeko
#define fseeko(x, y, z) fseeko64(x, y, z)
#undef ftello
#define ftello(x)       ftello64(x)
#elif defined(_WIN32)
#undef fseeko
#define fseeko(x, y, z) _fseeki64(x, y, z)
#undef ftello
#define ftello(x)       _ftelli64(x)
#endif


const char* get4cc(uint32_t v) {
    static char buf[8] = {0};
    snprintf(buf, 5, "%c%c%c%c", 
        (v >> 24) & 255,
        (v >> 16) & 255,
        (v >>  8) & 255,
        (v >>  0) & 255);
    return buf;
}


/****************************************************************************
 * raw data access
 ****************************************************************************/

int raw_malloc(raw_data_t *raw, size_t size) {
    memset(raw, 0, sizeof(raw_data_t));
    if (0 == (raw->buf = malloc(size))) {
        return ERR_MALLOC_BUF;
    }
    raw_rewind(raw);
    return ERR_NONE;
}

void raw_release(raw_data_t *raw) {
    if (raw==0)     return;
    if (raw->buf)   free(raw->buf);
    memset(raw, 0, sizeof(raw_data_t));
}

void raw_rewind(raw_data_t *raw)
{
	raw->pos = raw->buf;
}

size_t raw_tell(raw_data_t *raw) {
	return raw->pos - raw->buf;
}

size_t raw_left(raw_data_t *raw) {
	return raw->data_size - (raw->pos - raw->buf);
}

int raw_read(uint8_t **p, size_t size, size_t count, raw_data_t *raw)
{
	if (raw_left(raw) < size * count) {
		return 0;
	} else {
		*p = raw->pos;
		raw->pos += size * count;
		return count;
	}
} 

int raw_seek(raw_data_t *raw, long int offset, int origin)
{
	uint8_t *end = raw->buf + raw->data_size;

	if (origin == SEEK_SET) {
		raw->pos = raw->buf + offset;
	} else if (origin == SEEK_CUR){
		raw->pos = raw->pos + offset;
	} else if (origin == SEEK_END) {
		raw->pos = end + offset;
	} else {
		return 1;
	}

	if (raw->pos < raw->buf || raw->pos > end) {
		return 1;
	}

	return 0;
}


void box_reset(box_t *pbox) 
{
    memset(pbox, 0, sizeof(box_t));
}

/****************************************************************************
 * box access from file
 ****************************************************************************/

int box_read_head_f(FILE *infile, box_t *pbox) 
{
    unsigned char atom_bytes[8];
    box_reset(pbox);
    if (feof(infile)) { 
        return 0; 
    }

    pbox->atom_offset = ftello(infile);
    if (fread(atom_bytes, 8, 1, infile) != 1) {
        _log_error("not enough byte for box header\n");
        return 1;
    }
    pbox->atom_size = BE_32(&atom_bytes[0]);
    pbox->atom_type = BE_32(&atom_bytes[4]);

    /* 64-bit special case */
    if (pbox->atom_size == 1) {
        if (fread(atom_bytes, 8, 1, infile) != 1) {
            _log_error("not enough byte for large-size box header\n");
            return 1;
        }
        pbox->atom_size = BE_64(&atom_bytes[0]);
    } 

    _log_info("f'%s' %10"PRIu64":%-10"PRIu64" %10"PRIX64":0x%-10"PRIX64"\n",
        get4cc(pbox->atom_type),
        pbox->atom_offset, pbox->atom_size,
        pbox->atom_offset, pbox->atom_size);

    /* The atom header is 8 (or 16 bytes), if the atom size (which
     * includes these 8 or 16 bytes) is less than that, we won't be
     * able to continue scanning sensibly after this atom, so break. */
    pbox->data_offset = ftello(infile);
    if (pbox->atom_size < (pbox->data_offset - pbox->atom_offset)) {
        _log_error("unproper atom size\n");
        return 1;
    }

    // box_seek_to_end(infile, pbox);
    if (fseeko(infile, pbox->atom_offset + pbox->atom_size, SEEK_SET)) {
        _log_error("can't seek to box end\n");
        return 1;
    }

    return 0;
}

int box_seek_to_begin_f(FILE *infile, box_t *pbox)
{
    if (fseeko(infile, pbox->atom_offset, SEEK_SET)) {
        _log_error("can't seek to box data\n");
        return 1;
    }
    return 0;
}

int box_seek_to_data_f(FILE *infile, box_t *pbox, int b_change_to_fullbox) 
{
    unsigned char atom_bytes[8];

    if (fseeko(infile, pbox->data_offset, SEEK_SET)) {
        _log_error("can't seek to box data\n");
        return 1;
    }

    if ((pbox->b_fullbox = b_change_to_fullbox)) {
        if (fread(atom_bytes, 4, 1, infile) != 1) {
            _log_error("not enough byte for version & flags\n");
            return 1;
        }
        pbox->version = atom_bytes[0];
        pbox->flags = BE_24(&atom_bytes[1]);
        pbox->data_offset = ftello(infile);
    }

    return 0;
}

int box_seek_to_end_f(FILE *infile, box_t *pbox) 
{
    if (fseeko(infile, pbox->atom_offset + pbox->atom_size, SEEK_SET)) {
        _log_error("can't seek to box end\n");
        return 1;
    }
    return 0;
}

int box_end_assert_f(FILE *infile, box_t *pbox)
{
    if (ftello(infile) >= pbox->atom_offset + pbox->atom_size) {
        return 1;
    }
    return 0;
}

/****************************************************************************
 * box access from raw data buffer
 ****************************************************************************/

int box_read_head(raw_data_t *raw, box_t *pbox) 
{
    unsigned char *atom_bytes;

    pbox->atom_offset = raw_tell(raw);
    if (raw_read(&atom_bytes, 8, 1, raw) != 1) {
        _log_error("not enough byte for box header\n");
        return 1;
    }
    pbox->atom_size = BE_32(&atom_bytes[0]);
    pbox->atom_type = BE_32(&atom_bytes[4]);

    /* 64-bit special case */
    if (pbox->atom_size == 1) {
        if (raw_read(&atom_bytes, 8, 1, raw) != 1) {
            _log_error("not enough byte for large-size box header\n");
            return 1;
        }
        pbox->atom_size = BE_64(&atom_bytes[0]);
    } 

    _log_info("r'%s' %10"PRIu64":%-10"PRIu64" %10"PRIX64":0x%-10"PRIX64"\n",
        get4cc(pbox->atom_type),
        pbox->atom_offset, pbox->atom_size,
        pbox->atom_offset, pbox->atom_size);

    /* The atom header is 8 (or 16 bytes), if the atom size (which
     * includes these 8 or 16 bytes) is less than that, we won't be
     * able to continue scanning sensibly after this atom, so break. */
    pbox->data_offset = raw_tell(raw);
    if (pbox->atom_size < (pbox->data_offset - pbox->atom_offset)) {
        _log_error("unproper atom size\n");
        return 1;
    }

    // box_seek_to_end(infile, pbox);
    if (raw_seek(raw, pbox->atom_offset + pbox->atom_size, SEEK_SET)) {
        _log_error("can't seek to box end\n");
        return 1;
    }

    return 0;
}

int box_seek_to_begin(raw_data_t *raw, box_t *pbox)
{
    if (raw_seek(raw, pbox->atom_offset, SEEK_SET)) {
        _log_error("can't seek to box data\n");
        return 1;
    }
    return 0;
}

int box_seek_to_data(raw_data_t *raw, box_t *pbox, int b_change_to_fullbox) 
{
    unsigned char *atom_bytes;

    if (raw_seek(raw, pbox->data_offset, SEEK_SET)) {
        _log_error("can't seek to box data\n");
        return 1;
    }

    if ((pbox->b_fullbox = b_change_to_fullbox)) {
        if (raw_read(&atom_bytes, 4, 1, raw) != 1) {
            _log_error("not enough byte for version & flags\n");
            return 1;
        }
        pbox->version = atom_bytes[0];
        pbox->flags = BE_24(&atom_bytes[1]);
        pbox->data_offset = raw_tell(raw);
    }

    return 0;
}

int box_seek_to_end(raw_data_t *raw, box_t *pbox) 
{
    if (raw_seek(raw, pbox->atom_offset + pbox->atom_size, SEEK_SET)) {
        _log_error("can't seek to box end\n");
        return 1;
    }
    return 0;
}

int box_end_assert(raw_data_t *raw, box_t *pbox)
{
    if (raw_tell(raw) >= pbox->atom_offset + pbox->atom_size) {
        return 1;
    }
    return 0;
}