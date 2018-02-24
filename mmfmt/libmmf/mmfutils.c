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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mmfutils.h"


uint32_t mmf_showbe8 (uint8_t *p)
{
    uint32_t val   = *p++;
    return val;
}

uint32_t mmf_showbe16(uint8_t *p)
{
    uint32_t val   = *p++;
    val = (val<<8) | *p++;
    return val;
}

uint32_t mmf_showbe24(uint8_t *p)
{
    uint32_t val   = *p++;
    val = (val<<8) | *p++;
    val = (val<<8) | *p++;
    return val;
}

uint32_t mmf_showbe32(uint8_t *p)
{
    uint32_t val   = *p++;
    val = (val<<8) | *p++;
    val = (val<<8) | *p++;
    val = (val<<8) | *p++;
    return val;
}

uint64_t mmf_showbe64(uint8_t *p)
{
    uint32_t val1 = mmf_showbe32(p + 0);
    uint32_t val2 = mmf_showbe32(p + 4);
    return ((uint64_t)val1 << 32) | val2;
}

uint32_t mmf_buf_showbe8 (mmf_buf_t *pbuf)
{
    return pbuf->base[pbuf->pos];
}

uint32_t mmf_buf_showbe16(mmf_buf_t *pbuf)
{
    return mmf_showbe16(pbuf->base + pbuf->pos);
}

uint32_t mmf_buf_showbe24(mmf_buf_t *pbuf)
{
    return mmf_showbe24(pbuf->base + pbuf->pos);
}

uint32_t mmf_buf_showbe32(mmf_buf_t *pbuf)
{
    return mmf_showbe32(pbuf->base + pbuf->pos);
}

uint64_t mmf_buf_showbe64(mmf_buf_t *pbuf)
{
    return mmf_showbe64(pbuf->base + pbuf->pos);
}

uint32_t mmf_buf_getbe8 (mmf_buf_t *pbuf)
{
    return pbuf->base[pbuf->pos++];
}

uint32_t mmf_buf_getbe16(mmf_buf_t *pbuf)
{
    uint32_t val = mmf_showbe16(pbuf->base + pbuf->pos);
    pbuf->pos += 2;
    return val;
}

uint32_t mmf_buf_getbe24(mmf_buf_t *pbuf)
{
    uint32_t val = mmf_showbe24(pbuf->base + pbuf->pos);
    pbuf->pos += 3;
    return val;
}

uint32_t mmf_buf_getbe32(mmf_buf_t *pbuf)
{
    uint32_t val = mmf_showbe32(pbuf->base + pbuf->pos);
    pbuf->pos += 4;
    return val;
}

uint64_t mmf_buf_getbe64(mmf_buf_t *pbuf)
{
    uint64_t val = mmf_showbe64(pbuf->base + pbuf->pos);
    pbuf->pos += 8;
    return val;
}


void mmf_buf_attach(mmf_buf_t *pbuf, void *p, size_t sz)
{
    pbuf->base = p;
    pbuf->size = sz;
    pbuf->pos = 0;
}

void mmf_buf_detach(mmf_buf_t *pbuf)
{
    pbuf->base = 0;
    pbuf->size = 0;
    pbuf->pos = 0;
}

size_t mmf_buf_space(mmf_buf_t *pbuf)
{
    return pbuf->size - pbuf->pos;
}

size_t mmf_buf_full(mmf_buf_t *pbuf)
{
    return pbuf->pos;
}

int mmf_buf_malloc(mmf_buf_t *pbuf, size_t sz)
{
    void *p = malloc(sz);
    if (p == 0) {
        MMF_PERRSTR_EXIT(MMF_ERR_MALLOC);
    }
    
    mmf_buf_attach(pbuf, p, sz);
    return 0;
}

int mmf_buf_realloc(mmf_buf_t *pbuf, size_t sz)
{
    if (sz > pbuf->size) {
        void *p = realloc(pbuf, sz);
        if (p == 0) {
            MMF_PERRSTR_EXIT(MMF_ERR_MALLOC);
        } else {
            pbuf->base = p;
            pbuf->size = sz;
        }
    }
    return 0;
}

/**
 * @param memcpy_size
 *      num of data at front of @pbuf that will be copy to the new memory block
 */
int mmf_buf_enlarge(mmf_buf_t *pbuf, size_t sz, size_t memcpy_size)
{
    if (sz > pbuf->size) {
        void *p = malloc(sz);
        if (p == 0) {
            MMF_PERRSTR_EXIT(MMF_ERR_MALLOC);
        } else {
            memcpy_size = MMF_MIN(pbuf->size, memcpy_size);
            memcpy(p, pbuf->base, memcpy_size);
            mmf_buf_free(pbuf);
            mmf_buf_attach(pbuf, p, sz);
        }
    }
    return 0;
}

void mmf_buf_free(mmf_buf_t *pbuf)
{
    void *p = pbuf->base;
    mmf_buf_detach(pbuf);
    if (p) { free(p); }
}

