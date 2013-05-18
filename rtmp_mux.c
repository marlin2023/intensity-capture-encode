/*
 * rtmp_mux.c
 *
 *  Created on: May 18, 2013
 *      Author: chris
 */

#include <stdio.h>

#include "rtmp_mux.h"
#include "chris_error.h"



int init_rtmp_output(Output_Context *ptr_output_ctx ,char *rtmp_stream_name ,Output_Context *ptr_output_ctx_copy_from) {

	/* allocate the output media context */
	avformat_alloc_output_context2(&ptr_output_ctx->ptr_format_ctx, NULL, "flv", rtmp_stream_name);
	if (ptr_output_ctx->ptr_format_ctx == NULL) {
		printf(
				"Could not deduce[推断] output format from file extension: using MPEG.\n");
		exit(NOT_GUESS_OUT_FORMAT);
	}
	//in here ,if I get AVOutputFormat succeed ,the filed audio_codec and video_codec will be set default.
	ptr_output_ctx->fmt = ptr_output_ctx->ptr_format_ctx->oformat;

	/* add audio stream and video stream 	*/
	ptr_output_ctx->video_stream = NULL;
	ptr_output_ctx->audio_stream = NULL;

	ptr_output_ctx->audio_codec_id = CODEC_ID_AAC; //aac
	ptr_output_ctx->video_codec_id = CODEC_ID_H264; //h264

	if (ptr_output_ctx->fmt->video_codec != CODEC_ID_NONE) {

		ptr_output_ctx->video_stream = add_video_stream(
				ptr_output_ctx->ptr_format_ctx, ptr_output_ctx->video_codec_id ,ptr_output_ctx_copy_from ,-1);
		if (ptr_output_ctx->video_stream == NULL) {
			printf("in output ,add video stream failed \n");
			exit(ADD_VIDEO_STREAM_FAIL);
		}
	}

	if (ptr_output_ctx->fmt->audio_codec != CODEC_ID_NONE) {

		ptr_output_ctx->audio_stream = add_audio_stream(
				ptr_output_ctx->ptr_format_ctx, ptr_output_ctx->audio_codec_id,ptr_output_ctx_copy_from);
		if (ptr_output_ctx->audio_stream == NULL) {
			printf(".in output ,add audio stream failed \n");
			exit(ADD_AUDIO_STREAM_FAIL);
		}
	}

	/*output the file information */
	av_dump_format(ptr_output_ctx->ptr_format_ctx, 0, rtmp_stream_name, 1);

}

