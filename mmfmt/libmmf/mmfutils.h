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

#ifndef __MMFUTILS_H__
#define __MMFUTILS_H__


#include <stdint.h>
#include "sim_log.h"


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#define mmf_perror          printf
#define mmf_printf          printf

#define MMF_MIN(a,b)        ((a)<(b) ? (a) : (b))

typedef enum {
    MMF_RET_OK = 0,
    
    ///< pre exit reason
    MMF_QUIT_HELP = 1,
    MMF_QUIT_EOS = 2,
    
    ///< error return
    MMF_RET_ERR = 16,
    MMF_ERR_ARG_PARSE,
    MMF_ERR_ARG_CHECK,
	MMF_ERR_OVERFLOW,
	MMF_ERR_UNDERFLOW,
    MMF_ERR_NOT_ENOUGH_SPACE,
	MMF_ERR_NOT_ENOUGH_DATA,
    MMF_ERR_MALLOC,
	MMF_ERR_OPEN,
	MMF_ERR_WRITE,
	MMF_ERR_READ,
    MMF_ERR_EOS,
    MMF_ERR_ACCESS,
    MMF_ERR_OUT_OF_RANGE,
    MMF_ERR_UNMATCH,
    MMF_ERR_UNSUPPORT,
} MMF_RET_e;

#define MMF_PERROR_STR(err_code) do { \
    mmf_perror("error = [" #err_code "] in %s\n", __FUNCTION__); \
} while(0)
    
#define MMF_PERROR_VAL(err_val) do { \
    mmf_perror("error = [%d] in %s\n", err_val, __FUNCTION__); \
} while(0)

#define MMF_PERRSTR_EXIT(err_code) do { \
    MMF_PERROR_STR(err_code); \
    return err_code; \
} while(0)

#define MMF_PERRVAL_EXIT(err_code) do { \
    MMF_PERROR_VAL(err_code); \
    return err_code; \
} while(0)

#define MMF_ERR_CHECK_EXIT(err_code) do { \
    if (err_code >= MMF_RET_ERR) { MMF_PERRVAL_EXIT(err_code); } \
} while(0)

#define MMF_ERR_CHECK_JUMP(err_code, err_handler) do { \
    if (err_code >= MMF_RET_ERR) { goto err_handler; } \
} while(0)
    
#define MMF_ERR_CHECK_RET(err_expr, ret_code) do { \
    if (err_expr) { return ret_code; } \
} while(0)

typedef struct _mmf_buf {
    uint8_t     *base;
    size_t      size;
    int64_t     pos;
} mmf_buf_t;
    
void    mmf_buf_reset(mmf_buf_t *pbuf);
size_t  mmf_buf_space(mmf_buf_t *pbuf);
size_t  mmf_buf_full (mmf_buf_t *pbuf);

void    mmf_buf_attach(mmf_buf_t *pbuf, void *p, size_t sz);
void    mmf_buf_detach(mmf_buf_t *pbuf);
int     mmf_buf_malloc(mmf_buf_t *pbuf, size_t sz);
int     mmf_buf_realloc(mmf_buf_t *pbuf, size_t sz);
int     mmf_buf_enlarge(mmf_buf_t *pbuf, size_t sz, size_t memcpy_size);
void    mmf_buf_free(mmf_buf_t *pbuf);
mmf_buf_t*  mmf_buf_create(size_t sz);
void    mmf_buf_freep(mmf_buf_t **ppbuf);


typedef mmf_buf_t mmf_data_t;
int     mmf_buf_data_enlarge(mmf_data_t *pbuf, int64_t data_pos,
                             size_t data_size);
int     mmf_buf_data_ref(mmf_data_t *pdata, mmf_buf_t *pbuf,
                         int64_t data_pos, size_t data_size);
int     mmf_buf_data_ref_all(mmf_data_t *pdata, mmf_buf_t *pbuf);
mmf_data_t  mmf_buf_2_data(mmf_buf_t *pbuf);
int64_t mmf_buf_data_pos (mmf_data_t *pdata);
int64_t mmf_buf_data_left(mmf_data_t *pdata);
int64_t mmf_buf_data_skip(mmf_data_t *pdata, size_t n);



/*
typedef struct _mmf_pkt {
    mmf_buf_t   buf;        ///< buf used to hold data
    mmf_data_t  data;       ///< data reside in buf
} mmf_pkt_t;
 
int     mmf_pkt_data_layout(mmf_pkt_t *ppkt, size_t data_pos, size_t data_size);
intptr_t mmf_pkt_data_pos(mmf_pkt_t *ppkt);
#DEFINE  MMF_PKT_IS_VALID_POS(pos) ((pos)>=0)
int     mmf_pkt_data_size_limit(mmf_pkt_t *ppkt);
int     mmf_pkt_data_realloc(mmf_pkt_t *pbuf, size_t sz);
int     mmf_pkt_data_enlarge(mmf_pkt_t *pbuf, size_t sz);
*/


uint32_t mmf_showbe8 (uint8_t *p);
uint32_t mmf_showbe16(uint8_t *p);
uint32_t mmf_showbe24(uint8_t *p);
uint32_t mmf_showbe32(uint8_t *p);
uint64_t mmf_showbe64(uint8_t *p);

uint32_t mmf_buf_showbe8 (mmf_buf_t *pbuf);
uint32_t mmf_buf_showbe16(mmf_buf_t *pbuf);
uint32_t mmf_buf_showbe24(mmf_buf_t *pbuf);
uint32_t mmf_buf_showbe32(mmf_buf_t *pbuf);
uint64_t mmf_buf_showbe64(mmf_buf_t *pbuf);
    
uint32_t mmf_buf_getbe8 (mmf_buf_t *pbuf);
uint32_t mmf_buf_getbe16(mmf_buf_t *pbuf);
uint32_t mmf_buf_getbe24(mmf_buf_t *pbuf);
uint32_t mmf_buf_getbe32(mmf_buf_t *pbuf);
uint64_t mmf_buf_getbe64(mmf_buf_t *pbuf);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif //__MMFUTILS_H__
