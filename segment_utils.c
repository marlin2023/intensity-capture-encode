/*
 * segment_utils.c
 *
 *  Created on: Oct 16, 2012
 *      Author: chris
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>


#include "segment_utils.h"
#include "input_handle.h"
#include "output_handle.h"
#include "chris_global.h"
#include "chris_error.h"
#include "segment_yy.h"
int parse_option_argument(Segment_U ** seg_union_ptr ,int argc, char *argv[]) {

	Segment_U * seg_union = NULL;
	int i = 0;
	/*parse options*/
	int next_option;
	//short char option
	const char * const short_option = "vhm:d:t:p:n:r:w:e:v:a:s:c:02:3:";

	//long char option struction array
	const struct option long_option[] = {
			{ "version", 0, NULL, 'v' },
			{ "help",	0, NULL, 'h' },
			{ "mode", 1, NULL, 'm' }, //execute mode
			{ "dir", 1, NULL, 'd' }, //ts storage directory
			{ "segment_time", 1, NULL, 't' }, //segment duration
			{ "prefix_ts", 1, NULL, 'p' }, //the prefix in the m3u8 file
			{ "m3u8_name", 1, NULL, 'n' }, //m3u8 name
			{ "frame_rate", 1, NULL, 'r' }, //frame rate
			{ "width", 1, NULL, 'w' }, //video width
			{ "height", 1, NULL, 'e' }, //video height
			{ "vb", 1, NULL, 'o' }, //video bitrate
			{ "ab", 1, NULL, 'a' }, //audio bitrate
			{ "sample", 1, NULL, 's' }, //audio sample
			{ "channel", 1, NULL, 'c' }, //audio channels
			{ "decollator", 0, NULL, '0' },		//indicator a new bitrate
			{ "num_in_dir", 1, NULL, '2' },
			{ "num_in_m3u8", 1, NULL, '3' },
			{ NULL, 0, NULL, 0 }
	};

	do {

		next_option = getopt_long(argc, argv, short_option, long_option, NULL);

		switch (next_option) {

		case 'v': 	//version
			fprintf(stdout ,"**********************************\n");
			fprintf(stdout, "  segment version:%s \n\n", SEG_VERSION);
			exit(0);
		case 'h': //help
			fprintf(stderr,
					"\n**********************************\n"
					"--version			view the software version,with no argument\n"
					"--input			assign the input file ,at least one input file,at most 20 input files\n"
					"--mode 			you must assign the program work mode ,you can use 0:transcode ,1:vod ,2:live\n"
					"--dir				the directory where ts and m3u8 file storage\n"
					"--segment_time		the segment duration time in second\n"
					"--prefix_ts		the prefix of the ts file in the m3u8 file \n"
					"--m3u8_name		name of the m3u8 file\n"
					"--frame_rate		video frames per second\n"
					"--width			width of the video \n"
					"--height		    height of the video \n"
					"--vb				the bitrate of the video \n"
					"--ab				the bitrate of the audio\n"
					"--sample			the samples of the audio\n"
					"--channel			audio channel number\n"
					"--decollator		indicator a new program\n"
					"--num_in_dir		completed ts-file number in the directory\n"
					"--num_in_m3u8		ts record saved in the m3u8 list\n"
					"./capture --decollator --mode 2 --dir ./test/426x240  --segment_time 5 --prefix_ts yyt --m3u8 playlist.m3u8 --frame_rate 25 --width 426 --height 240 --vb 400k --ab 64k --sample 44100 --channel 2 --num_in_m3u8 5 --num_in_dir 5"
					"\n");
			exit(0);
//			break;
		case '0':
			if(i != 0){
				if( seg_union->mode_type == -1	||
						seg_union->m3u8_name == NULL ||
						seg_union->segment_duration == 0 ||
						seg_union->storage_dir == NULL ||
						seg_union->ts_prfix_name == NULL ||
						seg_union->frame_rate == 0 	||
						seg_union->width == 0 ||
						seg_union->height == 0	||
						seg_union->video_rate == 0 ||
						seg_union->audio_rate == 0 ||
						seg_union->sample  == 0 ||
						seg_union->channel == 0	||
						seg_union->num_in_dir == 0 ||
						seg_union->num_in_m3u8 == 0){

					fprintf(stdout ,"  Segment Invalid argument   ,please use"
																" '%s  --help '  to find some information\n", argv[0]);
					exit(SEG_INVALID_ARGUMENT);
				}

			}
			seg_union_ptr[i] = (Segment_U *)malloc(sizeof(Segment_U));
			if(seg_union_ptr[i] == NULL){
				fprintf(stderr ,"seg_union malloc failed .\n");
				exit(1);
			}
			seg_union = seg_union_ptr[i++];
			break;

		case '2':  //num_in_dir
			seg_union->num_in_dir = atoi(optarg);
			break;
		case '3':  //num_in_m3u8
			seg_union->num_in_m3u8 = atoi(optarg);
			break;
		case 'm': //the program work mode
			seg_union->mode_type = atoi(optarg);
			break;
		case 'd': //ts storage directory
			seg_union->storage_dir = optarg;
			break;
		case 't': //segment duration
			seg_union->segment_duration = atof(optarg);
			break;
		case 'p': //the ts file in the m3u8 file
			seg_union->ts_prfix_name = optarg;
			break;
		case 'n': //the m3u8 file
			seg_union->m3u8_name = optarg;
			break;
		case 'r': //frame rate
			seg_union->frame_rate = atoi(optarg);
			break;
		case 'w': //video width
			seg_union->width = atoi(optarg);
			break;
		case 'e': //video height
			seg_union->height = atoi(optarg);
			break;
		case 'o': //video bitrate 蛋疼的短选项

			if(strchr(optarg ,'k') == NULL)
				seg_union->video_rate = atoi(optarg);
			else
				seg_union->video_rate = atoi(optarg) * 1000;

			break;
		case 'a': //audio bitrate
			if(strchr(optarg ,'k') == NULL)
				seg_union->audio_rate = atoi(optarg);
			else
				seg_union->audio_rate = atoi(optarg) * 1000;

			break;
		case 's': //audio sample
			seg_union->sample = atoi(optarg);
			break;
		case 'c': //audio channels
			seg_union->channel = atoi(optarg);
			break;
		case '?':		//invalid options
			exit(0);
//			break;
		default:		//there is no options
			break;

		}
	} while (next_option != -1);


	/*	check the input argument valid or not*/
	if( seg_union->mode_type == -1	||
			seg_union->m3u8_name == NULL ||
			seg_union->segment_duration == 0 ||
			seg_union->storage_dir == NULL ||
			seg_union->ts_prfix_name == NULL ||
			seg_union->frame_rate == 0 	||
			seg_union->width == 0 ||
			seg_union->height == 0	||
			seg_union->video_rate == 0 ||
			seg_union->audio_rate == 0 ||
			seg_union->sample  == 0 ||
			seg_union->channel == 0	||
			seg_union->num_in_dir == 0 ||
			seg_union->num_in_m3u8 == 0){

		fprintf(stdout ,"  Segment Invalid argument   ,please use"
													" '%s  --help '  to find some information\n", argv[0]);
		exit(SEG_INVALID_ARGUMENT);
	}

	return i;
}

