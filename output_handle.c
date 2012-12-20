/*
 * output_handle.c
 *
 *  Created on: Sep 24, 2012
 *      Author: chris
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "libavutil/samplefmt.h"
#include "libavutil/opt.h"


#include "output_handle.h"
#include "input_handle.h"

#include "chris_error.h"
#include "chris_global.h"

#include "segment_utils.h"
#include "Capture_global.h"

//参考了output-example.c
AVStream * add_video_stream (AVFormatContext *fmt_ctx ,enum CodecID codec_id ,Output_Context *ptr_output_ctx){

	AVCodecContext *avctx;
	AVStream *st;

	//add video stream
	st = avformat_new_stream(fmt_ctx ,NULL);
	if(st == NULL){
		printf("in out file ,new video stream fimplicit dailed ..\n");
		exit(NEW_VIDEO_STREAM_FAIL);
	}

	//set the index of the stream
	st->id = ID_VIDEO_STREAM;

	//set AVCodecContext of the stream
	avctx = st->codec;

	avctx->codec_id = codec_id;
	avctx->codec_type = AVMEDIA_TYPE_VIDEO;

	//resolution
	avctx->width = ptr_output_ctx->width;//VIDEO_WIDTH;
	avctx->height = ptr_output_ctx->height;//VIDEO_HEIGHT;

//	//set bit rate
	avctx->bit_rate = ptr_output_ctx->video_rate;//VIDEO_BIT_RATE;
	avctx->rc_max_rate = 2 * ptr_output_ctx->video_rate;//VIDEO_BIT_RATE;
	avctx->rc_min_rate = ptr_output_ctx->video_rate / 2;//VIDEO_BIT_RATE;
//	avctx->bit_rate_tolerance = ptr_output_ctx->video_rate;//VIDEO_BIT_RATE;
//	avctx->rc_buffer_size = ptr_output_ctx->video_rate;//VIDEO_BIT_RATE;
//	avctx->rc_initial_buffer_occupancy = avctx->rc_buffer_size * 3 / 4;
//	avctx->rc_buffer_aggressivity = (float)1.0;
//	avctx->rc_initial_cplx = 0.5;



	avctx->pix_fmt = PIX_FMT_YUV420P;
	avctx->me_range = 24;
	avctx->qcompress = 0.85;
	avctx->qmin = 10;
	avctx->qmax = 41;
	avctx->max_qdiff = 4;

//
	avctx->i_quant_factor = 1.0/1.40f;  //
	avctx->b_quant_factor = 1.30f;	//值越大 B frame 劣化越严重
	avctx->time_base.den = ptr_output_ctx->frame_rate;//VIDEO_FRAME_RATE;
	avctx->time_base.num = 1;


	avctx->coder_type = FF_CODER_TYPE_AC;		//cabac
	//key frame
//	avctx->keyint_min = ptr_output_ctx->frame_rate;//VIDEO_FRAME_RATE;
//	avctx->scenechange_threshold = 0;
//	avctx->gop_size = ptr_output_ctx->frame_rate;//VIDEO_FRAME_RATE;
//
//	//other
//	avctx->global_quality = 6;
//
	avctx->refs = 3;
	avctx->trellis = 2;
//
	avctx->me_method = 8;		//umh
	avctx->me_range = 24;
	avctx->me_subpel_quality = 9; //subme

	avctx->me_cmp = FF_CMP_CHROMA;	//set chroma_me = 1
	avctx->b_frame_strategy = 1; 	//set b-adapt = 2; 1：“快速”演算法，較快，越大的--bframes值會稍微提高速度。當使用此模式時，基本上建議搭配--bframes 16使用。

	avctx->thread_count = 12;	//threads in mediainfo

//	avctx->chromaoffset = 0;
//	avctx->max_qdiff = 4;
//	avctx->qcompress = 0.6f;		//affect mbtree
//	avctx->qblur = 0.5f;
//	avctx->noise_reduction = 0;
//	avctx->scenechange_threshold = 40;
//
//	avctx->flags2 = CODEC_FLAG2_MIXED_REFS;
//	avctx->flags2 |= CODEC_FLAG2_8X8DCT;
//	avctx->flags |= CODEC_FLAG_LOOP_FILTER;

//	avctx->flags2 |= CODEC_FLAG2_AUD;
	avctx->flags2 |= CODEC_FLAG2_FAST;
//	avctx->flags2 |= CODEC_FLAG2_BPYRAMID;   //allow B-frames to be used as references
//	avctx->flags2 |= CODEC_FLAG_NORMALIZE_AQP;
//	avctx->flags2 |= CODEC_FLAG2_WPRED;
//	avctx->flags2 |= CODEC_FLAG2_MBTREE;  //宏块层次Use macroblock tree ratecontrol
//
	// some formats want stream headers to be separate(for example ,asfenc.c ,but not mpegts)
	if (fmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
		avctx->flags |= CODEC_FLAG_GLOBAL_HEADER;

	return st;
}



static AVStream * add_audio_stream (AVFormatContext *fmt_ctx ,enum CodecID codec_id ,Output_Context *ptr_output_ctx){
	AVCodecContext *avctx;
	AVStream *st;

	//add video stream
	st = avformat_new_stream(fmt_ctx ,NULL);
	if(st == NULL){
		printf("in out file ,new video stream failed ..\n");
		exit(NEW_VIDEO_STREAM_FAIL);
	}

	//set the index of the stream
	st->id = 1;

	//set AVCodecContext of the stream
	avctx = st->codec;
	avctx->codec_id = codec_id;
	avctx->codec_type = AVMEDIA_TYPE_AUDIO;

	avctx->sample_fmt = AV_SAMPLE_FMT_S16;
	avctx->bit_rate = ptr_output_ctx->audio_rate;//AUDIO_BIT_RATE;
	avctx->sample_rate = ptr_output_ctx->sample;//AUDIO_SAMPLE_RATE;//ptr_input_ctx->audio_codec_ctx->sample_rate/*44100*/;

	avctx->channels = ptr_output_ctx->channel;//2;

	// some formats want stream headers to be separate(for example ,asfenc.c ,but not mpegts)
	if (fmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
		avctx->flags |= CODEC_FLAG_GLOBAL_HEADER;

	return st;
}

