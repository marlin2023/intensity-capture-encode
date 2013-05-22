/*
 * segment_yy.c
 *
 *  Created on: Oct 16, 2012
 *      Author: chris
 */
#include <stdio.h>

#include "libswscale/swscale.h"
#include "segment_yy.h"
#include "segment_utils.h"
#include "chris_error.h"
#include "chris_global.h"
#include "output_handle.h"

#include "Capture_global.h"

int init_seg_union(Segment_U * segment_union ,int prog_no) {   //传递是指针变量的地址


	Segment_U * seg_union =segment_union;

	seg_union->segment_no = 0;
	seg_union->picture_capture = avcodec_alloc_frame();
	seg_union->picture_capture_no = 0;

	/*	-----------base seg_union do something---------------	*/
	//	create directory
	create_directory(seg_union->storage_dir);

	// create m3u8 file
	create_m3u8_name(seg_union);
	printf("seg_union->full_m3u8_name = %s \n" ,seg_union->full_m3u8_name);

	/*----------following ,start to set  Output_Context information----- */
	//malloc Output_context
	if( (seg_union->output_ctx = malloc (sizeof(Output_Context))) == NULL){

		printf("ptr_output_ctx malloc failed .\n");
		exit(MEMORY_MALLOC_FAIL);
	}
	//segment element in output context
	seg_union->output_ctx->frame_rate				=		 seg_union->frame_rate;						 //frame rate
	seg_union->output_ctx->width					=		 seg_union->width;							//video width
	seg_union->output_ctx->height					=		 seg_union->height;							//video height
	seg_union->output_ctx->video_rate				=		 seg_union->video_rate;						//video bitrate
	seg_union->output_ctx->audio_rate				=		 seg_union->audio_rate;						//audio bitrate
	seg_union->output_ctx->sample					=		 seg_union->sample;							//audio sample
	seg_union->output_ctx->channel					=		 seg_union->channel;						//audio channels
	seg_union->output_ctx->num_in_dir				= seg_union->num_in_dir;
	seg_union->output_ctx->num_in_m3u8 				= seg_union->num_in_m3u8;
	seg_union->output_ctx->segment_duration 		=		 seg_union->segment_duration;
	seg_union->output_ctx->ts_prfix_name 			= 		 seg_union->ts_prfix_name;
	seg_union->output_ctx->mode_type				=		 seg_union->mode_type;
	//
	seg_union->output_ctx->segment_no				= 		 seg_union->segment_no;
	seg_union->output_ctx->full_m3u8_name			=		 seg_union->full_m3u8_name;
	//
	seg_union->output_ctx->frame_count 				= 0;			//this frame_count be used to generate video pts.

	printf("num_in_m3u8 = %d ,num_in_dir = %d \n\n" ,seg_union->output_ctx->num_in_m3u8 ,seg_union->output_ctx->num_in_dir);

	/*-----------	following ,only do in the mode_type yy_live	--------------*/
	if(seg_union->mode_type == YY_LIVE){	// live mode

		/*	live something	*/
		//malloc live_buffer for m3u8 buffer  //the content of m3u8
		seg_union->output_ctx->live_write_buf = malloc(sizeof(char) * 1024 * 100);
		if (!seg_union->output_ctx->live_write_buf) {
			fprintf(stderr, "Could not allocate write buffer for live_write_buf\n");
			exit(ALLOCATE_M3U8_WRITE_BUFFER_LIVE);
		}
		memset(seg_union->output_ctx->live_write_buf ,0  ,1024*100);

		printf("ptr_output_ctx->num_in_m3u8 = %d \n" ,seg_union->output_ctx->num_in_m3u8);
		seg_union->output_ctx->seg_duration_arr = malloc(sizeof(double) *  ( seg_union->output_ctx->num_in_m3u8 + 1) );  	//use to storage the every ts file length in the m3u8 ,the array[0] reserved
		if(seg_union->output_ctx->seg_duration_arr == NULL){
			printf("seg_duration_arr malloc failed .. ,%s ,line %d \n" ,__FILE__ ,__LINE__);
			exit(MEMORY_MALLOC_FAIL);
		}

		/*	judge for the log.info 	*/
		int ret =find_log_file(seg_union);
		if(ret){//find the log file ,and recover the scene ,recover the segment_no use to create the ts name
			recover_from_log(seg_union);
			printf("......after recover from the log file ...\n\n\n");
		}
	}

	//splice the first ts name
	create_first_ts_name(seg_union ,seg_union->mode_type);

	//the followint ,dir_name_len and ts_name set must be set after the function create_first_ts_name!!!!
	seg_union->output_ctx->dir_name_len 		    = 		 seg_union->dir_name_len;   //
	seg_union->output_ctx->ts_name 					=		 seg_union->ts_name;		//
	printf("--------------->before transcode init function  ,seg_union->ts_name = %s..\n" ,seg_union->ts_name);

	//add audio stream and video into the output_ctx ,of course ,set codec information also in the function init_output
//	init_output(seg_union->output_ctx ,seg_union->ts_name );  //add stream information in this function
	init_output(seg_union->output_ctx ,seg_union->ts_name , prog_no);  //add stream information in this function

	//open video and audio codecs ,set video_out_buf and audio_out_buf
	open_stream_codec(seg_union->output_ctx ,prog_no);
	printf("--------------->after transcode init function ..\n");


	return 0;
}

