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
 
/*
 * Caution: 
 *  This file currently implemented according to ISO MP4 Specification
 *  Mismatch to QuickTime File Format Specification may happen:
 *  <https://developer.apple.com/library/content/documentation/QuickTime/QTFF/QTFFPreface/qtffPreface.html>
 */

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <math.h>
#include "box_reader.h"
#include "moov_parse.h"


// not thread save
const char* get4cc(uint32_t v);

#define BE_FOURCC(ch0, ch1, ch2, ch3)           \
    ( (uint32_t)(unsigned char)(ch3)        |   \
     ((uint32_t)(unsigned char)(ch2) <<  8) |   \
     ((uint32_t)(unsigned char)(ch1) << 16) |   \
     ((uint32_t)(unsigned char)(ch0) << 24) )

#define QT_ATOM BE_FOURCC
/* top level atoms */
#define FREE_ATOM QT_ATOM('f', 'r', 'e', 'e')
#define JUNK_ATOM QT_ATOM('j', 'u', 'n', 'k')
#define MDAT_ATOM QT_ATOM('m', 'd', 'a', 't')
#define MOOV_ATOM QT_ATOM('m', 'o', 'o', 'v')
#define PNOT_ATOM QT_ATOM('p', 'n', 'o', 't')
#define SKIP_ATOM QT_ATOM('s', 'k', 'i', 'p')
#define WIDE_ATOM QT_ATOM('w', 'i', 'd', 'e')
#define PICT_ATOM QT_ATOM('P', 'I', 'C', 'T')
#define FTYP_ATOM QT_ATOM('f', 't', 'y', 'p')
#define UUID_ATOM QT_ATOM('u', 'u', 'i', 'd')
#define CMOV_ATOM QT_ATOM('c', 'm', 'o', 'v')

/* moov sub atoms */
#define MVHD_ATOM QT_ATOM('m', 'v', 'h', 'd')
#define TRAK_ATOM QT_ATOM('t', 'r', 'a', 'k')
#define MVEX_ATOM QT_ATOM('m', 'v', 'e', 'x')
#define IPMC_ATOM QT_ATOM('i', 'p', 'm', 'c')

/* trak sub atoms */
#define TKHD_ATOM QT_ATOM('t', 'k', 'h', 'd')
#define TREF_ATOM QT_ATOM('t', 'r', 'e', 'f')
#define EDTS_ATOM QT_ATOM('e', 'd', 't', 's')
#define MDIA_ATOM QT_ATOM('m', 'd', 'i', 'a')

/* mdia sub atoms */
#define MDHD_ATOM QT_ATOM('m', 'd', 'h', 'd')
#define HDLR_ATOM QT_ATOM('h', 'd', 'l', 'r')
#define MINF_ATOM QT_ATOM('m', 'i', 'n', 'f')

/* minf sub atoms */
#define VMHD_ATOM QT_ATOM('v', 'm', 'h', 'd')
#define SMHD_ATOM QT_ATOM('s', 'm', 'h', 'd')
#define HMHD_ATOM QT_ATOM('h', 'm', 'h', 'd')
#define NMHD_ATOM QT_ATOM('m', 'm', 'h', 'd')
#define DINF_ATOM QT_ATOM('d', 'i', 'n', 'f')
#define STBL_ATOM QT_ATOM('s', 't', 'b', 'l')

/* stbl sub atoms */
#define STSD_ATOM QT_ATOM('s', 't', 's', 'd')
#define STTS_ATOM QT_ATOM('s', 't', 't', 's')
#define CTTS_ATOM QT_ATOM('c', 't', 't', 's')
#define STSC_ATOM QT_ATOM('s', 't', 's', 'c')
#define STSZ_ATOM QT_ATOM('s', 't', 's', 'z')
#define STZ2_ATOM QT_ATOM('s', 't', 'z', '2')
#define STCO_ATOM QT_ATOM('s', 't', 'c', 'o')
#define CO64_ATOM QT_ATOM('c', 'o', '6', '4')
#define STSS_ATOM QT_ATOM('s', 't', 's', 's')
#define STSH_ATOM QT_ATOM('s', 't', 's', 'h')
#define PADB_ATOM QT_ATOM('p', 'a', 'd', 'b')
#define STDP_ATOM QT_ATOM('s', 't', 'd', 'p')
#define SDTP_ATOM QT_ATOM('s', 'd', 't', 'p')
#define SBGP_ATOM QT_ATOM('s', 'b', 'g', 'p')
#define SGPD_ATOM QT_ATOM('s', 'g', 'p', 'd')
#define SUBS_ATOM QT_ATOM('s', 'u', 'b', 's')


