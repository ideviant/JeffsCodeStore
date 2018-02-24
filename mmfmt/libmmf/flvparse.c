/*****************************************************************************
 * Copyright 2016 Jeff <ggjogh@gmail.com>
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

#include <assert.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h> 

#include "sim_opt.h"
#include "mmfutils.h"
#include "flvparse.h"


static int      mmf_flv_parse_avc_nalu(mmf_flv_ctx_t *pflv, mmf_data_t *pdata);
static int64_t  mmf_flv_data_pos_2_file_pos(mmf_flv_ctx_t *pflv, int64_t pos);


static const char *mmf_flv_itoa(int val)
{
    static char sbuf[64] = { 0 };
    snprintf(sbuf, 64, "%d", val);
    return sbuf;
}

const char *mmf_itoa_tag_type(int val)
{
    switch (val) {
        case MMF_FLV_TAG_TYPE_VIDEO: return "video";
        case MMF_FLV_TAG_TYPE_AUDIO: return "audio";
        case MMF_FLV_TAG_TYPE_SCRIPT: return "script";
        default: break;
    }
    
    return mmf_flv_itoa(val);
}

const char *mmf_itoa_aud_codec(int val) {
    switch (val) {
        case MMF_FLV_AUD_FMT_AAC:   return "AAC";
        case MMF_FLV_AUD_FMT_MP3:   return "MP3";
        case MMF_FLV_AUD_FMT_SPEEX: return "SPEEX";
        default: break;
    }
    
    return mmf_flv_itoa(val);
}

const char *mmf_itoa_aud_rate(int val) {
    switch (val) {
        case MMF_FLV_AR_11K: return "11K";
        case MMF_FLV_AR_22K: return "22K";
        case MMF_FLV_AR_44K: return "44K";
        case MMF_FLV_AR_5K5: return "5.5K";
        default: break;
    }
    
    return mmf_flv_itoa(val);
}

const char *mmf_itoa_aac_pkt_type(int val) {
    switch (val) {
        case MMF_FLV_AAC_PKT_SEQ: return "SEQ";
        case MMF_FLV_AAC_PKT_RAW: return "RAW";
        default: break;
    }
    
    return mmf_flv_itoa(val);
}

const char *mmf_itoa_vid_codec(int val) {
    switch (val) {
        case MMF_FLV_VID_CODEC_AVC:  return "AVC";
        case MMF_FLV_VID_CODEC_H263: return "H263";
        default: break;
    }
    
    return mmf_flv_itoa(val);
}

const char *mmf_itoa_vid_frame_type(int val)
{
    switch (val) {
        case MMF_FLV_VID_FRAME_KEY:  return "KEY";
        case MMF_FLV_VID_FRAME_NKEY: return "non-KEY";
        default: break;
    }
    return mmf_flv_itoa(val);
}

const char *mmf_itoa_avc_pkt_type(int val) {
    switch (val) {
        case MMF_FLV_AVC_PKT_SEQ:  return "SEQ";
        case MMF_FLV_AVC_PKT_NALU: return "VLC";
        case MMF_FLV_AVC_PKT_EOS:  return "EOS";
        default: break;
    }
    
    return mmf_flv_itoa(val);
}

const char *mmf_itoa_avc_nalu_type(int val) {
    switch (val) {
        case MMF_AVC_NALU_NON_IDR: return "non-IDR";
        case MMF_AVC_NALU_PART_A: return "partA";
        case MMF_AVC_NALU_PART_B: return "partB";
        case MMF_AVC_NALU_PART_C: return "partc";
        case MMF_AVC_NALU_IDR: return "IDR";
        case MMF_AVC_NALU_SEI: return "SEI";
        case MMF_AVC_NALU_SPS: return "SPS";
        case MMF_AVC_NALU_PPS: return "PPS";
        case MMF_AVC_NALU_AUD: return "AUD";
        default: break;
    }
    
    return mmf_flv_itoa(val);
}

void mmf_flv_print_tag_header(mmf_flv_tag_t *ptag)
{
    mmf_printf("tag#%d: @0x%08"PRIx64", <%s>, data_size=0x%08X, pts=%"PRId64"\n",
               ptag->i_tag_idx, ptag->i_tag_pos,
               mmf_itoa_tag_type(ptag->i_tag_type),
               ptag->i_data_size, ptag->i_tag_pts);
}

void mmf_flv_print_aud_tag_header(mmf_aud_tag_t *paud)
{
    mmf_printf("\t codec=%s, rate=%s, bits=%d, ch=%d\n",
               mmf_itoa_aud_codec(paud->i_codec_id),
               mmf_itoa_aud_rate(paud->i_rate),
               paud->i_bits == MMF_FLV_AUD_BIT_8 ? 8 : 16,
               paud->b_stereo ? 2 : 1);
    
    if (paud->i_codec_id == MMF_FLV_AUD_FMT_AAC) {
        mmf_printf("\t AAC: <%s>\n", mmf_itoa_aac_pkt_type(paud->i_pkt_type));
    }
}

void mmf_flv_print_vid_tag_header(mmf_vid_tag_t *pvid)
{
    mmf_printf("\t codec=%s, frame_type=%s\n",
               mmf_itoa_vid_codec(pvid->i_codec_id),
               mmf_itoa_vid_frame_type(pvid->i_frame_type));
    
    if (pvid->i_codec_id == MMF_FLV_VID_CODEC_AVC) {
        mmf_printf("\t AVC: <%s>, pts_offset=%d\n",
                   mmf_itoa_avc_pkt_type(pvid->i_pkt_type),
                   pvid->i_pts_offset);
    }
}

int mmf_flv_parse_file_header(mmf_buf_t     *pdata,
                              mmf_flv_ctx_t *pflv)
{
    if (mmf_buf_data_left(pdata) < MMF_FLV_FILE_HEADER_SIZE) {
        mmf_perror("not enough space for flv header\n");
        return MMF_ERR_NOT_ENOUGH_DATA;
    }
    
    uint32_t flv4cc = mmf_buf_getbe32(pdata);
    if (flv4cc != 'FLV\1') {
        mmf_perror("head 4cc [%08"PRIX32"] is not 'FLV1':[%08"PRIX32"]\n",
                flv4cc, 'FLV\1');
        return MMF_RET_ERR;
    }

    uint32_t flags = mmf_buf_getbe8(pdata);
    pflv->b_audio = flags & 4;
    pflv->b_video = flags & 1;

    uint32_t offset = mmf_buf_getbe32(pdata);
    if (offset != 9) {
        mmf_perror("body offset <%"PRId32"> is not <9>", offset);
        return MMF_RET_ERR;
    }

    return 0;
}

int mmf_flv_parse_tag_header(mmf_buf_t *pdata, mmf_flv_tag_t *ptag)
{
    if (mmf_buf_data_left(pdata) < MMF_FLV_TAG_HEADER_SIZE) {
        mmf_perror("not enough data for tag header\n");
        return MMF_ERR_NOT_ENOUGH_DATA;
    }
    
    ptag->i_tag_type    = mmf_buf_getbe8 (pdata) & 0x1f;
    ptag->i_data_size   = mmf_buf_getbe24(pdata);
    
    uint32_t i_ts_lsb24 = mmf_buf_getbe24(pdata);
    uint32_t i_ts_msb8  = mmf_buf_getbe8 (pdata);
    ptag->i_time_stamp  = (i_ts_msb8 << 24) + i_ts_lsb24;
    
    ptag->i_stream_id   = mmf_buf_getbe24(pdata);
    
    return 0;
}

int mmf_flv_parse_aud_tag_data(mmf_flv_ctx_t *pflv, mmf_data_t *pdata)
{
    mmf_aud_tag_t _aud, *paud = &_aud;
    
    if (mmf_buf_data_left(pdata) < 2) {
        mmf_perror("not enough data for AAC tag data header\n");
        return MMF_ERR_NOT_ENOUGH_DATA;
    }
    
    uint32_t val0    = mmf_buf_getbe8 (pdata);
    paud->i_codec_id = (val0 >> 4) & 0xf;
    paud->i_rate     = (val0 >> 2) & 3;
    paud->i_bits     = (val0 >> 1) & 1;
    paud->b_stereo   = (val0 >> 0) & 1;
    
    if (paud->i_codec_id == MMF_FLV_AUD_FMT_AAC) {
        paud->i_pkt_type = mmf_buf_getbe8(pdata);
    }
    
    mmf_flv_print_aud_tag_header(paud);

    return 0;
}

int mmf_flv_parse_vid_tag_data(mmf_flv_ctx_t *pflv, mmf_data_t *pdata)
{
    mmf_vid_tag_t _vid, *pvid = &_vid;
    
    if (mmf_buf_data_left(pdata) < 5) {
        mmf_perror("not enough data for AVC tag data header\n");
        return MMF_ERR_NOT_ENOUGH_DATA;
    }
    
    uint32_t val0       = mmf_buf_getbe8 (pdata);
    pvid->i_frame_type  = (val0 >> 4) & 0xf;
    pvid->i_codec_id    = (val0 >> 0) & 0xf;
    
    if (pvid->i_codec_id == MMF_FLV_VID_CODEC_AVC) {
        pvid->i_pkt_type    = mmf_buf_getbe8 (pdata);
        pvid->i_pts_offset  = mmf_buf_getbe24(pdata);
    }
    
    mmf_flv_print_vid_tag_header(pvid);
    
    if (pvid->i_codec_id == MMF_FLV_VID_CODEC_AVC) {
        if (pvid->i_pkt_type == MMF_FLV_AVC_PKT_SEQ) {
            //mmf_flv_parse_avc_cfg(pflv, pbuf);
        } else if (pvid->i_pkt_type == MMF_FLV_AVC_PKT_NALU) {
            mmf_flv_parse_avc_nalu(pflv, pdata);
        }
    }

    return 0;
}

int mmf_flv_parse_amf_tag_data(mmf_flv_ctx_t *pflv, mmf_data_t *pdata)
{
    return 0;
}

int mmf_flv_parse_tag_data(mmf_flv_ctx_t *pflv, mmf_data_t *pdata)
{
    int ret = MMF_RET_OK;
    mmf_flv_tag_t *ptag = &pflv->tag;
    
    switch (ptag->i_tag_type) {
        case MMF_FLV_TAG_TYPE_AUDIO:
            ret = mmf_flv_parse_aud_tag_data(pflv, pdata);
            break;
        case MMF_FLV_TAG_TYPE_VIDEO:
            ret = mmf_flv_parse_vid_tag_data(pflv, pdata);
            break;
        case MMF_FLV_TAG_TYPE_SCRIPT:
            ret = mmf_flv_parse_amf_tag_data(pflv, pdata);
            break;
        default:
            MMF_PERRSTR_EXIT(MMF_ERR_UNSUPPORT);
            break;
    }
    
    return ret;
}

/**
 * One or more AVC NALUs (Full frames are required):
 *      nalu_size + nalu_data + ... + nalu_size + nalu_data
 */