int seg_write_header(Segment_U * seg_union){
	Output_Context *ptr_output_ctx = seg_union->output_ctx;

    if (!(ptr_output_ctx->fmt->flags & AVFMT_NOFILE)) {		//for mp4 or mpegts ,this must be performed
        if (avio_open(&(ptr_output_ctx->ptr_format_ctx->pb), seg_union->ts_name, AVIO_FLAG_WRITE) < 0) {
            fprintf(stderr, "Could not open '%s'\n", seg_union->ts_name);
            exit(OPEN_MUX_FILE_FAIL);
        }
    }
    // write the stream header, if any
    avformat_write_header(ptr_output_ctx->ptr_format_ctx ,NULL);
    write_m3u8_header(ptr_output_ctx);

	return 0;
}

int seg_write_frame(Segment_U * seg_union ,int input_width ,int input_height ,int flag  ,void * yuv_data ){

	Output_Context *ptr_output_ctx = seg_union->output_ctx;



	avpicture_fill((AVPicture *)seg_union->picture_capture ,(uint8_t*)yuv_data , PIX_FMT_UYVY422 ,input_width ,input_height );
		//encode video
	//input stream 的问题。
	ptr_output_ctx->sync_ipts = (double) seg_union->picture_capture_no / CAPTURE_FRAME_RATE ; //converter in seconds


	//first swscale
	sws_scale(ptr_output_ctx->img_convert_ctx,
			(const uint8_t* const *) seg_union->picture_capture->data,
			seg_union->picture_capture->linesize, 0,
			input_height,
			ptr_output_ctx->encoded_yuv_pict->data,
			ptr_output_ctx->encoded_yuv_pict->linesize);

	//second swscale
	encode_video_frame(ptr_output_ctx, ptr_output_ctx->encoded_yuv_pict,
			NULL );


	return 0;
}

int seg_write_tailer(Segment_U * seg_union){

	Output_Context *ptr_output_ctx = seg_union->output_ctx;

	printf("before flush ,ptr_output_ctx->ptr_format_ctx->nb_streams = %d \n\n" ,ptr_output_ctx->ptr_format_ctx->nb_streams);
	encode_flush(ptr_output_ctx ,ptr_output_ctx->ptr_format_ctx->nb_streams);

	write_m3u8_tailer(ptr_output_ctx);
	printf("before wirite tailer ...\n\n");
	av_write_trailer(ptr_output_ctx->ptr_format_ctx );
	return 0;
}