void create_directory(char *storage_dir) {

	char *temp = strdup(storage_dir);
	char *pos = temp;

    /* remove the start './' or '/' */
	if (strncmp(temp, "/", 1) == 0) {
		pos += 1;
	} else if (strncmp(temp, "./", 2) == 0) {
		pos += 2;
	}

	/* create directory  cyclic*/
	for (; *pos != '\0'; ++pos) {
		if (*pos == '/') {
			*pos = '\0';
			mkdir(temp, 0777);
			chris_printf("for %s\n", temp);
			*pos = '/';
		}
	}

	  /* if the last directory not end with '/' ,still create a directory*/
	if (*(pos - 1) != '/') {
		chris_printf("if %s\n", temp);
		mkdir(temp, 0777);
	}
	free(temp);


}

void create_m3u8_name(Segment_U * seg_union ){
	//add storage dir in the ts_name
	if( seg_union->storage_dir[strlen(seg_union->storage_dir) -1 ] == '/'){ //storage end with '/'

		sprintf(seg_union->full_m3u8_name ,"%s" ,seg_union->storage_dir);
	}else{
		sprintf(seg_union->full_m3u8_name ,"%s/" ,seg_union->storage_dir);
	}

	sprintf(&(seg_union->full_m3u8_name[strlen(seg_union->full_m3u8_name)]) ,"%s" ,seg_union->m3u8_name);

}

