/*
 * moov_parse.h by <jianfneg15@staff.weibo.cn>
 */

#ifndef _MOOV_PARSE_H_
#define _MOOV_PARSE_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


typedef struct _moov_context {
	float 		movie_duration;				// ms
	float 		video_duration;				// ms
	float 		audio_duration;				// ms
	uint32_t	hasaudio : 16;
	uint32_t	hasvideo : 16;

	// audio properties

	// video properties				
	uint32_t 	framecount;
	float		average_fps;
	float		min_frame_dur;				// ms
	float		max_frame_dur;				// ms
	uint32_t	frame_dur_100_count;
	uint32_t	frame_dur_200_count;
	uint32_t	second_frame_count[15];

	/** some constant_fps like 23.97 don't have strict unify frame-duration, 
	 * use frame_dur_variance < minimus to guess in this case
	 */
	//float		constant_fps;
	float		frame_dur_variance;			

	// raw data info
	//uint32_t	moov_offset;
	uint32_t	moov_size;
	uint32_t	video_stbl_offset;
	uint32_t	video_stbl_size;
	uint32_t	audio_stbl_offset;
	uint32_t	audio_stbl_size;
} moov_report_t;


int read_moov_data(const char *input, raw_data_t *raw, int max_alloc_size);
int parse_moov_data(raw_data_t *raw, moov_report_t *moov);
void print_moov_data(moov_report_t *moov);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // _MOOV_PARSE_H_