int mmf_flv_parse_avc_nalu(mmf_flv_ctx_t *pflv, mmf_data_t *pdata)
{
    while(mmf_buf_data_left(pdata) > 4)
    {
        if (mmf_buf_data_left(pdata) < 5) {
            mmf_perror("sizeof(AVC DATA) < 5\n");
            return MMF_ERR_NOT_ENOUGH_DATA;
        }

        int64_t pos = mmf_flv_data_pos_2_file_pos(pflv, mmf_buf_data_pos(pdata));
        uint32_t i_nalu_size = mmf_buf_getbe32(pdata);

        if (i_nalu_size > mmf_buf_data_left(pdata)) {
            mmf_perror("\t @0x%08"PRIx64": nalu_size=0x%08x exceed data_size=0x%08"PRIx64"\n",
                       pos, i_nalu_size, mmf_buf_data_left(pdata));
            return MMF_ERR_OUT_OF_RANGE;
        }

        if (i_nalu_size < 1) {
            mmf_perror("nalu_size < 1\n");
            return MMF_ERR_NOT_ENOUGH_DATA;
        }

        uint32_t val0 = mmf_buf_showbe8(pdata);
        int i_ref_idc   = (val0 >> 5) & 0x3;
        int i_nalu_type = (val0 >> 0) & 0x1f;
        mmf_printf("\t NALU @0x%08"PRIx64": size=0x%06x, ref_idc=%d, <%s>\n",
                   pos, i_nalu_size, i_ref_idc,
                   mmf_itoa_avc_nalu_type(i_nalu_type));

        mmf_buf_data_skip(pdata, i_nalu_size);
    }

    if (mmf_buf_data_left(pdata) != 0) {
        mmf_perror("\t error packed multi-nalu AVC tag\n");
        int64_t pos = mmf_flv_data_pos_2_file_pos(pflv, mmf_buf_data_pos(pdata));
        mmf_perror("\t @0x%08"PRIx64": 0x%"PRIx64" byte data still left \n",
                   pos, mmf_buf_data_left(pdata));
        if (mmf_buf_data_left(pdata) > 4) {
            mmf_perror("\t\t next 4-byte is 0x%08X\n", mmf_buf_showbe32(pdata));
        }
        MMF_PERRSTR_EXIT(MMF_ERR_UNMATCH);
    }

    return 0;
}