void create_first_ts_name(Segment_U * seg_union ,int mode_type){

	//add storage dir in the ts_name
	if( seg_union->storage_dir[strlen(seg_union->storage_dir) -1 ] == '/'){ //storage end with '/'

		sprintf(seg_union->ts_name ,"%s" ,seg_union->storage_dir);
	}else{
		sprintf(seg_union->ts_name ,"%s/" ,seg_union->storage_dir);
	}

	seg_union->dir_name_len = strlen(seg_union->ts_name);			//alter the dir_name_len
	//structure  the first ts name
	if(mode_type == YY_TRANSCODE){

		sprintf(&(seg_union->ts_name[strlen(seg_union->ts_name)]) ,"%s.ts" ,seg_union->ts_prfix_name);
	}else if(mode_type == YY_VOD){

		sprintf(&(seg_union->ts_name[strlen(seg_union->ts_name)]) ,"%s-%d.ts" ,seg_union->ts_prfix_name ,
				/*++seg_union->segment_no*/++seg_union->output_ctx->segment_no);
	}else if (mode_type == YY_LIVE){

		sprintf(&(seg_union->ts_name[strlen(seg_union->ts_name)]) ,"%s-%d.ts" ,seg_union->ts_prfix_name ,
				/*++seg_union->segment_no*/++seg_union->output_ctx->segment_no);
	}


}

int find_log_file(Segment_U * seg_union){

	if( seg_union->storage_dir[strlen(seg_union->storage_dir) -1 ] == '/'){ //storage end with '/'

		sprintf(seg_union->output_ctx->log_name ,"%s%s" ,seg_union->storage_dir ,LOG_FILE_NAME);
	}else{
		sprintf(seg_union->output_ctx->log_name ,"%s/%s" ,seg_union->storage_dir ,LOG_FILE_NAME);
	}

	//access the log file
	if(access (seg_union->output_ctx->log_name ,F_OK|R_OK|W_OK) == 0)
		return 1;
	else
		return 0;

}

void recover_from_log(Segment_U * seg_union){

	char tmp_buf[32];
	FILE *ptr_log = NULL;
	ptr_log = fopen(seg_union->output_ctx->log_name ,"r");  //open in only read mode
	if(ptr_log == NULL){
		fprintf(stderr ,"open log_file failed ,FILE:%s ,LINE:%d\n" ,__FILE__ ,__LINE__);
		exit(OPEN_LOG_FILE_FAIL);
	}

	//first ,judge the length of the file log.err
	fseek(ptr_log, 0, SEEK_END);
	int lengthOfFile = ftell(ptr_log);
	if (lengthOfFile == 0) {
		//printf("length is 0 \n");
		fclose(ptr_log);
		return ;
	}

	fseek(ptr_log, 0, SEEK_SET);

	/*
	 * read log file
	 * */
	if (fgets(tmp_buf, 32, ptr_log) == NULL) { //fgets read a whole line data
		fprintf(stderr, "read log_file failed\n");
		exit(READ_LOG_FILE_FAIL);
	}

	seg_union->segment_no =
	seg_union->output_ctx->segment_no = atoi(tmp_buf);	//last work ,the max segment index

	int i;

	if(seg_union->output_ctx->segment_no > seg_union->output_ctx->num_in_m3u8){
		for(i = 0 ;i <=  seg_union->output_ctx->num_in_m3u8 ;i ++){
			if(fgets(tmp_buf ,32 ,ptr_log) == NULL){  //fgets read a whole line data
				fprintf(stderr ,"read log_file failed\n");
				exit(READ_LOG_FILE_FAIL);
			}

			seg_union->output_ctx->seg_duration_arr[i] = atof(tmp_buf);
		}
	}else{

		for(i = 0 ;i <= seg_union->output_ctx->segment_no  ;i ++){
			if(fgets(tmp_buf ,32 ,ptr_log) == NULL){  //fgets read a whole line data
				fprintf(stderr ,"read log_file failed\n");
				exit(READ_LOG_FILE_FAIL);
			}

			seg_union->output_ctx->seg_duration_arr[i] = atof(tmp_buf);
		}
	}

	fclose(ptr_log);
}