void mmf_buf_reset(mmf_buf_t *pbuf)
{
    pbuf->pos = 0;
}

mmf_buf_t* mmf_buf_create(size_t sz)
{
    void *pbuf = calloc(1, sizeof(mmf_buf_t));
    if (pbuf == 0) {
        mmf_perror("failed to malloc(mmf_buf_t)");
        return 0;
    }
    
    if (sz) {
        int ret = mmf_buf_malloc(pbuf, sz);
        if (ret != MMF_RET_OK) {
            free(pbuf);
            mmf_perror("failed to malloc() for mmf_buf_t");
            return 0;
        }
    }
    
    return pbuf;
}

void mmf_buf_freep(mmf_buf_t **ppbuf)
{
    if (ppbuf == 0) return;
    mmf_buf_t *pbuf = *ppbuf;
    mmf_buf_free(pbuf);
    
    free(pbuf);
    *ppbuf = 0;
}

int mmf_buf_data_enlarge(mmf_buf_t *pbuf, size_t data_pos, size_t data_size)
{
    if (data_pos > pbuf->size) {
        MMF_PERRSTR_EXIT(MMF_ERR_OUT_OF_RANGE);
    }

    return mmf_buf_enlarge(pbuf, data_pos + data_size, data_size);
}

int mmf_buf_data_ref(mmf_data_t *pdata, mmf_buf_t *pbuf,
                     size_t data_pos, size_t data_size)
{
    if (data_pos > pbuf->size) {
        MMF_PERRSTR_EXIT(MMF_ERR_OUT_OF_RANGE);
    }
    
    if (data_size == 0) {
        data_size = pbuf->size - data_pos;
    }
    
    if (data_pos + data_size > pbuf->size) {
        MMF_PERRSTR_EXIT(MMF_ERR_NOT_ENOUGH_SPACE);
    }
    
    mmf_buf_attach(pdata, pbuf->base + data_pos, data_size);
    
    return 0;
}

int mmf_buf_data_ref_all(mmf_data_t *pdata, mmf_buf_t *pbuf)
{
    return mmf_buf_data_ref(pdata, pbuf, 0, pbuf->pos);
}

mmf_data_t mmf_buf_2_data(mmf_buf_t *pbuf)
{
    mmf_data_t data;
    mmf_buf_data_ref(&data, pbuf, 0, pbuf->pos);
    return data;
}

size_t mmf_buf_data_pos(mmf_data_t *pdata)
{
    return pdata->pos;
}

size_t mmf_buf_data_left(mmf_data_t *pdata)
{
    return pdata->size - pdata->pos;
}

size_t mmf_buf_data_skip(mmf_data_t *pdata, size_t n)
{
    return (pdata->pos += n);
}

/*
int mmf_pkt_data_layout(mmf_pkt_t *ppkt, size_t data_pos, size_t data_size)
{
    mmf_buf_t *pbuf = &ppkt->buf;
    mmf_buf_t *pdata = &ppkt->data;
    
    return mmf_buf_data_ref(pdata, pbuf, data_pos, data_size);
}

intptr_t mmf_pkt_data_pos(mmf_pkt_t *ppkt)
{
    return ppkt->data->base - ppkt->buf->base;
}

int mmf_pkt_data_size_limit(mmf_pkt_t *ppkt)
{
    return ppkt->pbuf->size - mmf_pkt_data_pos(ppkt);
}

int mmf_pkt_data_realloc(mmf_pkt_t *ppkt, size_t sz)
{
    mmf_buf_t *pbuf = &ppkt->buf;
    mmf_buf_t *pdata = &ppkt->data;
    
    intprt_t pos = mmf_pkt_data_pos(ppkt);
    if (!MMF_PKT_IS_VALID_POS(pos)) {
        MMF_PERRSTR_EXIT(MMF_ERR_ACCESS);
    }
    
    if (pos+sz < pbuf->size) {
        pdata->size = sz;
        return 0;
    }
    
    int ret = mmf_buf_realloc(pbuf, pos+sz);
    MMF_ERR_CHECK_EXIT(ret);
    
    //mmf_pkt_data_layout(ppkt, pos, sz);
    mmf_buf_attach(pdata, pbuf->base + pos, sz);
    return 0;
}

int mmf_pkt_data_enlarge(mmf_buf_t *ppkt, size_t sz)
{
    mmf_buf_t *pbuf = &ppkt->buf;
    mmf_buf_t *pdata = &ppkt->data;
    
    intprt_t pos = mmf_pkt_data_pos(ppkt);
    if (!MMF_PKT_IS_VALID_POS(pos)) {
        MMF_PERRSTR_EXIT(MMF_ERR_ACCESS);
    }
    
    if (pos+sz < pbuf->size) {
        pdata->size = sz;
        return 0;
    }
    
    int ret = mmf_buf_enlarge(pbuf, sz, pos);
    MMF_ERR_CHECK_EXIT(ret);
    
    mmf_buf_attach(pdata, pbuf->base + pos, sz);
    return 0;
}
*/