int mmf_flv_read_file_header(mmf_flv_ctx_t *pflv)
{
    uint8_t sbuf[MMF_FLV_FILE_HEADER_SIZE] = { 0 };
    size_t nr = fread(sbuf, 1, MMF_FLV_FILE_HEADER_SIZE, pflv->fp);
    if (nr < MMF_FLV_FILE_HEADER_SIZE) {
        mmf_perror("Can't read enough byte for file header\n");
        return MMF_ERR_NOT_ENOUGH_DATA;
    }
    
    mmf_buf_t _buf, *pbuf = &_buf;
    mmf_buf_attach(pbuf, sbuf, MMF_FLV_FILE_HEADER_SIZE);
    mmf_data_t data = mmf_buf_2_data(pbuf);
    int ret = mmf_flv_parse_file_header(&data, pflv);
    MMF_ERR_CHECK_EXIT(ret);
    
    pflv->i_byte_pos += MMF_FLV_FILE_HEADER_SIZE;
    
    return 0;
}

int mmf_flv_read_tag_size(mmf_flv_ctx_t *pflv)
{
    uint8_t sbuf[4];
    size_t nr = fread(sbuf, 1, 4, pflv->fp);
    if (nr < 4) {
        if (nr == 0 && feof(pflv->fp)) {
            mmf_printf("\n== EOS ==\n\n");
            return MMF_QUIT_EOS;
        }
        
        mmf_perror("Can't read enough byte for pre-tag size\n");
        return MMF_ERR_NOT_ENOUGH_DATA;
    }
    
    int i_pre_tag_size = (int)mmf_showbe32(sbuf);
    if (i_pre_tag_size != pflv->tag.i_tag_size) {
        mmf_perror("pre tag size (%d) not match %d + %d (data size)\n",
                   i_pre_tag_size,
                   MMF_FLV_TAG_HEADER_SIZE,
                   pflv->tag.i_data_size);
        MMF_PERRSTR_EXIT(MMF_ERR_UNMATCH);
    }
    
    pflv->i_byte_pos += 4;
    pflv->i_tag_idx += 1;
    
    return 0;
}