void update_log_file(Output_Context *ptr_output_ctx){

	char tmp_buf[1024];
	int  buf_index;
	FILE *ptr_log = NULL;

	ptr_log = fopen(ptr_output_ctx->log_name ,"w");
	if(ptr_log == NULL){
		fprintf(stderr ,"open log_file failed ,FILE:%s ,LINE:%d\n" ,__FILE__ ,__LINE__);
		exit(OPEN_LOG_FILE_FAIL);
	}

	//write the segment no
	snprintf(tmp_buf, 1024, "%u\n", ptr_output_ctx->segment_no);
	buf_index = strlen(tmp_buf);

	//write the target segment duration
	snprintf(tmp_buf + buf_index, 32, "%f\n", ptr_output_ctx->segment_duration);
	buf_index = strlen(tmp_buf);

	//write the segment time
	int i ;
	if(ptr_output_ctx->segment_no > ptr_output_ctx->num_in_m3u8){
		for(i = 1; i <= ptr_output_ctx->num_in_m3u8 ; i++ ){
			snprintf(tmp_buf + buf_index, 32, "%f\n", ptr_output_ctx->seg_duration_arr[i]);
			buf_index = strlen(tmp_buf);
		}
	}else{
		for(i = 1; i <= ptr_output_ctx->segment_no ; i++){
			snprintf(tmp_buf + buf_index, 32, "%f\n", ptr_output_ctx->seg_duration_arr[i]);
			buf_index = strlen(tmp_buf);
		}
	}
	if (fwrite(tmp_buf, strlen(tmp_buf), 1, ptr_log) != 1) {
		fprintf(stderr, "Could not write to m3u8 index file, will not continue writing to index file\n");
		fclose(ptr_log);
		exit(WRITE_LOG_FILE_FAIL);
	}

	fclose(ptr_log);
}

//implication the compress the video frame to jpeg
//libjpeg相关的头文件
#include "jpeglib.h"
static void draw_jpeg(AVPicture *pic, int width, int height, char * jpeg_name) {
// AVPicture my_pic ;
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	JSAMPROW row_pointer[1];
	int row_stride;
	uint8_t *buffer;
	FILE *fp;

	//vfmt2rgb(my_pic,pic) ;
	buffer = pic->data[0];
	fp = fopen(jpeg_name, "wb");
	if (fp == NULL) {
		av_log(NULL, AV_LOG_ERROR, "fopen %s error/n", jpeg_name);
		return;
	}
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);
	jpeg_stdio_dest(&cinfo, fp);
	cinfo.image_width = width;
	cinfo.image_height = height;
	cinfo.input_components = 3;
	cinfo.in_color_space = JCS_RGB;
	jpeg_set_defaults(&cinfo);

	jpeg_set_quality(&cinfo, 80, 1 /*true*/);

	jpeg_start_compress(&cinfo, 1 /*TRUE*/);
	row_stride = width * 3;
	while (cinfo.next_scanline < height) {
		row_pointer[0] = &buffer[cinfo.next_scanline * row_stride];
		(void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
	}
	jpeg_finish_compress(&cinfo);
	fclose(fp);
	jpeg_destroy_compress(&cinfo);
	chris_printf("compress frame finished!==========================================>>>>>>>>>>>>>>>>>>>>>>>>>./n");
	return;
}

