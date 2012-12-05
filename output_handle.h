#ifndef _OUTPUT_CTX_H
#define _OUTPUT_CTX_H

#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libswresample/swresample.h"
#include "libavutil/fifo.h"
#include "input_handle.h"

//output file information
typedef struct {
	//output file
	AVFormatContext *ptr_format_ctx;
	AVOutputFormat *fmt;

	/*	audio information */
	AVStream *audio_stream;
	enum CodecID audio_codec_id;

	//audio encodec buffer
	uint8_t *audio_outbuf;
	int audio_outbuf_size;
	//int16_t *samples;


	/*	video information */
	AVStream *video_stream;
	enum CodecID video_codec_id;
	AVPacket pkt;					//video packet encode to output file

	//video encoded buffer [out]
	uint8_t *video_outbuf;
	int 	video_outbuf_size;

	//swscale
	struct SwsContext *img_convert_ctx;
	AVFrame *encoded_yuv_pict;
	uint8_t * pict_buf;

	//audio resample
	struct SwrContext *swr;
	AVFifoBuffer *fifo;     /* for compression: one audio fifo per codec */
	uint8_t *audio_buf;
	unsigned int allocated_audio_buf_size;
	int audio_resample;

	//the input stream
	double sync_ipts;
	double base_ipts;

	/*segment relative	*/
	int mode_type;
	//segment time measure
	int start_time_mark ; //开始计时
	double prev_segment_time ;
	double curr_segment_time;

	int dir_name_len;
	char * ts_name ;						//ts name (rely on the mode_type )
	double segment_duration;			//every segment duration
	//m3u8
	char *full_m3u8_name;							//m3u8 name
	FILE *fp_m3u8;
	char *ts_prfix_name;
	unsigned int segment_no;

	//user can control input
	int frame_rate;						 //frame rate
	int width;							//video width
	int height;							//video height
	int video_rate;						//video bitrate
	int audio_rate;						//audio bitrate
	int sample;							//audio sample
	int channel;						//audio channels

	int frame_count;

}Output_Context;

/*
 * function : init_input
 * @param:	ptr_output_ctx 	 	a structure contain the output file information
 * @param:	output_file			the output file name
 *
 * */
int init_output(Output_Context *ptr_output_ctx, char* output_file );

/*
 * function : open_stream_codec
 * @param:	ptr_output_ctx 	 	a structure contain the output file information
 *
 * */
void open_stream_codec(Output_Context *ptr_output_ctx ,int prog_no);

/*
 * function : encode_video_frame
 * @param:	ptr_output_ctx 	 	a structure contain the output file information
 * @param:	pict				the input picture to encode
 *
 * */
void encode_video_frame(Output_Context *ptr_output_ctx ,AVFrame *pict ,Input_Context *ptr_input_ctx );

/*
 * function : encode_audio_frame
 * @param:	ptr_output_ctx 	 	a structure contain the output file information
 * @param:	buf					buf contain the decode audio data ,and then put into audio encoder
 *
 * */
void encode_audio_frame(Output_Context *ptr_output_ctx[] , uint8_t *buf ,int buf_size ,int prog_num);


/*
 * function : encode_flush
 * @param:	ptr_output_ctx 	 	a structure contain the output file information
 * @param:  nb_ostreams			the number in the output file
 *
 * */
void encode_flush(Output_Context *ptr_output_ctx , int nb_ostreams);


/*
 * function : maybe resample the audio argument ,and then encode the audio data
 * */
void do_audio_out(Output_Context *ptr_output_ctx[] ,void *audio_buf ,int audio_buf_size ,int nb_sample ,int prog_num);
//void do_audio_out(Output_Context *ptr_output_ctx ,Input_Context *ptr_input_ctx ,AVFrame *decoded_frame);


/*
 * function :free the output memory
 * */
void free_output_memory(Output_Context *ptr_output_ctx);

#endif