int init_output(Output_Context *ptr_output_ctx, char* output_file ){

	//set AVOutputFormat
    /* allocate the output media context */
	printf("output_file = %s \n" ,output_file);
    avformat_alloc_output_context2(&ptr_output_ctx->ptr_format_ctx, NULL, NULL, output_file);
    if (ptr_output_ctx->ptr_format_ctx == NULL) {
        printf("Could not deduce[推断] output format from file extension: using MPEG.\n");
        avformat_alloc_output_context2(&ptr_output_ctx->ptr_format_ctx, NULL, "mpeg", output_file);
        if(ptr_output_ctx->ptr_format_ctx == NULL){
        	 printf("Could not find suitable output format\n");
        	 exit(NOT_GUESS_OUT_FORMAT);
        }
    }
    //in here ,if I get AVOutputFormat succeed ,the filed audio_codec and video_codec will be set default.
    ptr_output_ctx->fmt = ptr_output_ctx->ptr_format_ctx->oformat;


    /* add audio stream and video stream 	*/
    ptr_output_ctx->video_stream = NULL;
    ptr_output_ctx->audio_stream = NULL;

    ptr_output_ctx->audio_codec_id = CODEC_ID_AAC; //aac
    ptr_output_ctx->video_codec_id = CODEC_ID_H264; //h264

    if (ptr_output_ctx->fmt->video_codec != CODEC_ID_NONE) {

    	ptr_output_ctx->video_stream = add_video_stream(ptr_output_ctx->ptr_format_ctx, ptr_output_ctx->video_codec_id ,ptr_output_ctx);
    	if(ptr_output_ctx->video_stream == NULL){
    		printf("in output ,add video stream failed \n");
    		exit(ADD_VIDEO_STREAM_FAIL);
    	}
    }

    if (ptr_output_ctx->fmt->audio_codec != CODEC_ID_NONE) {

    	ptr_output_ctx->audio_stream = add_audio_stream(ptr_output_ctx->ptr_format_ctx, ptr_output_ctx->audio_codec_id ,ptr_output_ctx);
    	if(ptr_output_ctx->audio_stream == NULL){
    		printf(".in output ,add audio stream failed \n");
    		exit(ADD_AUDIO_STREAM_FAIL);
    	}
    }


    /*	malloc buffer	*/
    ptr_output_ctx->encoded_yuv_pict = avcodec_alloc_frame();
    if(ptr_output_ctx->encoded_yuv_pict == NULL){
		printf("yuv_frame allocate failed %s ,%d line\n" ,__FILE__ ,__LINE__);
		exit(MEMORY_MALLOC_FAIL);
	}
    int size = avpicture_get_size(ptr_output_ctx->video_stream->codec->pix_fmt ,
    		ptr_output_ctx->video_stream->codec->width ,
    		ptr_output_ctx->video_stream->codec->height);

    printf("size = %d ,width = %d \n" ,size ,ptr_output_ctx->video_stream->codec->width );
    ptr_output_ctx->pict_buf = av_malloc(size);
    if(ptr_output_ctx->pict_buf == NULL){
    	printf("pict allocate failed ...\n");
    	exit(MEMORY_MALLOC_FAIL);
    }
    //bind
    avpicture_fill((AVPicture *)ptr_output_ctx->encoded_yuv_pict ,ptr_output_ctx->pict_buf ,
    				ptr_output_ctx->video_stream->codec->pix_fmt ,
    		    		ptr_output_ctx->video_stream->codec->width ,
    		    		ptr_output_ctx->video_stream->codec->height);


    /*	init some member value */
    ptr_output_ctx->audio_resample = 0;
    ptr_output_ctx->swr = NULL;
    ptr_output_ctx->base_ipts = 0;
    //segment time
    ptr_output_ctx->prev_segment_time = 0.0;
    ptr_output_ctx->curr_segment_time = 0.0;
    ptr_output_ctx->start_time_mark = 0;


    /*output the file information */
    av_dump_format(ptr_output_ctx->ptr_format_ctx, 0, output_file, 1);

    //fifo
    ptr_output_ctx->fifo = av_fifo_alloc(1024);
	if (!ptr_output_ctx->fifo) {
		exit (1);
	}
	av_log(NULL, AV_LOG_WARNING ,"--av_fifo_size(ost->fifo) = %d \n" ,av_fifo_size(ptr_output_ctx->fifo));  //输出是0？！

	//add
	pthread_mutex_init(&ptr_output_ctx->output_mutex ,NULL);

	return 0;
}


