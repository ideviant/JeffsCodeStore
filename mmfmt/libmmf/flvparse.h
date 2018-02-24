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

#ifndef __FLVPARSE_H__
#define __FLVPARSE_H__


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#define MMF_FLV1_FILE_HEADER_SIZE 	9
#define MMF_FLV1_FILE_HEADER_4CC 	'FLV1'
#define MMF_FLV1_DATA_OFFSET 		MMF_FLV1_FILE_HEADER_SIZE

#define MMF_FLV_FILE_HEADER_SIZE    MMF_FLV1_FILE_HEADER_SIZE
#define MMF_FLV_TAG_HEADER_SIZE 	11
#define MMF_FLV_TAG_HEADER_FMS_RSV2	0
typedef enum {
    MMF_FLV_TAG_TYPE_AUDIO = 8,
    MMF_FLV_TAG_TYPE_VIDEO = 9,
    MMF_FLV_TAG_TYPE_SCRIPT = 18,
    MMF_FLV_TAG_TYPE_AMF = MMF_FLV_TAG_TYPE_SCRIPT,
} MMF_FLV_TAG_TYPE_e;

typedef enum {
    MMF_FLV_AUD_FMT_LPCM_PE = 0,
    MMF_FLV_AUD_FMT_ADPCM = 1,
    MMF_FLV_AUD_FMT_MP3 = 2,
    MMF_FLV_AUD_FMT_LPCM_BE = 3,
    MMF_FLV_AUD_FMT_AAC = 10,
    MMF_FLV_AUD_FMT_SPEEX = 11,
} MMF_FLV_AUD_FMT_e;

typedef enum {
    MMF_FLV_AR_5K5 = 0,
    MMF_FLV_AR_11K = 1,
    MMF_FLV_AR_22K = 2,
    MMF_FLV_AR_44K = 3
} MMF_FLV_AR_e;

typedef enum {
    MMF_FLV_AUD_BIT_8 = 0,
    MMF_FLV_AUD_BIT_16 = 1,
} MMF_FLV_AUD_BIT_e;

typedef enum {
    MMF_FLV_CH_MONO = 0,
    MMF_FLV_CH_STEREO = 1,
} MMF_FLV_CH_e;

typedef enum {
    MMF_FLV_AAC_PKT_SEQ = 0,
    MMF_FLV_AAC_PKT_RAW = 1,
} MMF_FLV_AAC_PKT_e;

typedef enum {
    MMF_FLV_VID_FRAME_KEY = 1,
    MMF_FLV_VID_FRAME_NKEY = 2,
    MMF_FLV_VID_FRAME_DISPOSABLE = 3,
    MMF_FLV_VID_FRAME_SERVER_KEY = 4,
    MMV_FLV_VID_FRAME_CRTL = 5
} MMF_FLV_VID_FRAME_e;

typedef enum {
    MMF_FLV_VID_CODEC_H263 = 2,
    MMF_FLV_VID_CODEC_SCREEN = 3,
    MMF_FLV_VID_CODEC_VP6 = 4,
    MMF_FLV_VID_CODEC_VP6A = 5,
    MMF_FLV_VID_CODEC_SCREEN_V2 = 6,
    MMF_FLV_VID_CODEC_AVC = 7,
} MMF_FLV_VID_CODEC_e;

typedef enum {
    MMF_FLV_AVC_PKT_SEQ = 0,
    MMF_FLV_AVC_PKT_NALU = 1,
    MMF_FLV_AVC_PKT_EOS = 2,
} MMF_FLV_AVC_PKT_e;

typedef enum {
    MMF_FLV_AMF_NUMBER = 0,
    MMF_FLV_AMF_BOOLEAN = 1,
    MMF_FLV_AMF_STRING = 2,
    MMF_FLV_AMF_OBJECT = 3,
    MMF_FLV_AMF_MOVCLIP = 4,
    MMF_FLV_AMF_NULL = 5,
    MMF_FLV_AMF_UNDEF = 6,
    MMF_FLV_AMF_REF = 7,
    MMF_FLV_AMF_ECMA = 8,
    MMF_FLV_AMF_OBJEND = 9,
    MMF_FLV_AMF_ARRAY = 10,
    MMF_FLV_AMF_DATE = 11,
    MMF_FLV_AMF_LONGSTR = 12,
} MMF_FLV_AMF_e;

typedef enum {
    MMF_AVC_NALU_UNSPECIFIED = 0,
    MMF_AVC_NALU_NON_IDR = 1,
    MMF_AVC_NALU_PART_A = 2,
    MMF_AVC_NALU_PART_B = 3,
    MMF_AVC_NALU_PART_C = 4,
    MMF_AVC_NALU_IDR = 5,
    MMF_AVC_NALU_SEI = 6,
    MMF_AVC_NALU_SPS = 7,
    MMF_AVC_NALU_PPS = 8,
    MMF_AVC_NALU_AUD = 9,
    MMF_AVC_NALU_END_SEQ = 10,
    MMF_AVC_NALU_END_STREAM = 11,
    MMF_AVC_NALU_FILTER = 12,
    MMF_AVC_NALU_SPS_EXT = 13,
    /* reserved 14..18 */
    MMF_AVC_NALU_AUX = 19,
    /* reserved 20..23 */
    /* unspecified 24..31 */
} MMF_AVC_NALU_TYPE_e;

typedef struct _mmf_flv_tag {
    int         i_tag_type;         // MMF_FLV_TAG_TYPE_e
    int         i_data_size;
    int         i_time_stamp;       // <ms>
    int         i_stream_id;
    
    ///< non-std prop.
    int         i_tag_idx;
    int64_t     i_tag_pos;          // byte position
    int         i_tag_size;
    int64_t     i_tag_dts;          // <us>
    int64_t     i_tag_pts;          // <us>
} mmf_flv_tag_t;

typedef struct _mmf_flv_ctx {
    ///< static contex
    int         b_video;
    int         b_audio;
    int         b_script;
    
    ///< run time contex
    FILE*           fp;
    int64_t         i_byte_pos;
    
    int             i_tag_idx;
    mmf_flv_tag_t   tag;            // last tag record
} mmf_flv_ctx_t;


typedef struct _mmf_aud_tag {
    mmf_flv_tag_t   tag;
    
    int         i_codec_id;         // 4b, MMF_FLV_AUD_CODEC_e
    int         i_rate;             // 2b, MMF_FLV_AR_e
    int         i_bits;             // 1b, MMF_FLV_AUD_BIT_e
    int         b_stereo;
    
    ///< for aac;
    int         i_pkt_type;         // MMF_FLV_AAC_PKT_e
} mmf_aud_tag_t;

typedef struct _mmf_vid_tag {
    mmf_flv_tag_t   tag;
    
    int         i_frame_type;       // 4b, MMF_FLV_VID_FRAME_e
    int         i_codec_id;         // 4b, MMF_FLV_VID_CODEC_e
    
    ///< for avc
    int         i_pkt_type;         // MMF_FLV_AVC_PKT_e
    int         i_pts_offset;       // pts offset to dts in <ms>
} mmf_vid_tag_t;

typedef struct _mmf_amf_tag {
    mmf_flv_tag_t   tag;
    
} mmf_amf_tag_t;

enum flvparse_ios_ch {
    FLVPARSE_IOS_SRC = 0,
    FLVPARSE_IOS_DST = 1,
    FLVPARSE_IOS_CNT,
};

typedef struct _flvparse_opt {
    ios_t       ios[2];
    int         frame_range[2];
} flvparse_opt_t;

int flv_parse(int argc, char **argv);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  // __FLVPARSE_H__
