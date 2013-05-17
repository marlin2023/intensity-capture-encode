/*
 * segment_yy.h
 *
 *  Created on: Oct 16, 2012
 *      Author: chris
 */

#ifndef SEGMENT_YY_H_
#define SEGMENT_YY_H_

#include "input_handle.h"
#include "output_handle.h"

#define 	MAX_INPUT_NUM	20

/*	the program execute mode */
#define 	YY_TRANSCODE    0
#define		YY_VOD 		    1
#define 	YY_LIVE			2

typedef struct Segment_U{

	/*input file for segment info*/
//	int input_nb;						//input file number
//	char *input_file[MAX_INPUT_NUM];					//one or more file

	/*output for segment info */
	int mode_type;						//YY_TRANSCODE ,YY_VOD ,YY_LIVE
	char *storage_dir;					//directory storage the ts file and m3u8 file
	//ts
	//怎么把这几个变量放在output里面去就可以了。
	int dir_name_len;
	char ts_name[1024] ;						//ts name (rely on the mode_type )
	double segment_duration;					// segment target duration
	//m3u8
	char *m3u8_name;							//m3u8 name
	char full_m3u8_name[1024];
	char *ts_prfix_name;				//the prefix of the ts in the m3u8 file，if just transcode ,this is the ts file prefix

//	/*	input file decode	*/
//	Input_Context *input_ctx;

	/*	output file encode	*/
	Output_Context *output_ctx;

	/*	VOD	*/
	unsigned int segment_no;


	//user can control input
	int frame_rate;						 //frame rate
	int width;							//video width
	int height;							//video height
	int video_rate;						//video bitrate
	int audio_rate;						//audio bitrate
	int sample;							//audio sample
	int channel;						//audio channels

	//ts num
	int num_in_dir;						//ts number in the directory
	int num_in_m3u8;					//ts record in the m3u8 list

	AVFrame *picture_capture;
	unsigned int picture_capture_no;


}Segment_U;
/*
 * function	: init the argument of the struct Segment_U
 *
 * */
int init_seg_union(Segment_U * segment_union ,int prog_no);  //in here ,allocate memory ,and in the end there should be a function used to release memory



int seg_transcode_main(Segment_U * seg_union);

/*
 * function	: free the memory that allocate in init_seg_union function or others
 *
 * */
int free_seg_union(Segment_U * seg_union);


int seg_write_header(Segment_U * seg_union);

int seg_write_frame(Segment_U * seg_union ,int input_width ,int input_height ,int flag  ,void * yuv_data );
#endif /* SEGMENT_YY_H_ */
