/*
 * box_reader.h by <jianfneg15@staff.weibo.cn>
 */

#ifndef _BOX_READER_H_
#define _BOX_READER_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#define _log_level    4
#define _log_debug    if (6<=_log_level) printf
#define _log_info     if (5<=_log_level) printf
#define _log_warning  if (4<=_log_level) printf
#define _log_error    if (3<=_log_level) printf


#define MIN(a,b) ((a) > (b) ? (b) : (a))

#define BE_16(x) ((((uint8_t*)(x))[0] <<  8) | ((uint8_t*)(x))[1])

#define BE_24(x) (((uint32_t)(((uint8_t*)(x))[0]) << 16) |  \
                             (((uint8_t*)(x))[1]  <<  8) |  \
                              ((uint8_t*)(x))[2])

#define BE_32(x) (((uint32_t)(((uint8_t*)(x))[0]) << 24) |  \
                             (((uint8_t*)(x))[1]  << 16) |  \
                             (((uint8_t*)(x))[2]  <<  8) |  \
                              ((uint8_t*)(x))[3])

#define BE_64(x) (((uint64_t)(((uint8_t*)(x))[0]) << 56) |  \
                  ((uint64_t)(((uint8_t*)(x))[1]) << 48) |  \
                  ((uint64_t)(((uint8_t*)(x))[2]) << 40) |  \
                  ((uint64_t)(((uint8_t*)(x))[3]) << 32) |  \
                  ((uint64_t)(((uint8_t*)(x))[4]) << 24) |  \
                  ((uint64_t)(((uint8_t*)(x))[5]) << 16) |  \
                  ((uint64_t)(((uint8_t*)(x))[6]) <<  8) |  \
                  ((uint64_t)( (uint8_t*)(x))[7]))

enum error_code
{
    ERR_NONE = 0,
    ERR_OPEN_INPUT,
    ERR_MALLOC_BUF,
    //ERR_BOX_UNSUPPORT,
    ERR_BOX_READ_HEAD,
    ERR_BOX_PASS_IN,
    ERR_NO_MOOV,
    ERR_MALLOC_MOOV,
    ERR_READ_MOOV,
    ERR_MOOV_TOO_LARGE,
    ERR_NO_MORE_DATA,
    ERR_PARSE_HDLR,
    ERR_PARSE_MDHD,
    ERR_PARSE_STBL,
    ERR_PARSE_STTS,
    ERR_PARSE_CTTS,
    ERR_PARSE_MINF,
    ERR_PARSE_TRAK,
};


typedef struct box {
    uint32_t atom_level;

    uint32_t b_largesize;
    uint64_t atom_offset;
    uint64_t data_offset;

    uint32_t atom_type;
    uint64_t atom_size;
    
    uint32_t b_fullbox;
    uint32_t version;
    uint32_t flags;
} box_t;

void    box_reset(box_t *pbox);
int     box_read_head_f(FILE *infile, box_t *pbox);
int     box_seek_to_begin_f(FILE *infile, box_t *pbox);
int     box_seek_to_data_f(FILE *infile, box_t *pbox, int b_change_to_fullbox);
int     box_seek_to_end_f(FILE *infile, box_t *pbox);
int     box_end_assert_f(FILE *infile, box_t *pbox);


typedef struct _raw_data {
  size_t    data_size;
  size_t    buf_size;
  uint8_t*  pos;
  uint8_t*  buf;
} raw_data_t;


int     raw_malloc(raw_data_t *raw, size_t size);
void    raw_release(raw_data_t *raw);
void    raw_rewind(raw_data_t *raw);
size_t  raw_tell(raw_data_t *raw);
size_t  raw_left(raw_data_t *raw);
int     raw_read(uint8_t **p, size_t size, size_t count, raw_data_t *raw);
int     raw_seek(raw_data_t *raw, long int offset, int origin);

int     box_read_head(raw_data_t *raw, box_t *pbox);
int     box_seek_to_begin(raw_data_t *raw, box_t *pbox);
int     box_seek_to_data(raw_data_t *raw, box_t *pbox, int b_change_to_fullbox);
int     box_seek_to_end(raw_data_t *raw, box_t *pbox);
int     box_end_assert(raw_data_t *raw, box_t *pbox);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // _BOX_READER_H_