typedef struct _pack_entry {
    uint32_t count;
    uint32_t value;
} pack_entry_t;

typedef struct _mvhd {
    uint64_t creation_time;
    uint64_t modification_time;
    uint32_t timescale;
    uint64_t duration;
} mvhd_t;

typedef struct _tkhd {
    uint64_t creation_time;
    uint64_t modification_time;
    uint32_t track_ID;
    uint64_t duration;
} tkhd_t;

typedef struct _mdhd {
    uint64_t creation_time;
    uint64_t modification_time;
    uint32_t timescale;
    uint64_t duration;
} mdhd_t;

typedef struct _hdlr {
    uint32_t handler_type;                      // 'vide', 'soun', 'hint'
} hdlr_t;

typedef struct _stts {
    uint32_t entry_count;
    pack_entry_t *entries;
} stts_t;

typedef struct _ctts {
    uint32_t entry_count;
    pack_entry_t *entries;
} ctts_t;

typedef struct _stsz {                                  // ‘stsz’, ‘stz2’
    uint32_t b_stsz2;
    uint32_t filed_size;
    uint32_t sample_size;

    uint32_t sample_count;
    uint32_t *entry_size;
} stsz_t;

typedef struct _stbl {
    stts_t stts;
    ctts_t ctts;
    stsz_t stsz;
} stbl_t;

typedef struct _minf {
    // union {  
    //     vmhd_t vmhd;
    //     smhd_t smhd;
    //     hmhd_t hmhd;
    //     nmhd_t nmhd;
    // };

    stbl_t stbl;
} minf_t;

typedef struct _mdia {
    mdhd_t mdhd;
    hdlr_t hdlr;
    minf_t minf;
} mdia_t;

typedef struct _trak {
    tkhd_t tkhd;
    mdia_t mdia;

    uint32_t stbl_offset;
    uint32_t stbl_size;

    float   constant_fps;
    uint32_t framecount;
    float   *pts_array;
} trak_t;

typedef struct _moov_ctx {
    mvhd_t mvhd;
    trak_t trak[2];
    uint32_t trak_saved;
    trak_t *curr_trak;
    trak_t *vide_trak;
    trak_t *soun_trak;

    uint32_t vide_count;
    uint32_t soun_count;
} moov_ctx_t;


void release_parse_moov_malloc(moov_ctx_t *moov);
int parse_hdlr(raw_data_t *raw, box_t *box1, hdlr_t *hdlr);
int parse_moov(raw_data_t *raw, box_t *box1, moov_ctx_t *moov);
int parse_mvhd(raw_data_t *raw, box_t *box1, moov_ctx_t *moov);
int parse_trak(raw_data_t *raw, box_t *box1, moov_ctx_t *moov);
int parse_mdia(raw_data_t *raw, box_t *box1, moov_ctx_t *moov);
int parse_mdhd(raw_data_t *raw, box_t *box1, moov_ctx_t *moov);
int parse_minf(raw_data_t *raw, box_t *box1, moov_ctx_t *moov);
int parse_stbl(raw_data_t *raw, box_t *box1, moov_ctx_t *moov);
int parse_stts(raw_data_t *raw, box_t *box1, moov_ctx_t *moov);
int parse_ctts(raw_data_t *raw, box_t *box1, moov_ctx_t *moov);


static int _cmpfloat (const void * a, const void * b) {
   return ( *(float*)a - *(float*)b );
}