//===========================================================
static void open_video (Output_Context *ptr_output_ctx ,AVStream * st ,int prog_no){

	AVCodec *video_encode;
	AVCodecContext *video_codec_ctx;

	video_codec_ctx = st->codec;

	//find video encode
	video_encode = avcodec_find_encoder(video_codec_ctx->codec_id);
	if(video_encode == NULL){
		printf("in output ,open_video ,can not find video encode.\n");
		exit(NO_FIND_VIDEO_ENCODE);
	}

	AVDictionary *opts = NULL;

	if(prog_no == 0){
		av_dict_set(&opts, "profile", "main", 0);	//set profile
		av_dict_set(&opts, "level", "30", 0);		//set level
		av_dict_set(&opts, "tune", "film", 0);
		//av_dict_set(&opts, "preset", "veryslow", 0);
		av_dict_set(&opts, "deblock", "0:0", 0);	// / deblock = 1:0:0

		av_dict_set(&opts, "aq_mode", "1", 0);
		av_dict_set(&opts, "psy", "1", 0);
		av_dict_set(&opts, "psy_rd", "1.00:0.15", 0);
//		av_dict_set(&opts, "mixed_refs", "1", 0);
		av_dict_set(&opts, "fast_pskip", "0", 0);
		av_dict_set(&opts, "b_pyramid", "0", 0);

		av_dict_set(&opts, "rc_lookahead", "60", 0);
//		av_dict_set(&opts, "partitions", "all", 0);

		//connect the string content x264opts
		av_dict_set(&opts, "x264opts", "ref=3:me=umh:bframes=3:b-adapt=1:force-cfr=1" ,0);


	}else if(prog_no == 1){
		av_dict_set(&opts, "profile", "high", 0);
//		av_dict_set(&opts, "level", "31", 0);
		av_dict_set(&opts, "tune", "film", 0);
		av_dict_set(&opts, "preset", "veryslow", 0);
		av_dict_set(&opts, "deblock", "-1:-1", 0);
		//connect the string content x264opts
		av_dict_set(&opts, "x264opts", "vbv-bufsize=2000:vbv-maxrate=1000:crf=18:force-cfr=1" ,0);
	}else if(prog_no == 2){
		av_dict_set(&opts, "profile", "high", 0);
		av_dict_set(&opts, "level", "31", 0);
		av_dict_set(&opts, "tune", "film", 0);
		av_dict_set(&opts, "preset", "slower", 0);
		//connect the string content x264opts
		av_dict_set(&opts, "x264opts", "bitrate=2000:subme=10:trellis=2:bframes=3:vbv-maxrate=2000:vbv-bufsize=6000:"
				"force-cfr=1" ,0); //:nal-hrd=vbr

	}

	//open video encode
	if(avcodec_open2(video_codec_ctx ,video_encode ,&opts/*NULL*/) < 0){

		printf("in open_video function ,can not open video encode.\n");
		exit(OPEN_VIDEO_ENCODE_FAIL);
	}

	//set video encoded buffer
	ptr_output_ctx->video_outbuf = NULL;
	if (!(ptr_output_ctx->ptr_format_ctx->oformat->flags & AVFMT_RAWPICTURE)) {//in ffmpeg,only nullenc and yuv4mpeg have this flags
																		//so ,mp4 and mpegts both go in here
		printf(".....malloc video buffer ...\n");
		ptr_output_ctx->video_outbuf_size = VIDEO_OUT_BUF_SIZE;
		ptr_output_ctx->video_outbuf = av_malloc(ptr_output_ctx->video_outbuf_size);
		if(ptr_output_ctx->video_outbuf == NULL){
			printf("video_outbuf malloc failed ...\n");
			exit(MEMORY_MALLOC_FAIL);
		}
	}


}

