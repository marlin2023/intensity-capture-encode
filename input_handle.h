#ifndef _INPUT_CTX_H
#define _INPUT_CTX_H

#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"

typedef struct {
	//input file
	AVFormatContext *ptr_format_ctx;
	AVPacket pkt;

	/*	audio information */
	int audio_index;
	AVCodecContext *audio_codec_ctx;
	AVCodec 	*audio_codec;

	AVFrame 	*audio_decode_frame;
	int 		audio_size;				//audio size ,the output invoke avcodec_decode_audio4 one time

	/*	video information */
	int video_index;
	AVCodecContext *video_codec_ctx;
	AVCodec 	*video_codec;
	AVFrame 	*yuv_frame;
//	AVFrame *encoded_pict;
//	uint8_t * pict_buf;

	/*	decode mark	*/
	int 		mark_have_frame;
}Input_Context;



/*
 * function : init_input
 * @param:	ptr_input_ctx 	 	a structure contain the inputfile information
 * @param:	input_file			the input file name
 *
 * */
int init_input(Input_Context *ptr_input_ctx, char* input_file);

/*
 * function : free input ,in order to close codecs and input streams
 * @param:	ptr_input_ctx 	 	a structure contain the inputfile information
 *
 * */
void free_input(Input_Context *ptr_input_ctx);


/*
 * function : manual malloc some memory
 * @param:	ptr_input_ctx 	 	a structure contain the inputfile information
 *
 * */
void malloc_input_memory(Input_Context *ptr_input_ctx);


void free_input_memory(Input_Context *ptr_input_ctx);
/*
 * function : init_input
 * @param:	ptr_input_ctx 	 	a structure contain the inputfile information
 * @param:	pkt					the packet read from the AVFormatContext
 *
 * */
void decode_frame(Input_Context *ptr_input_ctx ,AVPacket *pkt);


#endif