int mmf_flv_read_tag_header(mmf_flv_ctx_t *pflv)
{
    uint8_t sbuf[MMF_FLV_TAG_HEADER_SIZE] = { 0 };
    size_t nr = fread(sbuf, 1, MMF_FLV_TAG_HEADER_SIZE, pflv->fp);
    if (nr == 0 && feof(pflv->fp)) {
        mmf_printf("\n== EOS ==\n\n");
        return MMF_QUIT_EOS;
    } else if (nr < MMF_FLV_TAG_HEADER_SIZE) {
        mmf_perror("Can't read enough byte for tag header\n");
        return MMF_ERR_NOT_ENOUGH_DATA;
    }
    
    mmf_buf_t _buf, *pbuf = &_buf;
    mmf_buf_attach(pbuf, sbuf, MMF_FLV_TAG_HEADER_SIZE);
    mmf_data_t data = mmf_buf_2_data(pbuf);
    
    mmf_flv_tag_t *ptag = &pflv->tag;
    mmf_flv_parse_tag_header(&data, ptag);
    
    ptag->i_tag_idx = pflv->i_tag_idx;
    ptag->i_tag_pos = pflv->i_byte_pos;
    ptag->i_tag_size = MMF_FLV_TAG_HEADER_SIZE + ptag->i_data_size;
    ptag->i_tag_dts = 1000 * ptag->i_time_stamp;
    ptag->i_tag_pts = ptag->i_tag_dts;
    pflv->i_byte_pos += MMF_FLV_TAG_HEADER_SIZE;
    
    return 0;
}