static void open_audio (Output_Context *ptr_output_ctx ,AVStream * st){

	AVCodec *audio_encode;
	AVCodecContext *audio_codec_ctx;

	audio_codec_ctx = st->codec;

	//find audio encode
	audio_encode = avcodec_find_encoder(audio_codec_ctx->codec_id);
	if(audio_encode == NULL){
		printf("in output ,open_audio ,can not find audio encode.\n");
		exit(NO_FIND_AUDIO_ENCODE);
	}

//    //add acc experimental
//    audio_codec_ctx->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL; //if not set ,the follow codec can not perform

	//open audio encode
	if(avcodec_open2(audio_codec_ctx ,audio_encode ,NULL) < 0){

		printf("in open_audio function ,can not open audio encode.\n");
		exit(OPEN_AUDIO_ENCODE_FAIL);
	}

	ptr_output_ctx->audio_outbuf_size = AUDIO_OUT_BUF_SIZE;
	ptr_output_ctx->audio_outbuf = av_malloc(ptr_output_ctx->audio_outbuf_size);
	if (ptr_output_ctx->audio_outbuf == NULL) {
		printf("audio_outbuf malloc failed ...\n");
		exit(MEMORY_MALLOC_FAIL);
	}

    /* ugly hack for PCM codecs (will be removed ASAP with new PCM
       support to compute the input frame size in samples */
	int audio_input_frame_size;
    if (audio_codec_ctx->frame_size <= 1) {
    	audio_input_frame_size = ptr_output_ctx->audio_outbuf_size / audio_codec_ctx->channels;
    	printf("&&$$&&#&#&#&&#&#&#&&#\n\n");
    	sleep(10);
        switch(st->codec->codec_id) {
        case CODEC_ID_PCM_S16LE:
        case CODEC_ID_PCM_S16BE:
        case CODEC_ID_PCM_U16LE:
        case CODEC_ID_PCM_U16BE:
            audio_input_frame_size >>= 1;
            break;
        default:
            break;
        }
    } else {
        audio_input_frame_size = audio_codec_ctx->frame_size;
    }
    //ptr_output_ctx->samples = av_malloc(audio_input_frame_size * 2 * audio_codec_ctx->channels);

}


void open_stream_codec(Output_Context *ptr_output_ctx ,int prog_no){

	open_video (ptr_output_ctx ,ptr_output_ctx->video_stream ,prog_no);

	open_audio (ptr_output_ctx ,ptr_output_ctx->audio_stream);

}