int main(int argc, char *argv[])
{
    int ret = ERR_NONE;

    if (argc != 2) {
        _log_error("Usage: qt-faststart <infile.mov>\n");
        return 0;
    }

    raw_data_t moov_raw;
    if (ERR_NONE != (ret = read_moov_data(argv[1], &moov_raw, 0))) {
        _log_error("fail to read moov (%d)\n", ret);
        return ret;
    }

    moov_report_t report;
    if (ERR_NONE != (ret = parse_moov_data(&moov_raw, &report))) {
        _log_error("fail to parse moov (%d)\n", ret);
    }

    raw_release(&moov_raw);

    return ret;
}

int read_moov_data(const char *input, raw_data_t *raw, int max_alloc_size)
{
    int ret = ERR_NONE;
    #define ERROR_EXIT(c) ret=(c); goto error_out;

    FILE *infile = fopen(input, "rb");
    if (!infile) {
        return ERR_OPEN_INPUT;
    }

    box_t _box1 = {0}, *box1 = &_box1;
    /* traverse through the atoms until 'moov' is found */
    while (!feof(infile)) {
        if (box_read_head_f(infile, box1) != 0) {
            return ERR_BOX_READ_HEAD;
        }

        if ((box1->atom_type != MOOV_ATOM) &&
            (box1->atom_type != FREE_ATOM) &&
            (box1->atom_type != JUNK_ATOM) &&
            (box1->atom_type != MDAT_ATOM) &&
            (box1->atom_type != PNOT_ATOM) &&
            (box1->atom_type != SKIP_ATOM) &&
            (box1->atom_type != WIDE_ATOM) &&
            (box1->atom_type != PICT_ATOM) &&
            (box1->atom_type != UUID_ATOM) &&
            (box1->atom_type != FTYP_ATOM)) {
            _log_warning("warning: '%s' is not top-level atom (is this a QuickTime file?)\n", get4cc(box1->atom_type));
            //ERROR_EXIT(ERR_BOX_UNSUPPORT);
        }

        if (box1->atom_type == MOOV_ATOM) {
            break;
        }
    }

    if (box1->atom_type != MOOV_ATOM) {
        _log_error("no moov atom found\n");
        ERROR_EXIT(ERR_NO_MOOV);
    }

    if (max_alloc_size > 0 && box1->atom_size < max_alloc_size) {
        _log_error("moov size exceed %"PRIX64"\n", box1->atom_size);
        ERROR_EXIT(ERR_MOOV_TOO_LARGE);
    }

    memset(raw, 0, sizeof(raw_data_t));
    if (ERR_NONE != raw_malloc(raw, box1->atom_size)) {
        _log_error("fail to malloc memory for moov\n");
        ERROR_EXIT(ERR_MALLOC_MOOV);
    }
    box_seek_to_begin_f(infile, box1);
    if (fread(raw->buf, box1->atom_size, 1, infile) != 1) {
        _log_error("not enough byte for version & flags\n");
        raw_release(raw);
        ERROR_EXIT(ERR_READ_MOOV);
    }
    raw->data_size = box1->atom_size;

error_out:
    fclose(infile);
    return ret;   
}