int64_t mmf_flv_data_pos_2_file_pos(mmf_flv_ctx_t *pflv, int64_t pos)
{
    mmf_flv_tag_t *ptag = &pflv->tag;
    return ptag->i_tag_pos + MMF_FLV_TAG_HEADER_SIZE + pos;
}

int mmf_flv_read_tag_data(mmf_flv_ctx_t *pflv, mmf_buf_t *pbuf)
{
    mmf_flv_tag_t *ptag = &pflv->tag;
    int ret = mmf_buf_enlarge(pbuf, ptag->i_tag_size, 0);
    MMF_ERR_CHECK_EXIT(ret);
    
    mmf_buf_reset(pbuf);
    size_t nr = fread(pbuf->base, 1, ptag->i_data_size, pflv->fp);
    if (nr < ptag->i_data_size) {
        mmf_perror("Can't read enough byte for tag payload\n");
        return MMF_ERR_NOT_ENOUGH_DATA;
    }
    pbuf->pos = ptag->i_data_size;
    pflv->i_byte_pos += ptag->i_data_size;
        
    return 0;
}

/*****************************************************************************
 * cmdl interfaces
 ****************************************************************************/
int flvparse_arg_init (flvparse_opt_t *cfg, int argc, char *argv[])
{
    memset(cfg, 0, sizeof(flvparse_opt_t));
    cfg->frame_range[1] = INT_MAX;
    return 0;
}

int flvparse_arg_parse(flvparse_opt_t *cfg, int argc, char *argv[])
{
    int i;
    
    ENTER_FUNC();
    
    if (argc<2) {
        xerr("No arg specified.\n");
        return -1;
    }
    
    const char* start_opts = "h, help, i, src, x, xl, xlevel, xall, xnon";
    if (argv[1][0]!='-' || 0 > field_in_record(argv[1], start_opts))
    {
        xerr("1st opt not in `%s`\n", start_opts);
        return -1;
    }
    
    /**
     *  loop options
     */
    for (i=1; i>=0 && i<argc; )
    {
        xdbg("@cmdl>> argv[%d]=%s\n", i, argv[i]);
        
        char *arg = argv[i];
        if (arg[0]!='-') {
            xerr("`%s` is not an option\n", arg);
            return -i;
        }
        arg += 1;
        ++i;
        
        if (0==strcmp(arg, "h") || 0==strcmp(arg, "help")) {
            return 0;
        } else if (0==strcmp(arg, "i") || 0==strcmp(arg, "src")) {
            char *path = 0;
            i = arg_parse_str(i, argc, argv, &path);
            ios_cfg(cfg->ios, FLVPARSE_IOS_SRC, path, "rb");
        } else if (0==strcmp(arg, "n-frame") || 0==strcmp(arg, "nframe") ||
                   0==strcmp(arg, "f")       || 0==strcmp(arg, "frame")) {
            int nframe = 0;
            i = arg_parse_int(i, argc, argv, &nframe);
            cfg->frame_range[1] = nframe + cfg->frame_range[0];
        } else if (0==strcmp(arg, "f-start")) {
            i = arg_parse_int(i, argc, argv, &cfg->frame_range[0]);
        } else if (0==strcmp(arg, "f-range")) {
            i = arg_parse_range(i, argc, argv, cfg->frame_range);
        } else {
            xerr("unknown option `-%s`\n", arg);
            return -(--i);
        }
    }
    
    LEAVE_FUNC();
    
    return i;

}