void encode_video_frame(Output_Context *ptr_output_ctx, AVFrame *pict,
		Input_Context *ptr_input_ctx ) {

//	static int frame_count = 0;			// use multiple ,do not define static variable in function
	int nb_frames;
	double sync_ipts;
	double duration = 0;

	/* compute the duration */
//	duration =
//			FFMAX(av_q2d(ptr_input_ctx->ptr_format_ctx->streams[ptr_input_ctx->video_index]->time_base),
//							av_q2d(ptr_input_ctx->video_codec_ctx->time_base));

	duration =  1/CAPTURE_FRAME_RATE;
//	if (ptr_input_ctx->ptr_format_ctx->streams[ptr_input_ctx->video_index]->avg_frame_rate.num)
//		duration = FFMAX(duration, 1/av_q2d(ptr_input_ctx->ptr_format_ctx->streams[ptr_input_ctx->video_index]->avg_frame_rate));

//	duration = FFMAX(duration, 1/av_q2d(ptr_input_ctx->ptr_format_ctx->streams[ptr_input_ctx->video_index]->avg_frame_rate));
	duration /= av_q2d(ptr_output_ctx->video_stream->codec->time_base);


	/*	compute the sync_ipts ,use for to determine duplicate or drop the encode pic*/
	sync_ipts = ptr_output_ctx->sync_ipts / av_q2d(ptr_output_ctx->video_stream->codec->time_base);


    /* by default, we output a single frame ,there is no different in fps of input file  and fps of output file*/
    nb_frames = 1;


    //compute the vdelta ,do not forget the duration
    double vdelta = sync_ipts - ptr_output_ctx->frame_count + duration;

	// FIXME set to 0.5 after we fix some dts/pts bugs like in avidec.c
	if (vdelta < -1.1)
		nb_frames = 0;
	else if (vdelta > 1.1)
		nb_frames = lrintf(vdelta);


	//set chris_count
	int tmp_count;
	for (tmp_count = 0; tmp_count < nb_frames; tmp_count++) {
		//encode the image
		int video_encoded_out_size;
		pict->pts = ptr_output_ctx->frame_count++;

		video_encoded_out_size = avcodec_encode_video(
				ptr_output_ctx->video_stream->codec,
				ptr_output_ctx->video_outbuf, ptr_output_ctx->video_outbuf_size,
				pict);

		if (video_encoded_out_size < 0) {
			fprintf(stderr, "Error encoding video frame\n");
			exit(VIDEO_ENCODE_ERROR);
		}

//		if (video_encoded_out_size == 0)  //the first several number  pict ,there will no data to output because of the AVCodecContext buffer
//			return;
		//in here ,video_encodec_out_size > 0

		if(video_encoded_out_size > 0){
			//AVPacket pkt; //use ptr_output_ctx->pkt instead
			av_init_packet(&ptr_output_ctx->pkt);
			ptr_output_ctx->pkt.stream_index = ptr_output_ctx->video_stream->index;
			ptr_output_ctx->pkt.data = ptr_output_ctx->video_outbuf; // packet data will be allocated by the encoder
			ptr_output_ctx->pkt.size = video_encoded_out_size;

			if (ptr_output_ctx->video_stream->codec->coded_frame->pts
					!= AV_NOPTS_VALUE)
				ptr_output_ctx->pkt.pts = av_rescale_q(
						ptr_output_ctx->video_stream->codec->coded_frame->pts,
						ptr_output_ctx->video_stream->codec->time_base,
						ptr_output_ctx->video_stream->time_base);

			if (ptr_output_ctx->video_stream->codec->coded_frame->key_frame)
				ptr_output_ctx->pkt.flags |= AV_PKT_FLAG_KEY;

			//get lock
			pthread_mutex_lock(&ptr_output_ctx->output_mutex);
#if 0
			//judge if key frame or not
			if(ptr_output_ctx->pkt.flags && AV_PKT_FLAG_KEY){

				//init segment_time
				record_segment_time(ptr_output_ctx);

			}
#endif

//			av_write_frame(ptr_output_ctx->ptr_format_ctx, &ptr_output_ctx->pkt);
			av_interleaved_write_frame(ptr_output_ctx->ptr_format_ctx, &ptr_output_ctx->pkt);
			av_free_packet(&ptr_output_ctx->pkt);
			pthread_mutex_unlock(&ptr_output_ctx->output_mutex);
		}

	}

}