int parse_moov_data(raw_data_t *raw, moov_report_t *report)
{
    int ret = ERR_NONE;

    raw_rewind(raw);
    box_t _box = {0}, *pbox = &_box;
    if (ERR_NONE != (ret=box_read_head(raw, pbox))) {
        return ERR_BOX_READ_HEAD;
    }
    moov_ctx_t _moov = {0}, *moov = &_moov;
    if (ERR_NONE != (ret=parse_moov(raw, pbox, moov))) {
        release_parse_moov_malloc(moov);
        return ret;
    }

    memset(report, 0, sizeof(moov_report_t));
    report->moov_size = raw->data_size;
    report->movie_duration = moov->mvhd.duration / (moov->mvhd.timescale + 0.0001);
    if (moov->soun_trak) {
        report->hasaudio = 1;
        trak_t *trak = moov->soun_trak;

        report->audio_duration = trak->mdia.mdhd.duration / (trak->mdia.mdhd.timescale + 0.0001);
        report->audio_stbl_offset = trak->stbl_offset;
        report->audio_stbl_size = trak->stbl_size;
        //report->sample_rate = trak->timescale;
    }
    if (moov->vide_trak) {
        report->hasvideo = 1;
        trak_t *trak = moov->vide_trak;
        qsort(trak->pts_array, trak->framecount, sizeof(float), _cmpfloat);

        report->video_duration = trak->mdia.mdhd.duration / (trak->mdia.mdhd.timescale + 0.0001);
        report->video_stbl_offset = trak->stbl_offset;
        report->video_stbl_size = trak->stbl_size;
        //report->constant_fps = trak->constant_fps;
        report->framecount = trak->framecount;
        if (report->video_duration)
            report->average_fps = trak->framecount / report->video_duration;

        if (trak->framecount > 1) {
            report->min_frame_dur = trak->pts_array[1] - trak->pts_array[0];
            report->max_frame_dur = trak->pts_array[1] - trak->pts_array[0];
            float frame_aver_duration = (trak->pts_array[trak->framecount -1] - trak->pts_array[0]) / (trak->framecount -1);

            for (int k=1; k<trak->framecount; k++) {
                float delta = trak->pts_array[k] - trak->pts_array[k-1];
                report->min_frame_dur = delta < report->min_frame_dur ? delta : report->min_frame_dur;
                report->max_frame_dur = delta > report->max_frame_dur ? delta : report->max_frame_dur;
                report->frame_dur_100_count += (delta >= 100);
                report->frame_dur_200_count += (delta >= 200);
                report->frame_dur_variance += (delta - frame_aver_duration) * (delta - frame_aver_duration);
                _log_debug("frame %d, pts=%.0f (+%.0f)\n", k, trak->pts_array[k], delta);
            }
            report->frame_dur_variance = sqrt(report->frame_dur_variance/(trak->framecount-1));
        }
        for (int k=0; k<trak->framecount && trak->pts_array[k]<15*1000; k++) {
            report->second_frame_count[ (int) (trak->pts_array[k]/1000) ] += 1;
        }
    }

    print_moov_data(report);

    release_parse_moov_malloc(moov);
    return ret;
}

void print_moov_data(moov_report_t *report)
{
    int ret = 0;
    #define SUBFSIZE 4096
    static char sbuf[SUBFSIZE];

    ret = snprintf(sbuf, SUBFSIZE, 
        "hasaudio=%d, hasvideo=%d, audio_duration=%.3f, video_duration=%.3f, "
        "average_fps=%.3f, min_frame_dur=%.3f, max_frame_dur=%.3f, "
        "frame_dur_100_count=%d, frame_dur_200_count=%d, frame_dur_variance=%.3f, "
        ,
        report->hasaudio, report->hasvideo, report->audio_duration, report->video_duration,
        report->average_fps, report->min_frame_dur, report->max_frame_dur,
        report->frame_dur_100_count, report->frame_dur_200_count, report->frame_dur_variance
        );

    // if not the constant_fps, print frame_count every second
    if (report->frame_dur_variance > 1/(report->average_fps + 0.001)) {
        if (ret > 0 && ret < SUBFSIZE) {
            ret += snprintf(sbuf+ret, SUBFSIZE-ret, "second_frame_count=(");
        }
        for (int k=0; k<15 && ret > 0 && ret < SUBFSIZE; k++) {
            ret += snprintf(sbuf+ret, SUBFSIZE-ret, "%d,", report->second_frame_count[k]);
        }
        if (ret > 0 && ret < SUBFSIZE) {
            ret += snprintf(sbuf+ret, SUBFSIZE-ret, ")");
        }
    }

    _log_warning("%s\n", sbuf);
}

void release_parse_moov_malloc(moov_ctx_t *moov)
{
    if (moov->trak[0].pts_array) free(moov->trak[0].pts_array);
    if (moov->trak[1].pts_array) free(moov->trak[1].pts_array);
    memset(moov, 0, sizeof(moov_ctx_t));
}