int flvparse_arg_check(flvparse_opt_t *cfg, int argc, char *argv[])
{
    if (!ios_open(cfg->ios, FLVPARSE_IOS_CNT, 0)) {
        ios_close(cfg->ios, FLVPARSE_IOS_CNT);
        return 1;
    }
    
    return 0;
}

int flvparse_arg_close(flvparse_opt_t *cfg)
{
    ios_close(cfg->ios, FLVPARSE_IOS_CNT);

    return 0;
}

int flvparse_arg_help()
{
    printf("flv parser. \nOptions:\n");
    printf("\t -i|-src name<%%s> {...props...}\n");
    printf("\t ...frame range...   <%%d~%%d>\n");

    printf("\nset frame range as follow:\n");
    printf("\t [-f-range    <%%d~%%d>]\n");
    printf("\t [-f-start    <%%d>]\n");
    printf("\t [-frame|-f   <%%d>]\n");
    
    return 0;
}

int flv_parse(int argc, char **argv)
{
    int ret = 0;
    flvparse_opt_t  cfg;
    flvparse_arg_init(&cfg, argc, argv);
    
    ret = flvparse_arg_parse(&cfg, argc, argv);
    if (ret == 0) {
        flvparse_arg_help();
        return 0;
    }
    MMF_ERR_CHECK_RET(ret < 0, MMF_ERR_ARG_PARSE);
    
    ret = flvparse_arg_check(&cfg, argc, argv);
    MMF_ERR_CHECK_JUMP(ret, flv_arg_parse_err);
    
    mmf_flv_ctx_t _flv, *pflv = &_flv;
    mmf_buf_t _buf = { 0 }, *pbuf = &_buf;
    
    pflv->fp = cfg.ios[FLVPARSE_IOS_SRC].fp;
    ret = mmf_flv_read_file_header(pflv);
    MMF_ERR_CHECK_JUMP(ret, flv_parse_err);
    ret = mmf_flv_read_tag_size(pflv);
    MMF_ERR_CHECK_JUMP(ret, flv_parse_err);
    
    /************* frame loop **************/
    while (1) {
        ret = mmf_flv_read_tag_header(pflv);
        if (ret == MMF_QUIT_EOS) {
            break;
        }
        MMF_ERR_CHECK_JUMP(ret, flv_parse_err);
        
        if (pflv->tag.i_tag_idx >= cfg.frame_range[0]) {
            mmf_flv_print_tag_header(&pflv->tag);
        }
        
        ret = mmf_flv_read_tag_data(pflv, pbuf);
        MMF_ERR_CHECK_JUMP(ret, flv_parse_err);
        
        if (pflv->tag.i_tag_idx >= cfg.frame_range[0]) {
            mmf_data_t data = mmf_buf_2_data(pbuf);
            int ret2 = mmf_flv_parse_tag_data(pflv, &data);
            MMF_ERR_CHECK_JUMP(ret2, flv_parse_err);
        }
        
        ret = mmf_flv_read_tag_size(pflv);
        if (ret == MMF_QUIT_EOS) {
            break;
        }
        MMF_ERR_CHECK_JUMP(ret, flv_parse_err);
        if (pflv->tag.i_tag_idx > cfg.frame_range[1]) {
            break;
        }
    } // end frame loop

flv_parse_err:
    mmf_buf_free(pbuf);
    
flv_arg_parse_err:
    flvparse_arg_close(&cfg);

    return ret;
}