void encode_audio_frame(Output_Context *ptr_output_ctx1[] , uint8_t *buf ,int buf_size ,int prog_num){

	Output_Context *ptr_output_ctx = ptr_output_ctx1[0];
	int ret;
	AVCodecContext *c = ptr_output_ctx->audio_stream->codec;


	//packet for output
	AVPacket pkt;
	av_init_packet(&pkt);
	pkt.data = NULL;
	pkt.size = 0;
	//frame for input
	AVFrame *frame = avcodec_alloc_frame();
	if (frame == NULL) {
		printf("frame malloc failed ...\n");
		exit(1);
	}

	frame->nb_samples = buf_size /
					(c->channels * av_get_bytes_per_sample(c->sample_fmt));

	if ((ret = avcodec_fill_audio_frame(frame, c->channels, AV_SAMPLE_FMT_S16,
				buf, buf_size, 1)) < 0) {
		av_log(NULL, AV_LOG_FATAL, ".Audio encoding failed\n");
		exit(AUDIO_ENCODE_ERROR);
	}

	int got_packet = 0;
	if (avcodec_encode_audio2(c, &pkt, frame, &got_packet) < 0) {
		av_log(NULL, AV_LOG_FATAL, "..Audio encoding failed\n");
		exit(AUDIO_ENCODE_ERROR);
	}
	pkt.pts = 0;
	pkt.stream_index = ptr_output_ctx->audio_stream->index;


//	av_write_frame(ptr_output_ctx->ptr_format_ctx, &pkt);
	int i = 0;
	//printf("prog_num = %d , pkt.stream = %d \n" ,prog_num ,pkt.stream_index);
	for(i = 0 ;i < prog_num ; i ++){
		pthread_mutex_lock(&(ptr_output_ctx1[i]->output_mutex));
//		av_write_frame(ptr_output_ctx1[i]->ptr_format_ctx, &pkt);
		av_interleaved_write_frame(ptr_output_ctx1[i]->ptr_format_ctx, &pkt);
		pthread_mutex_unlock(&(ptr_output_ctx1[i]->output_mutex));
	}

	av_free(frame);
	av_free_packet(&pkt);
}



/*	add silence audio data	*/
static void generate_silence(uint8_t* buf, enum AVSampleFormat sample_fmt, size_t size)
{
    int fill_char = 0x00;
    if (sample_fmt == AV_SAMPLE_FMT_U8)
        fill_char = 0x80;
    memset(buf, fill_char, size);
}

void encode_flush(Output_Context *ptr_output_ctx , int nb_ostreams){

	int i ;

	for (i = 0; i < nb_ostreams; i++){

		AVStream *st = ptr_output_ctx->ptr_format_ctx->streams[i];
		AVCodecContext *enc = st->codec;
		int stop_encoding = 0;

		for (;;){
			AVPacket pkt;
			int fifo_bytes;
			av_init_packet(&pkt);
			pkt.data = NULL;
			pkt.size = 0;

			switch (st->codec->codec_type) {
			/*audio stream*/
			case AVMEDIA_TYPE_AUDIO:
			{


				fifo_bytes = av_fifo_size(ptr_output_ctx->fifo);
				if (fifo_bytes > 0) {
					/* encode any samples remaining in fifo */
					int frame_bytes = fifo_bytes;
					av_fifo_generic_read(ptr_output_ctx->fifo, ptr_output_ctx->audio_buf, fifo_bytes, NULL);

					/* pad last frame with silence if needed */
					frame_bytes = enc->frame_size * enc->channels *
								  av_get_bytes_per_sample(enc->sample_fmt);
					if (ptr_output_ctx->allocated_audio_buf_size < frame_bytes)
						exit(1);
					generate_silence(ptr_output_ctx->audio_buf+fifo_bytes, enc->sample_fmt, frame_bytes - fifo_bytes);

					printf("audio ...........\n");
//					encode_audio_frame(ptr_output_ctx, ptr_output_ctx->audio_buf, frame_bytes);  //???????

				} else {
					/* flush encoder with NULL frames until it is done
					   returning packets */
					int got_packet = 0;
					int ret1;
					ret1 = avcodec_encode_audio2(enc, &pkt, NULL, &got_packet);
					if ( ret1 < 0) {
						av_log(NULL, AV_LOG_FATAL, "..Audio encoding failed\n");
						exit(AUDIO_ENCODE_ERROR);
					}

					printf("audio ...........\n");
					if (ret1 == 0){
						stop_encoding = 1;
						break;
					}
					pkt.pts = 0;
					pkt.stream_index = ptr_output_ctx->audio_stream->index;

					av_write_frame(ptr_output_ctx->ptr_format_ctx, &pkt);

					av_free(&pkt);
				}

				break;

			}
			/*video stream*/
			case AVMEDIA_TYPE_VIDEO:
			{
				 int nEncodedBytes = avcodec_encode_video(
								ptr_output_ctx->video_stream->codec,
								ptr_output_ctx->video_outbuf, ptr_output_ctx->video_outbuf_size,
								NULL);

				if (nEncodedBytes < 0) {
					av_log(NULL, AV_LOG_FATAL, "Video encoding failed\n");
					exit(VIDEO_FLUSH_ERROR);
				}

				printf("video ...........\n");
				if(nEncodedBytes > 0){
					pkt.stream_index = ptr_output_ctx->video_stream->index;
					pkt.data = ptr_output_ctx->video_outbuf; // packet data will be allocated by the encoder
					pkt.size = nEncodedBytes;

					if (ptr_output_ctx->video_stream->codec->coded_frame->pts
							!= AV_NOPTS_VALUE)
						pkt.pts =
								av_rescale_q(
										ptr_output_ctx->video_stream->codec->coded_frame->pts,
										ptr_output_ctx->video_stream->codec->time_base,
										ptr_output_ctx->video_stream->time_base);

					if (ptr_output_ctx->video_stream->codec->coded_frame->key_frame)
						pkt.flags |= AV_PKT_FLAG_KEY;

					//judge key frame
					if(ptr_output_ctx->pkt.flags && AV_PKT_FLAG_KEY){
						//init segment_time
						record_segment_time(ptr_output_ctx);

					}

					av_write_frame(ptr_output_ctx->ptr_format_ctx, &pkt);

					av_free_packet(&pkt);
				}else if(nEncodedBytes == 0){
					stop_encoding = 1;
					break;
				}
				break;
			}

			default:
				stop_encoding = 1;
			}//end switch

			if(stop_encoding) break;

		}//end for


	}


}