#if 0
int seg_transcode_main(Segment_U * seg_union){

	Output_Context *ptr_output_ctx = seg_union->output_ctx;

    if (!(ptr_output_ctx->fmt->flags & AVFMT_NOFILE)) {		//for mp4 or mpegts ,this must be performed
        if (avio_open(&(ptr_output_ctx->ptr_format_ctx->pb), seg_union->ts_name, AVIO_FLAG_WRITE) < 0) {
            fprintf(stderr, "Could not open '%s'\n", seg_union->ts_name);
            exit(OPEN_MUX_FILE_FAIL);
        }
    }
    // write the stream header, if any
    avformat_write_header(ptr_output_ctx->ptr_format_ctx ,NULL);
    write_m3u8_header(ptr_output_ctx);
    int i ;
    for(i = 0 ; i < seg_union->input_nb ; i ++){

    	/*initialize input file information*/
    	init_input( seg_union->input_ctx ,seg_union->input_file[i]);
    	Input_Context *ptr_input_ctx = seg_union->input_ctx;

		ptr_output_ctx->img_convert_ctx = sws_getContext(
				ptr_input_ctx->video_codec_ctx->width ,ptr_input_ctx->video_codec_ctx->height ,PIX_FMT_YUV420P,
				 ptr_output_ctx->video_stream->codec->width ,ptr_output_ctx->video_stream->codec->height ,PIX_FMT_YUV420P ,
				 SWS_BICUBIC ,NULL ,NULL ,NULL);


		printf("src_width = %d ,src_height = %d \n" ,ptr_input_ctx->video_codec_ctx->width ,ptr_input_ctx->video_codec_ctx->height);
		printf("dts_width = %d ,dts_height = %d \n" ,ptr_output_ctx->video_stream->codec->width ,
				ptr_output_ctx->video_stream->codec->height);

		printf("before av_read_frame ...\n");
		/*************************************************************************************/
		/*decoder loop*/
		//
		//
		//***********************************************************************************/
		while(av_read_frame(ptr_input_ctx->ptr_format_ctx ,&ptr_input_ctx->pkt) >= 0){

			if (ptr_input_ctx->pkt.stream_index == ptr_input_ctx->video_index) {

				//decode video packet
				int got_picture = 0;
				avcodec_decode_video2(ptr_input_ctx->video_codec_ctx,
						ptr_input_ctx->yuv_frame, &got_picture, &ptr_input_ctx->pkt);

				if (got_picture) {
					//encode video
					//input stream 的问题。
					ptr_output_ctx->sync_ipts = av_q2d(ptr_input_ctx->ptr_format_ctx->streams[ptr_input_ctx->video_index]->time_base) *
							(ptr_input_ctx->yuv_frame->best_effort_timestamp  )
							- (double)ptr_input_ctx->ptr_format_ctx->start_time / AV_TIME_BASE
							+	ptr_output_ctx->base_ipts;    //current packet time in second

					//printf("ptr_output_ctx->sync_ipts = %f \n" ,ptr_output_ctx->sync_ipts);

					//first swscale
					sws_scale(ptr_output_ctx->img_convert_ctx ,
							(const uint8_t* const*)ptr_input_ctx->yuv_frame->data ,ptr_input_ctx->yuv_frame->linesize ,
							0 ,
							ptr_input_ctx->video_codec_ctx->height ,
							ptr_output_ctx->encoded_yuv_pict->data ,ptr_output_ctx->encoded_yuv_pict->linesize);

					//second swscale
					encode_video_frame(ptr_output_ctx , ptr_output_ctx->encoded_yuv_pict ,ptr_input_ctx );
				}

			} else if (ptr_input_ctx->pkt.stream_index == ptr_input_ctx->audio_index) {
				//printf("audio ...\n");
				//decode audio packet
				uint8_t *tmp_data = ptr_input_ctx->pkt.data;
				int tmp_size = ptr_input_ctx->pkt.size;
				while (ptr_input_ctx->pkt.size > 0) {
					int got_frame = 0;
					int len = avcodec_decode_audio4(ptr_input_ctx->audio_codec_ctx,
							ptr_input_ctx->audio_decode_frame, &got_frame,
							&ptr_input_ctx->pkt);

					if (len < 0) { //decode failed ,skip frame
						fprintf(stderr, "Error while decoding audio frame\n");
						break;
					}

					if (got_frame) {
						//encode the audio data ,and write the data into the output
//						do_audio_out(ptr_output_ctx ,ptr_input_ctx ,ptr_input_ctx->audio_decode_frame);
					} else { //no data
						printf("======>avcodec_decode_audio4 ,no data ..\n");
						continue;
					}

					ptr_input_ctx->pkt.size -= len;
					ptr_input_ctx->pkt.data += len;

				}

				//renew
				ptr_input_ctx->pkt.size = tmp_size;
				ptr_input_ctx->pkt.data = tmp_data;

			}

			if(&ptr_input_ctx->pkt)
				av_free_packet(&ptr_input_ctx->pkt);

		}//endwhile

		double file_duration = ptr_input_ctx->ptr_format_ctx->duration / AV_TIME_BASE
					+ (double)( ptr_input_ctx->ptr_format_ctx->duration % AV_TIME_BASE ) / AV_TIME_BASE;


		ptr_output_ctx->base_ipts  += file_duration;  //completed files sum time duration
		printf("end while ......,time_base = %f .............> \n" ,ptr_output_ctx->base_ipts  );
		ptr_output_ctx->audio_resample = 0;
		sws_freeContext(ptr_output_ctx->img_convert_ctx);
		free_input(ptr_input_ctx);
    } //end for

	printf("before flush ,ptr_output_ctx->ptr_format_ctx->nb_streams = %d \n\n" ,ptr_output_ctx->ptr_format_ctx->nb_streams);
	encode_flush(ptr_output_ctx ,ptr_output_ctx->ptr_format_ctx->nb_streams);

	write_m3u8_tailer(ptr_output_ctx);
	printf("before wirite tailer ...\n\n");
	av_write_trailer(ptr_output_ctx->ptr_format_ctx );

	return 0;
}

#endif


int free_seg_union(Segment_U * seg_union){
	printf("start free segment union ...\n");

	//free output context relevance
	free_output_memory(seg_union->output_ctx);
	if(seg_union->output_ctx)
		free(seg_union->output_ctx);
	//at last
	if(seg_union)
		free(seg_union);
	return 0;
}