void record_segment_time(Output_Context *ptr_output_ctx){

	if(ptr_output_ctx->start_time_mark == 0){
		ptr_output_ctx->start_time_mark = 1;
		ptr_output_ctx->prev_segment_time = av_q2d(ptr_output_ctx->video_stream->time_base) *
													(ptr_output_ctx->pkt.pts )
													- (double)ptr_output_ctx->ptr_format_ctx->start_time / AV_TIME_BASE;

		chris_printf("ptr_output_ctx->prev_segment_time = %f \n" ,ptr_output_ctx->prev_segment_time);

	}

	ptr_output_ctx->curr_segment_time =
						av_q2d(ptr_output_ctx->video_stream->time_base) *
										(ptr_output_ctx->pkt.pts )
										- (double)ptr_output_ctx->ptr_format_ctx->start_time / AV_TIME_BASE;

//	//time meet
	if(ptr_output_ctx->curr_segment_time - ptr_output_ctx->prev_segment_time >= ptr_output_ctx->segment_duration){
		chris_printf("...meet time .. ,duration = %f ,start_time = %f .\n" ,ptr_output_ctx->curr_segment_time - ptr_output_ctx->prev_segment_time ,
				(double)ptr_output_ctx->ptr_format_ctx->start_time / AV_TIME_BASE);
//		//get lock
//		pthread_mutex_lock(&ptr_output_ctx->output_mutex);
		avio_flush(ptr_output_ctx->ptr_format_ctx->pb);
		avio_close(ptr_output_ctx->ptr_format_ctx->pb);

		chris_printf("complete the %d.ts ,and write the m3u8 file..\n" ,ptr_output_ctx->segment_no);

//zhangyanlong
#if 1
		flock(STDOUT_FILENO ,LOCK_EX);
		fprintf(stdout ,"{\"ret\":%s ,\"seg\":%u,\"duration\":%.02f}\n" ,ptr_output_ctx->m3u8 ,ptr_output_ctx->segment_no ,
				ptr_output_ctx->curr_segment_time - ptr_output_ctx->prev_segment_time);
		flock(STDOUT_FILENO ,LOCK_UN);
#endif
		write_m3u8_body( ptr_output_ctx ,ptr_output_ctx->curr_segment_time - ptr_output_ctx->prev_segment_time);
		//create next ts file name
		sprintf(&(ptr_output_ctx->ts_name[ptr_output_ctx->dir_name_len]) ,"%s-%d.ts" ,ptr_output_ctx->ts_prfix_name ,++ptr_output_ctx->segment_no);
		if (avio_open(&(ptr_output_ctx->ptr_format_ctx->pb), ptr_output_ctx->ts_name, AVIO_FLAG_WRITE) < 0) {
			fprintf(stderr, "Could not open '%s'\n", ptr_output_ctx->ts_name);
			exit(OPEN_MUX_FILE_FAIL);
		}
//		//unlock
//		pthread_mutex_unlock(&ptr_output_ctx->output_mutex);
		ptr_output_ctx->prev_segment_time = ptr_output_ctx->curr_segment_time;   //place here
	}

}


void write_m3u8_header(Output_Context *ptr_output_ctx){
	//vod
	if(ptr_output_ctx->mode_type  == YY_VOD){
		ptr_output_ctx->fp_m3u8 = fopen(ptr_output_ctx->full_m3u8_name, "wb+");

		if(ptr_output_ctx->fp_m3u8 == NULL){
			fprintf(stderr ,"Could not open m3u8 file %s...\n" ,ptr_output_ctx->full_m3u8_name);
			exit(OPEN_M3U8_FAIL);
		}

		//write header
		fprintf(ptr_output_ctx->fp_m3u8 ,"#EXTM3U\n#EXT-X-TARGETDURATION:%.02f\n#EXT-X-MEDIA-SEQUENCE:1\n" ,ptr_output_ctx->segment_duration);

		fclose(ptr_output_ctx->fp_m3u8);
	}
}


/*	round function	*/
static int segmenter_duration_round(double src_num){

	int tmp_num = (int)src_num;  //acquire the integer of the double data

	double src_num1 = src_num + 0.5;
	int target_num = (int)src_num1;
	if(target_num > tmp_num){
		return target_num;
	}else{
		return tmp_num;
	}

}