void do_audio_out(Output_Context *ptr_output_ctx1[] ,void * src_audio_buf ,int src_audio_buf_size ,int nb_sample ,int prog_num){

	Output_Context *ptr_output_ctx = ptr_output_ctx1[0];
	enum AVSampleFormat dec_sample_fmt = AV_SAMPLE_FMT_S16;
	uint64_t dec_layout_channel = 0;
	uint8_t *buftmp;
	int64_t audio_buf_size, size_out;

	int frame_bytes;
	AVCodecContext *enc = ptr_output_ctx->audio_stream->codec;
//	AVCodecContext *dec = ptr_input_ctx->audio_codec_ctx;
	int osize = av_get_bytes_per_sample(enc->sample_fmt);

	int isize = av_get_bytes_per_sample(dec_sample_fmt);
//	printf("osize = %d  ,isize = %d \n" ,osize ,isize);
//
//	printf("enc->channel_layout  = %lu \n" ,enc->channel_layout );
//	while(1);
	/*	in buf ,is the decoded audio data	*/
	uint8_t *buf = (uint8_t * )src_audio_buf;
	int size = src_audio_buf_size;

	int64_t allocated_for_size = size;

/*need_realloc:*/
	audio_buf_size = (allocated_for_size + isize * /*dec->channels*/CAPTURE_AUDIO_CHANNEL_NUM - 1) / (isize * CAPTURE_AUDIO_CHANNEL_NUM);
	audio_buf_size = (audio_buf_size * enc->sample_rate + /*dec->sample_rate*/CAPTURE_AUDIO_SAMPLE_RATE) / CAPTURE_AUDIO_SAMPLE_RATE;
	audio_buf_size = audio_buf_size * 2 + 10000; // safety factors for the deprecated resampling API
	audio_buf_size = FFMAX(audio_buf_size, enc->frame_size);
	audio_buf_size *= osize * enc->channels;

	if (audio_buf_size > INT_MAX) {
		av_log(NULL, AV_LOG_FATAL, "Buffer sizes too large\n");
		exit(1);
	}

	av_fast_malloc(&(ptr_output_ctx->audio_buf), &(ptr_output_ctx->allocated_audio_buf_size), audio_buf_size);
	if (!ptr_output_ctx->audio_buf) {
		av_log(NULL, AV_LOG_FATAL, "Out of memory in do_audio_out\n");
		exit(1);
	}


	/*	judge resample or not*/
    if (enc->channels != CAPTURE_AUDIO_CHANNEL_NUM
    		|| enc->sample_fmt != dec_sample_fmt
    		|| enc->sample_rate!= CAPTURE_AUDIO_SAMPLE_RATE
    )
    	ptr_output_ctx->audio_resample = 1;
    /*	init SwrContext	,perform only one time	*/
	if (ptr_output_ctx->audio_resample && !ptr_output_ctx->swr) {

		printf("ptr_output_ctx->audio_resample = %d ,and we need resample \n" ,ptr_output_ctx->audio_resample);
		ptr_output_ctx->swr = swr_alloc_set_opts(NULL, enc->channel_layout,
				enc->sample_fmt, enc->sample_rate, /*dec->channel_layout*/dec_layout_channel,
				dec_sample_fmt, CAPTURE_AUDIO_SAMPLE_RATE, 0, NULL);

		if (av_opt_set_int(ptr_output_ctx->swr, "ich", CAPTURE_AUDIO_CHANNEL_NUM, 0) < 0) {
			av_log(NULL, AV_LOG_FATAL,
					"Unsupported number of input channels\n");
			exit(1);
		}
		if (av_opt_set_int(ptr_output_ctx->swr, "och", enc->channels, 0) < 0) {
			av_log(NULL, AV_LOG_FATAL,
					"Unsupported number of output channels\n");
			exit(1);
		}

		if (ptr_output_ctx->swr && swr_init(ptr_output_ctx->swr) < 0) {
			av_log(NULL, AV_LOG_FATAL, "swr_init() failed\n");
			swr_free(&ptr_output_ctx->swr);
		}

		if (!ptr_output_ctx->swr) {
			av_log(NULL, AV_LOG_FATAL,
					"Can not resample %d channels @ %d Hz to %d channels @ %d Hz\n",
					CAPTURE_AUDIO_CHANNEL_NUM, CAPTURE_AUDIO_SAMPLE_RATE, enc->channels,
					enc->sample_rate);
			exit(1);
		}

	}

	if(ptr_output_ctx->audio_resample ){

		//swr_convert
		buftmp = ptr_output_ctx->audio_buf;
		size_out = swr_convert(ptr_output_ctx->swr, ( uint8_t*[]) {buftmp},audio_buf_size / (enc->channels * osize),
															 (const uint8_t*[]){buf   }, size / (CAPTURE_AUDIO_CHANNEL_NUM * isize));
		size_out = size_out * enc->channels * osize;
	}else {
		buftmp = buf ;
		size_out = size;

	}


	//write data

//		printf("av_fifo_size(ptr_output_ctx->fifo) = %d \n" ,av_fifo_size(ptr_output_ctx->fifo));
		if (av_fifo_realloc2(ptr_output_ctx->fifo,
				av_fifo_size(ptr_output_ctx->fifo) + size_out) < 0) {
			av_log(NULL, AV_LOG_FATAL, "av_fifo_realloc2() failed\n");
			exit(1);
		}
		av_fifo_generic_write(ptr_output_ctx->fifo, buftmp, size_out, NULL);


		frame_bytes = enc->frame_size * osize * enc->channels;

		while (av_fifo_size(ptr_output_ctx->fifo) >= frame_bytes) {
//			printf("av_fifo_size(ost->fifo) = %d ,frame_bytes = %d\n" ,av_fifo_size(ptr_output_ctx->fifo) ,frame_bytes);
			av_fifo_generic_read(ptr_output_ctx->fifo, ptr_output_ctx->audio_buf, frame_bytes, NULL);

			encode_audio_frame(ptr_output_ctx1, ptr_output_ctx->audio_buf, frame_bytes ,prog_num);
			//printf("encode audio ....\n");
		}

}


void free_output_memory(Output_Context *ptr_output_ctx){


	//malloc in open_video
	av_free(ptr_output_ctx->video_outbuf);

	//malloc in open_audio
	av_free(ptr_output_ctx->audio_outbuf);

	//audio buffer
	av_fifo_free(ptr_output_ctx->fifo);

	av_free(ptr_output_ctx->pict_buf);

	av_free(ptr_output_ctx->encoded_yuv_pict);

	//close codecs
	avcodec_close(ptr_output_ctx->video_stream->codec);
	avcodec_close(ptr_output_ctx->audio_stream->codec);




	//free
	avformat_free_context(ptr_output_ctx->ptr_format_ctx);



}