int parse_moov(raw_data_t *raw, box_t *box1, moov_ctx_t *moov)
{
    if (box1->atom_type != MOOV_ATOM) {
        _log_error("this is not moov atom\n");
        return ERR_BOX_PASS_IN;
    } box_seek_to_data(raw, box1, 0);

    while (!box_end_assert(raw, box1)) {
        box_t _box2 = {0}, *box2 = &_box2;
        if (box_read_head(raw, box2) != 0) {
            return ERR_BOX_READ_HEAD;
        }

        if ((box2->atom_type != MVHD_ATOM) &&
            (box2->atom_type != TRAK_ATOM) &&
            (box2->atom_type != MVEX_ATOM) &&
            (box2->atom_type != IPMC_ATOM) ) {
            _log_warning("warning: '%s' is not moov-level atom (is this a QuickTime file?)\n", get4cc(box2->atom_type));
            //return ERR_BOX_UNSUPPORT;
        }

        int ret = 0;
        if (box2->atom_type == TRAK_ATOM) {
            ret = parse_trak(raw, box2, moov);
        } else if (box2->atom_type == MVHD_ATOM) {
            ret = parse_mvhd(raw, box2, moov);
        }
        if (ret != 0) {
            _log_error("parse '%s' failed\n", get4cc(box2->atom_type));
            return ret;
        }
        box_seek_to_end(raw, box2);
    }

    return 0;
}

int parse_mvhd(raw_data_t *raw, box_t *box1, moov_ctx_t *moov) 
{
    if (box1->atom_type != MVHD_ATOM) {
        _log_error("this is not mvhd atom\n");
        return ERR_BOX_PASS_IN;
    } box_seek_to_data(raw, box1, 1);

    mvhd_t *mvhd = &moov->mvhd;
    uint8_t *p = raw->pos;
    if (box1->version==1) {
        if (raw_read(&p, 28, 1, raw) != 1)  return ERR_NO_MORE_DATA;
        mvhd->creation_time     = BE_64(p);     p+=8;
        mvhd->modification_time = BE_64(p);     p+=8;
        mvhd->timescale         = BE_32(p);     p+=4;
        mvhd->duration          = BE_64(p);     p+=8;
    } else { // version==0
        if (raw_read(&p, 16, 1, raw) != 1)  return ERR_NO_MORE_DATA;
        mvhd->creation_time     = BE_32(p);     p+=4;
        mvhd->modification_time = BE_32(p);     p+=4;
        mvhd->timescale         = BE_32(p);     p+=4;
        mvhd->duration          = BE_32(p);     p+=4;
    }

    return 0;
}

int parse_trak(raw_data_t *raw, box_t *box1, moov_ctx_t *moov) 
{
    int ret = ERR_NONE;

    if (box1->atom_type != TRAK_ATOM) {
        _log_error("this is not trak atom\n");
        return ERR_BOX_PASS_IN;
    } box_seek_to_data(raw, box1, 0);

    while (!box_end_assert(raw, box1)) {
        box_t _box2 = {0}, *box2 = &_box2;
        if (box_read_head(raw, box2) != 0) {
            return ERR_BOX_READ_HEAD;
        }

        if ((box2->atom_type != TKHD_ATOM) &&
            (box2->atom_type != TREF_ATOM) &&
            (box2->atom_type != EDTS_ATOM) &&
            (box2->atom_type != MDIA_ATOM) ) {
            _log_warning("warning: '%s' is not trak-level atom (is this a QuickTime file?)\n", get4cc(box2->atom_type));
            //return ERR_BOX_UNSUPPORT;
        }

        if (box2->atom_type == MDIA_ATOM) {
            box_seek_to_data(raw, box2, 0);
            if ((ret = parse_mdia(raw, box2, moov))) {
                _log_error("parse trak failed\n");
                return ret;
            }
            box_seek_to_end(raw, box2);
        }
    }
    return 0;
}

