/*
 * input_handle.c
 *
 *  Created on: Sep 24, 2012
 *      Author: chris
 */
#include <stdio.h>
#include <stdlib.h>

#include "input_handle.h"
#include "chris_error.h"
//ffmpeg header
#include <libavutil/avutil.h>

int init_input(Input_Context *ptr_input_ctx, char* input_file) {

	//open input file
	ptr_input_ctx->ptr_format_ctx = NULL;
	if( avformat_open_input(&ptr_input_ctx->ptr_format_ctx, input_file, NULL, NULL) != 0){
		printf("inputfile init ,avformat_open_input failed .\n");
		exit(AV_OPEN_INPUT_FAIL);
	}

	//find stream info
	if ( avformat_find_stream_info(ptr_input_ctx->ptr_format_ctx, NULL) < 0){
		printf("inputfile init ,avformat_find_stream_info failed .\n");
		exit(AV_FIND_STREAM_INFO_FAIL);
	}

	//find streams in the input file
	ptr_input_ctx->audio_index = -1;
	ptr_input_ctx->video_index = -1;
	int i;
	for (i = 0; i < ptr_input_ctx->ptr_format_ctx->nb_streams; i++) {

		//the first video stream
		if (ptr_input_ctx->video_index < 0
				&& ptr_input_ctx->ptr_format_ctx->streams[i]->codec->codec_type
						== AVMEDIA_TYPE_VIDEO) {
			ptr_input_ctx->video_index = i;

		}

		//the first audio stream
		if (ptr_input_ctx->audio_index < 0
				&& ptr_input_ctx->ptr_format_ctx->streams[i]->codec->codec_type
						== AVMEDIA_TYPE_AUDIO) {
			ptr_input_ctx->audio_index = i;

		}
	}

	printf("audio_index = %d ,video_index = %d \n" ,ptr_input_ctx->audio_index ,
			ptr_input_ctx->video_index);

	if(ptr_input_ctx->video_index < 0 ){

		printf("do not find video stream ..\n");
		exit(NO_VIDEO_STREAM);
	}

	if(ptr_input_ctx->audio_index < 0 ){

		printf("do not find audio stream ..\n");
		exit(NO_AUDIO_STREAM);
	}

	av_dump_format(ptr_input_ctx->ptr_format_ctx ,0 ,input_file ,0);
	//open video codec
	ptr_input_ctx->video_codec_ctx = ptr_input_ctx->ptr_format_ctx->streams[ptr_input_ctx->video_index]->codec;
	ptr_input_ctx->video_codec = avcodec_find_decoder(ptr_input_ctx->video_codec_ctx->codec_id);
	if(ptr_input_ctx->video_codec == NULL ){

		printf("in inputfile init ,unsupported video codec ..\n");
		exit(UNSPPORT_VIDEO_CODEC);
	}

	if(avcodec_open2(ptr_input_ctx->video_codec_ctx ,ptr_input_ctx->video_codec ,NULL) < 0){

		printf("in inputfile init ,can not open video_codec_ctx ..\n");
		exit(OPEN_VIDEO_CODEC_FAIL);
	}

	//open audio codec
	ptr_input_ctx->audio_codec_ctx = ptr_input_ctx->ptr_format_ctx->streams[ptr_input_ctx->audio_index]->codec;
	ptr_input_ctx->audio_codec = avcodec_find_decoder(ptr_input_ctx->audio_codec_ctx->codec_id);
	if(ptr_input_ctx->audio_codec == NULL ){

		printf("in inputfile init ,unsupported audio codec ..\n");
		exit(UNSPPORT_AUDIO_CODEC);
	}

	if(avcodec_open2(ptr_input_ctx->audio_codec_ctx ,ptr_input_ctx->audio_codec ,NULL) < 0){

		printf("in inputfile init ,can not open audio_codec_ctx ..\n");
		exit(OPEN_AUDIO_CODEC_FAIL);
	}

	printf("in here ,have open video codec ,and audio codec .\n");

	return 0;

}

void free_input(Input_Context *ptr_input_ctx){

	avformat_close_input(&ptr_input_ctx->ptr_format_ctx);

	avcodec_close(ptr_input_ctx->video_codec_ctx);

	avcodec_close(ptr_input_ctx->audio_codec_ctx);

}


void malloc_input_memory(Input_Context *ptr_input_ctx){
	/*	malloc memory 	*/
	ptr_input_ctx->yuv_frame = avcodec_alloc_frame();
	if(ptr_input_ctx->yuv_frame == NULL){
		printf("yuv_frame allocate failed %s ,%d line\n" ,__FILE__ ,__LINE__);
		exit(MEMORY_MALLOC_FAIL);
	}

	//audio frame
	ptr_input_ctx->audio_decode_frame = avcodec_alloc_frame();
	if(ptr_input_ctx->audio_decode_frame == NULL){
		printf("audio_decode_frame allocate failed %s ,%d line\n" ,__FILE__ ,__LINE__);
		exit(MEMORY_MALLOC_FAIL);
	}
}

void free_input_memory(Input_Context *ptr_input_ctx){

	if(ptr_input_ctx->yuv_frame)
		av_free(ptr_input_ctx->yuv_frame);

	if(ptr_input_ctx->audio_decode_frame)
		av_free(ptr_input_ctx->audio_decode_frame);
}