void write_m3u8_body(Output_Context *ptr_output_ctx ,double segment_duration){

	chris_printf("=====segment_duration = %f \n" ,segment_duration);
//	while(1);

	if (ptr_output_ctx->mode_type == YY_LIVE) { //live

		ptr_output_ctx->fp_m3u8 = fopen(ptr_output_ctx->full_m3u8_name, "wb+");

		if (ptr_output_ctx->fp_m3u8 == NULL) {
			fprintf(stderr, "Could not open m3u8 file %s...\n",
					ptr_output_ctx->full_m3u8_name);
			exit(OPEN_M3U8_FAIL);
		}

		int live_buf_index;

		/*	header in m3u8	*/
		if (ptr_output_ctx->segment_no <= ptr_output_ctx->num_in_m3u8) { //amite-1.ts

			snprintf(ptr_output_ctx->live_write_buf, 1024,
					"#EXTM3U\n#EXT-X-VERSION:3\n#EXT-X-ALLOW-CACHE:NO\n#EXT-X-TARGETDURATION:%d\n#EXT-X-MEDIA-SEQUENCE:1\n",
					segmenter_duration_round(ptr_output_ctx->segment_duration));

		} else {		// the first ts file have been removed
			snprintf(ptr_output_ctx->live_write_buf, 1024,
					"#EXTM3U\n#EXT-X-VERSION:3\n#EXT-X-ALLOW-CACHE:NO\n#EXT-X-TARGETDURATION:%d\n#EXT-X-MEDIA-SEQUENCE:%d\n",
					segmenter_duration_round((int) ptr_output_ctx->segment_duration),
					ptr_output_ctx->segment_no - ptr_output_ctx->num_in_m3u8
							+ 1);
		}
		live_buf_index = strlen(ptr_output_ctx->live_write_buf);

		int j;
		/*	context in m3u8	*/
		//situation one
		if (ptr_output_ctx->segment_no <= ptr_output_ctx->num_in_m3u8) {

			ptr_output_ctx->seg_duration_arr[ptr_output_ctx->segment_no] =
					segment_duration;

			for (j = 1; j <= ptr_output_ctx->segment_no; j++) {

				if (bool_interger)
					snprintf(ptr_output_ctx->live_write_buf + live_buf_index,
							1024, "#EXTINF:%d,\n%s-%u.ts\n",
							segmenter_duration_round(
									ptr_output_ctx->seg_duration_arr[j]), ptr_output_ctx->ts_prfix_name,
							j);
				else
					snprintf(ptr_output_ctx->live_write_buf + live_buf_index,
							1024, "#EXTINF:%.02f,\n%s-%u.ts\n",
							ptr_output_ctx->seg_duration_arr[j], ptr_output_ctx->ts_prfix_name,
							j);

				//update live_buf_index
				live_buf_index = strlen(ptr_output_ctx->live_write_buf);
			}

			if (fwrite(ptr_output_ctx->live_write_buf,
					strlen(ptr_output_ctx->live_write_buf), 1,
					ptr_output_ctx->fp_m3u8) != 1) {
				fprintf(stderr,
						"Could not write to m3u8 index file, will not continue writing to index file\n");
				fclose(ptr_output_ctx->fp_m3u8);
				exit(WRITE_M3U8_FAIL);
			}
			chris_printf("#chirs :success write the m3u8 file\n");
		} else { ////situation second
			/*先对数组中保存的ts的时间长度进行更新*/

//===========================
			for (j = 1; j < ptr_output_ctx->num_in_m3u8; j++) {

				ptr_output_ctx->seg_duration_arr[j] =
						ptr_output_ctx->seg_duration_arr[j + 1]; //元素前移

			}
			ptr_output_ctx->seg_duration_arr[ptr_output_ctx->num_in_m3u8] =
					segment_duration; //处理数组最后一个成员

			for (j = ptr_output_ctx->segment_no - ptr_output_ctx->num_in_m3u8
					+ 1; j <= ptr_output_ctx->segment_no; j++) { //last_segment - sf_segmenter_handle_union->m3u8_ts_num + 1 从这开始 没错

				if (bool_interger)
					snprintf(ptr_output_ctx->live_write_buf + live_buf_index,
							1024, "#EXTINF:%d,\n%s-%u.ts\n",
							segmenter_duration_round(
									ptr_output_ctx->seg_duration_arr[ptr_output_ctx->num_in_m3u8
											- (ptr_output_ctx->segment_no - j)]), ptr_output_ctx->ts_prfix_name,
							j);
				else
					snprintf(ptr_output_ctx->live_write_buf + live_buf_index,
							1024, "#EXTINF:%.02f,\n%s-%u.ts\n",
							ptr_output_ctx->seg_duration_arr[ptr_output_ctx->num_in_m3u8
									- (ptr_output_ctx->segment_no - j)], ptr_output_ctx->ts_prfix_name, j);

				//拼接
				//update live_buf_index
				live_buf_index = strlen(ptr_output_ctx->live_write_buf);
			}
///===============================
			if (fwrite(ptr_output_ctx->live_write_buf,
					strlen(ptr_output_ctx->live_write_buf), 1,
					ptr_output_ctx->fp_m3u8) != 1) {
				fprintf(stderr,
						"Could not write to m3u8 index file, will not continue writing to index file\n");
				fclose(ptr_output_ctx->fp_m3u8);
				exit(WRITE_M3U8_FAIL);
			}
			chris_printf("#chirs :success write the m3u8 file\n");
		}

		fclose(ptr_output_ctx->fp_m3u8);


		//delete ts file generate before (I must save num_in_dir completed ts files)
		if (ptr_output_ctx->segment_no
				> ptr_output_ctx->num_in_dir) {

			char remove_ts_filename[1024] = {0};
			chris_printf("ptr_output_ctx->ts_name = %s \n" ,ptr_output_ctx->ts_name);
			snprintf(remove_ts_filename ,ptr_output_ctx->dir_name_len + 1 ,"%s" ,ptr_output_ctx->ts_name );
			chris_printf("ptr_output_ctx->dir_name_len = %d ,no = %d \n" ,ptr_output_ctx->dir_name_len ,ptr_output_ctx->segment_no);
			chris_printf("-------------remove_ts_filename = %s\n" ,remove_ts_filename);
			snprintf(
					&(remove_ts_filename[ptr_output_ctx->dir_name_len]),
					1024,
					"%s-%u.ts",
					ptr_output_ctx->ts_prfix_name,
					ptr_output_ctx->segment_no - ptr_output_ctx->num_in_dir);
			chris_printf("-------------remove_ts_filename = %s\n" ,remove_ts_filename);
			//remove this file
			unlink(remove_ts_filename);
		}

		/*
		 * update the log file
		 *
		 * */
		update_log_file(ptr_output_ctx);


	} else if (ptr_output_ctx->mode_type == YY_VOD) { //vod

		ptr_output_ctx->fp_m3u8 = fopen(ptr_output_ctx->full_m3u8_name, "ab+");

		if (ptr_output_ctx->fp_m3u8 == NULL) {
			fprintf(stderr, "Could not open m3u8 file %s...\n",
					ptr_output_ctx->full_m3u8_name);
			exit(OPEN_M3U8_FAIL);
		}
		fprintf(ptr_output_ctx->fp_m3u8, "#EXTINF:%.02f,\n%s-%u.ts\n",
				segment_duration, ptr_output_ctx->ts_prfix_name,
				ptr_output_ctx->segment_no);

		fclose(ptr_output_ctx->fp_m3u8);
	}

}

void write_m3u8_tailer(Output_Context *ptr_output_ctx){

	//vod
	if(ptr_output_ctx->mode_type  == YY_VOD){

		double tailer_duration = ptr_output_ctx->sync_ipts - ptr_output_ctx->prev_segment_time;
		ptr_output_ctx->fp_m3u8 = fopen(ptr_output_ctx->full_m3u8_name, "ab+");

		if(ptr_output_ctx->fp_m3u8 == NULL){
			fprintf(stderr ,"Could not open m3u8 file %s...\n" ,ptr_output_ctx->full_m3u8_name);
			exit(OPEN_M3U8_FAIL);
		}
		fprintf(ptr_output_ctx->fp_m3u8 ,"#EXTINF:%.02f,\n%s-%u.ts\n#EXT-X-ENDLIST\n" ,tailer_duration ,ptr_output_ctx->ts_prfix_name ,
									ptr_output_ctx->segment_no);

		fclose(ptr_output_ctx->fp_m3u8);
	}
}