int parse_mdia(raw_data_t *raw, box_t *box1, moov_ctx_t *moov)
{
    int ret = ERR_NONE;
    if (box1->atom_type != MDIA_ATOM) {
        _log_error("this is not mdia atom\n");
        return ERR_BOX_PASS_IN;
    } 

    box_t _box2 = {0}, *box2 = &_box2;
    uint8_t *atom_bytes = 0;

    /* 1 - get hdlr so we know trak type */
    box_reset(box2);
    box_seek_to_data(raw, box1, 0);
    while (!box_end_assert(raw, box1)) {
        if (box_read_head(raw, box2) == 0 && box2->atom_type == HDLR_ATOM) {
            break;
        } 
        box_seek_to_end(raw, box2);
    }
    hdlr_t hdlr = {0};
    if (box2->atom_type != HDLR_ATOM || parse_hdlr(raw, box2, &hdlr)) {
        _log_error("error parse hdlr\n");
        return ERR_PARSE_HDLR;
    }

    /* 2 - pick only one video/audio track */
    moov->vide_count += (hdlr.handler_type == 'vide');
    moov->soun_count += (hdlr.handler_type == 'soun');
    if ((hdlr.handler_type == 'vide' && moov->vide_trak ==0)) {
        moov->vide_trak = moov->curr_trak = &moov->trak[moov->trak_saved++];
    } else if (hdlr.handler_type == 'soun' && moov->soun_trak == 0) {
        moov->soun_trak = moov->curr_trak = &moov->trak[moov->trak_saved++];
    } else {
        return 0;
    }

    /* 3 - get mdhd so we know timescale */
    box_reset(box2);
    box_seek_to_data(raw, box1, 0);
    while (!box_end_assert(raw, box1)) {
        if (box_read_head(raw, box2) == 0 && box2->atom_type == MDHD_ATOM) {
            break;
        } 
        box_seek_to_end(raw, box2);
    } 
    if (box2->atom_type != MDHD_ATOM || parse_mdhd(raw, box2, moov)) {
        _log_error("error parse mdhd\n");
        return ERR_PARSE_MDHD;
    }

    /* 4 - get stbl */
    box_reset(box2);
    box_seek_to_data(raw, box1, 0);
    while (!box_end_assert(raw, box1)) {
        if (box_read_head(raw, box2) == 0 && box2->atom_type == MINF_ATOM) {
            break;
        } 
        box_seek_to_end(raw, box2);
    } 
    if (box2->atom_type != MINF_ATOM || parse_minf(raw, box2, moov)) {
        _log_error("error parse minf\n");
        return ERR_PARSE_MINF;
    }

    return 0;
}

int parse_hdlr(raw_data_t *raw, box_t *box1, hdlr_t *hdlr)
{
    if (box1->atom_type != HDLR_ATOM) {
        _log_error("this is not hdlr atom\n");
        return ERR_BOX_PASS_IN;
    } box_seek_to_data(raw, box1, 1);

    uint8_t *p = 0;
    if (raw_read(&p, 20, 1, raw) != 1) return ERR_NO_MORE_DATA;
    hdlr->handler_type = BE_32(p+4);

    return 0;
}

int parse_mdhd(raw_data_t *raw, box_t *box1, moov_ctx_t *moov) 
{
    if (box1->atom_type != MDHD_ATOM) {
        _log_error("this is not mdhd atom\n");
        return ERR_BOX_PASS_IN;
    } box_seek_to_data(raw, box1, 1);

    mdhd_t *mdhd = &moov->curr_trak->mdia.mdhd;
    uint8_t *p = 0;

    if (box1->version==1) {
        if (raw_read(&p, 28, 1, raw) != 1)  return ERR_NO_MORE_DATA;
        mdhd->creation_time     = BE_64(p);     p+=8;
        mdhd->modification_time = BE_64(p);     p+=8;
        mdhd->timescale         = BE_32(p);     p+=4;
        mdhd->duration          = BE_64(p);     p+=8;
    } else { // version==0
        if (raw_read(&p, 16, 1, raw) != 1)  return ERR_NO_MORE_DATA;
        mdhd->creation_time     = BE_32(p);     p+=4;
        mdhd->modification_time = BE_32(p);     p+=4;
        mdhd->timescale         = BE_32(p);     p+=4;
        mdhd->duration          = BE_32(p);     p+=4;
    }

    return 0;
}

int parse_minf(raw_data_t *raw, box_t *box1, moov_ctx_t *moov)
{
    int ret = ERR_NONE;

    if (box1->atom_type != MINF_ATOM) {
        _log_error("this is not minf atom\n");
        return ERR_BOX_PASS_IN;
    } box_seek_to_data(raw, box1, 0);

    while (!box_end_assert(raw, box1)) {
        box_t _box2 = {0}, *box2 = &_box2;
        if (box_read_head(raw, box2) != 0) {
            return ERR_BOX_READ_HEAD;
        }

        if ((box2->atom_type != VMHD_ATOM) &&
            (box2->atom_type != SMHD_ATOM) &&
            (box2->atom_type != HMHD_ATOM) &&
            (box2->atom_type != NMHD_ATOM) &&
            (box2->atom_type != DINF_ATOM) &&
            (box2->atom_type != STBL_ATOM) ) {
            _log_warning("warning: '%s' is not minf-level atom (is this a QuickTime file?)\n", get4cc(box2->atom_type));
            //return ERR_BOX_UNSUPPORT;
        }

        if (box2->atom_type == STBL_ATOM) {
            if ((ret = parse_stbl(raw, box2, moov))) {
                _log_error("parse stbl failed\n");
                return ret;
            }
        }
        box_seek_to_end(raw, box2);
    }
    return 0;
}

int parse_stbl(raw_data_t *raw, box_t *box1, moov_ctx_t *moov)
{
    int ret = ERR_NONE;
    if (box1->atom_type != STBL_ATOM) {
        _log_error("this is not stbl atom\n");
        return ERR_BOX_PASS_IN;
    } box_seek_to_data(raw, box1, 0);

    moov->curr_trak->stbl_offset = box1->atom_offset;
    moov->curr_trak->stbl_size   = box1->atom_size;
    //_log_error("stbl found\n");

    while (!box_end_assert(raw, box1)) {
        box_t _box2 = {0}, *box2 = &_box2;
        if (box_read_head(raw, box2) != 0) {
            return ERR_BOX_READ_HEAD;
        }

        if ((box2->atom_type != STSD_ATOM) &&
            (box2->atom_type != STTS_ATOM) &&
            (box2->atom_type != CTTS_ATOM) &&
            (box2->atom_type != STSC_ATOM) &&
            (box2->atom_type != STSZ_ATOM) &&
            (box2->atom_type != STZ2_ATOM) &&
            (box2->atom_type != STCO_ATOM) &&
            (box2->atom_type != CO64_ATOM) &&
            (box2->atom_type != STSS_ATOM) &&
            (box2->atom_type != STSH_ATOM) &&
            (box2->atom_type != PADB_ATOM) &&
            (box2->atom_type != STDP_ATOM) &&
            (box2->atom_type != SDTP_ATOM) &&
            (box2->atom_type != SBGP_ATOM) &&
            (box2->atom_type != SGPD_ATOM) &&
            (box2->atom_type != SUBS_ATOM) ) {
            _log_warning("warning: '%s' is not stbl-level atom (is this a QuickTime file?)\n", get4cc(box2->atom_type));
            //return ERR_BOX_UNSUPPORT;
        }
        box_seek_to_end(raw, box2);
    }

    box_t _box2 = {0}, *box2 = &_box2;
    uint8_t *atom_bytes = 0;

    /* 1 - parse_stts */
    box_reset(box2);
    box_seek_to_data(raw, box1, 0);
    while (!box_end_assert(raw, box1)) {
        if (box_read_head(raw, box2) == 0 && box2->atom_type == STTS_ATOM)
            break;
        box_seek_to_end(raw, box2);
    } 
    if (box2->atom_type != STTS_ATOM || parse_stts(raw, box2, moov)) {
        _log_error("error parse mdhd\n");
        return ERR_PARSE_STTS;
    }

    /* 1 - parse ctts */
    box_reset(box2);
    box_seek_to_data(raw, box1, 0);
    while (!box_end_assert(raw, box1)) {
        if (box_read_head(raw, box2) == 0 && box2->atom_type == CTTS_ATOM) {
            if (parse_ctts(raw, box2, moov)) {
                _log_error("error parse ctts\n");
                return ERR_PARSE_CTTS;
            }
        } 
        box_seek_to_end(raw, box2);
    } 

    return 0;
}

int parse_stts(raw_data_t *raw, box_t *box1, moov_ctx_t *moov)
{
    int ret = ERR_NONE;

    if (box1->atom_type != STTS_ATOM) {
        _log_error("this is not stts atom\n");
        return ERR_BOX_PASS_IN;
    } box_seek_to_data(raw, box1, 1);

    unsigned char *atom_bytes;
    if (raw_read(&atom_bytes, 4, 1, raw) != 1)
        return ERR_NO_MORE_DATA;
    uint32_t entry_count = BE_32(atom_bytes);
    if (raw_read(&atom_bytes, 8, entry_count, raw) != entry_count)
        return ERR_NO_MORE_DATA;

    trak_t *trak = moov->curr_trak;
    trak->framecount = 0;
    uint32_t timescale = trak->mdia.mdhd.timescale;
    for (int i=0; i<entry_count; i++) {
        uint32_t sample_count = BE_32(atom_bytes + i*8 + 0); 
        uint32_t sample_delta = BE_32(atom_bytes + i*8 + 4);
        trak->framecount += sample_count;
    }
    if (entry_count == 1) 
        trak->constant_fps = trak->framecount * timescale / (float) trak->mdia.mdhd.duration;

    trak->pts_array = (float*) malloc(sizeof(float) * trak->framecount);
    if (trak->pts_array == 0) 
        return ERR_MALLOC_BUF;

    for (int i=0, k=0; i<entry_count; i++) {
        uint32_t sample_count = BE_32(atom_bytes + i*8 + 0); 
        uint32_t sample_delta = BE_32(atom_bytes + i*8 + 4);
        for (int j=0; j<sample_count && k<trak->framecount; j++) {
            trak->pts_array[k++] = sample_delta * 1000.0 / trak->mdia.mdhd.timescale;
        }
    }
    for (int k=1; k<trak->framecount; k++) {
        trak->pts_array[k] += trak->pts_array[k-1];
        //_log_debug("delta %d : %.0f\n", k, trak->pts_array[k]);
    }

    _log_debug("stts %d sample found\n", trak->framecount);

    return 0;
}

int parse_ctts(raw_data_t *raw, box_t *box1, moov_ctx_t *moov)
{
    int ret = ERR_NONE;

    if (box1->atom_type != CTTS_ATOM) {
        _log_error("this is not ctts atom\n");
        return ERR_BOX_PASS_IN;
    } box_seek_to_data(raw, box1, 1);

    unsigned char *atom_bytes;
    if (raw_read(&atom_bytes, 4, 1, raw) != 1)
        return ERR_NO_MORE_DATA;
    uint32_t entry_count = BE_32(atom_bytes);
    if (raw_read(&atom_bytes, 8, entry_count, raw) != entry_count)
        return ERR_NO_MORE_DATA;

    trak_t *trak = moov->curr_trak;
    uint32_t framecount = 0;
    for (int i=0, k=0; i<entry_count; i++) {
        uint32_t sample_count = BE_32(atom_bytes + i*8 + 0); 
        uint32_t sample_delta = BE_32(atom_bytes + i*8 + 4);

        // in quicktime, it can be negative
        int32_t sample_offset = (int32_t) sample_delta;

        for (int j=0; j<sample_count && k<trak->framecount; j++) {
            trak->pts_array[k++] += sample_offset * 1000.0 / trak->mdia.mdhd.timescale;
        }
    }

    // for (int k=0; k<trak->framecount; k++) {
    //     _log_debug("delta %d : %.0f\n", k, trak->pts_array[k]);
    // }

    return 0;
}
