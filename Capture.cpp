#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>

#include "DeckLinkAPI.h"
#include "Capture.h"
#include "Capture_global.h"
#include "chris_global.h"
extern "C"{
#ifndef   UINT64_C

#define   UINT64_C(value)__CONCAT(value,ULL)

#endif
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include "output_handle.h"
#include "segment_utils.h"
#include "libswscale/swscale.h"
#include "libavformat/avformat.h"
}

//may be ,all the following variable can be remove

DeckLinkCaptureDelegate::DeckLinkCaptureDelegate() : m_refCount(0)
{
	pthread_mutex_init(&m_mutex, NULL);
}

DeckLinkCaptureDelegate::~DeckLinkCaptureDelegate()
{
	pthread_mutex_destroy(&m_mutex);
}

ULONG DeckLinkCaptureDelegate::AddRef(void)
{
	pthread_mutex_lock(&m_mutex);
		m_refCount++;
	pthread_mutex_unlock(&m_mutex);

	return (ULONG)m_refCount;
}

ULONG DeckLinkCaptureDelegate::Release(void)
{
	pthread_mutex_lock(&m_mutex);
		m_refCount--;
	pthread_mutex_unlock(&m_mutex);

	if (m_refCount == 0)
	{
		delete this;
		return 0;
	}

	return (ULONG)m_refCount;
}

HRESULT DeckLinkCaptureDelegate::VideoInputFrameArrived(IDeckLinkVideoInputFrame* videoFrame, IDeckLinkAudioInputPacket* audioFrame)
{
	void*					frameBytes;
	void*					audioFrameBytes;
	
	/* Video Frame	*/
	if(videoFrame)
	{	

		if (videoFrame->GetFlags() & bmdFrameHasNoInputSource)
		{
//			fprintf(stderr, "Frame received (#%u) - No input signal detected\n", this->seg_union->picture_capture_no);
		}
		else
		{
			videoFrame->GetBytes(&frameBytes);

			int i ;
			for(i = 0; i < this->prog_num ; i ++){

				if( pthread_mutex_trylock(&this->yuv_video_buf[i]->yuv_buf_mutex) == 0){ //lock sucess

					if(this->yuv_video_buf[i]->have_data_mark == 0){
//						this->yuv_video_buf[i]->yuv_data = (unsigned char *)frameBytes;
						memcpy(this->yuv_video_buf[i]->yuv_data ,(unsigned char *)frameBytes ,this->width_capture * this->height_caputre * 2);

						this->yuv_video_buf[i]->have_data_mark = 1; // not set
						pthread_cond_signal(&this->yuv_video_buf[i]->yuv_buf_cond);
					}
					pthread_mutex_unlock(&this->yuv_video_buf[i]->yuv_buf_mutex);
				}

				this->seg_union[i]->picture_capture_no ++ ;			//here ,do not forget
			}

		}

	}

	/* Handle Audio Frame */
	if (audioFrame)
	{
		//printf("audio .... ,frame count = %ld\n" ,audioFrame->GetSampleFrameCount());
		//int haha = audioFrame->GetSampleFrameCount() * CAPTURE_AUDIO_CHANNEL_NUM * (CAPTURE_AUDIO_SAMPLE_DEPTH / 8);
		audioFrame->GetBytes(&audioFrameBytes);
		int i;

		static int iii = 1;
		static Output_Context *output_ctx1[3];
		if(iii == 1){
			iii = 0;
			int j;
			for(j = 0; j < this->prog_num ; j ++){
				output_ctx1[j] = this->seg_union[j]->output_ctx;
			}
		}
		//here ,can not encode only one time ,but write according the prog_num
		for(i = 0; i < 1 ; i ++){
			do_audio_out(output_ctx1 ,audioFrameBytes
							,audioFrame->GetSampleFrameCount() * CAPTURE_AUDIO_CHANNEL_NUM * (CAPTURE_AUDIO_SAMPLE_DEPTH / 8)
							,audioFrame->GetSampleFrameCount() ,this->prog_num);
		}

//			write(audioOutputFile, audioFrameBytes, audioFrame->GetSampleFrameCount() * AUDIO_CHANNEL_NUM * (AUDIO_SAMPLE_DEPTH / 8));
	}
    return S_OK;
}

HRESULT DeckLinkCaptureDelegate::VideoInputFormatChanged(BMDVideoInputFormatChangedEvents events, IDeckLinkDisplayMode *mode, BMDDetectedVideoInputFormatFlags)
{
    return S_OK;
}

void * encode_yuv_data( void *void_del){

	DeckLinkCaptureDelegate 	*delegate  = (DeckLinkCaptureDelegate 	*)void_del;
	while(1){

		//1.get lock
		pthread_mutex_lock(&delegate->yuv_video_buf[0]->yuv_buf_mutex);

		if(delegate->yuv_video_buf[0]->have_data_mark == 0){
			pthread_cond_wait(&delegate->yuv_video_buf[0]->yuv_buf_cond ,&delegate->yuv_video_buf[0]->yuv_buf_mutex);
		}
		//printf("after wait 1.. ,have_data_mark = %d.\n" ,delegate->yuv_video_buf[0]->have_data_mark);
		//encode
		seg_write_frame(delegate->seg_union[0] ,
						delegate->width_capture ,delegate->height_caputre ,
						VIDEO_STREAM_FLAG  ,delegate->yuv_video_buf[0]->yuv_data );

		//change mark
		delegate->yuv_video_buf[0]->have_data_mark = 0;

		//unlock lock
		pthread_mutex_unlock(&delegate->yuv_video_buf[0]->yuv_buf_mutex);


		//judge exit or not
		if(delegate->quit_mark == 1){
			// do something

			return NULL;
		}

	}

	return NULL;
}

void * encode_yuv_data1( void *void_del){

	DeckLinkCaptureDelegate 	*delegate  = (DeckLinkCaptureDelegate 	*)void_del;
	while(1){

		//1.get lock
		pthread_mutex_lock(&delegate->yuv_video_buf[1]->yuv_buf_mutex);

		if(delegate->yuv_video_buf[1]->have_data_mark == 0){
			pthread_cond_wait(&delegate->yuv_video_buf[1]->yuv_buf_cond ,&delegate->yuv_video_buf[1]->yuv_buf_mutex);
		}
		//printf("after wait 1.. ,have_data_mark = %d.\n" ,delegate->yuv_video_buf[1]->have_data_mark);
//		//encode

		seg_write_frame(delegate->seg_union[1] ,
						delegate->width_capture ,delegate->height_caputre ,
						VIDEO_STREAM_FLAG  ,delegate->yuv_video_buf[1]->yuv_data );

		//change mark
		delegate->yuv_video_buf[1]->have_data_mark = 0;

		//unlock lock
		pthread_mutex_unlock(&delegate->yuv_video_buf[1]->yuv_buf_mutex);

		//judge exit or not
		if(delegate->quit_mark == 1){
			// do something

			return NULL;
		}

	}

	return NULL;
}

void * encode_yuv_data2( void *void_del){

	DeckLinkCaptureDelegate 	*delegate  = (DeckLinkCaptureDelegate 	*)void_del;
	while(1){

		//1.get lock
		pthread_mutex_lock(&delegate->yuv_video_buf[2]->yuv_buf_mutex);

		if(delegate->yuv_video_buf[2]->have_data_mark == 0){
			pthread_cond_wait(&delegate->yuv_video_buf[2]->yuv_buf_cond ,&delegate->yuv_video_buf[2]->yuv_buf_mutex);
		}
		//printf("after wait 1.. ,have_data_mark = %d.\n" ,delegate->yuv_video_buf[2]->have_data_mark);
//		//encode
		seg_write_frame(delegate->seg_union[2] ,
						delegate->width_capture ,delegate->height_caputre ,
						VIDEO_STREAM_FLAG  ,delegate->yuv_video_buf[2]->yuv_data );

		//change mark
		delegate->yuv_video_buf[2]->have_data_mark = 0;

		//unlock lock
		pthread_mutex_unlock(&delegate->yuv_video_buf[2]->yuv_buf_mutex);

		//judge exit or not
		if(delegate->quit_mark == 1){
			// do something

			return NULL;
		}

	}

	return NULL;
}

/*
 * listen keyboard function
 * */
void * key_listen(void *handle) {

	printf("#chris:in monitor pthread\n");

	DeckLinkCaptureDelegate 	*delegate  = (DeckLinkCaptureDelegate 	*)handle;
	fd_set rdfds;
	struct timeval tv;	 	//the duration time for select function listen

	FD_ZERO(&rdfds);
	FD_SET(0, &rdfds);

	tv.tv_sec = 1;
	tv.tv_usec = 0; /* set tv value */

	char str_consle_input1[12] = {0};

	printf("Press [q] to stop\n");

	while (1) {

		int ret = select(1, &rdfds, NULL, NULL, /*&tv*/&tv); /* 注意是最大值还要加1 */
		if (ret < 0) {
			printf(" error\n");/* 出错 */
		} else if (ret == 0) {
			//over time
		} else { //receive key value from keyboard

			fgets(str_consle_input1, 12, stdin); //stdin

			if (strcmp(str_consle_input1, "q\n") == 0) { //notice ,must end with the '\n'
				printf("hahah -->q \n");
				delegate->quit_mark = 1;
				exit(0); //here ,use exit ,and force kill current thread...
//				pthread_exit(0);
				printf("after pthread_exit() in the listen key board thread \n");
			}


			memset(str_consle_input1, 0, 12);
		}

		//rest fd set
		FD_SET(0, &rdfds);
		//reset duration for select function
		tv.tv_sec = 1;
		tv.tv_usec = 0;
	}
	printf("exit while loop\n");

	return NULL;
}


/*
 * callback thread in charge of to capture the video and audio data ,and then put they into pipe;
 *
 * create new thread to read the data from the pipe ,and encode ...
 *
 * in sum : 3 threads
 * */
int main(int argc, char *argv[])
{

	IDeckLink 						*deckLink;
	IDeckLinkInput					*deckLinkInput;
	IDeckLinkDisplayModeIterator	*displayModeIterator;
	//
	IDeckLinkIterator			*deckLinkIterator = CreateDeckLinkIteratorInstance();
	DeckLinkCaptureDelegate 	*delegate;
	IDeckLinkDisplayMode		*displayMode;
	BMDVideoInputFlags			inputFlags = 0;
	BMDDisplayMode				selectedDisplayMode = bmdModeNTSC;
	BMDPixelFormat				pixelFormat = bmdFormat8BitYUV;    //mode 11 only support this pixel format
	int							displayModeCount = 0;
	int							ch;
	bool 						foundDisplayMode = false;
	HRESULT						result;
	

	if (!deckLinkIterator)
	{
		fprintf(stderr, "This application requires the DeckLink drivers installed.\n");
		goto bail;
	}
	
	/* Connect to the first DeckLink instance */
	result = deckLinkIterator->Next(&deckLink);   //traverse decklink card ...
	//The  IDeckLink  object interface represents a physical DeckLink device attached to the host computer.
	if (result != S_OK)
	{
		fprintf(stderr, "No DeckLink PCI cards found.\n");
		goto bail;
	}
    
	if (deckLink->QueryInterface(IID_IDeckLinkInput, (void**)&deckLinkInput) != S_OK)
		goto bail;


	delegate = new DeckLinkCaptureDelegate();
   
	result = deckLinkInput->GetDisplayModeIterator(&displayModeIterator);
	if (result != S_OK)
	{
		fprintf(stderr, "Could not obtain the video output display mode iterator - result = %08x\n", result);
		goto bail;
	}
	
	/*
	 * The  IDeckLinkDisplayModeIterator  object interface is used to enumerate the available
		display modes for a DeckLink device.
	 * */
	while (displayModeIterator->Next(&displayMode) == S_OK)//The Next method returns the next available IDeckLinkDisplayMode interface.
	{

		if (VIDEO_MODE_INDEX == displayModeCount)
		{
			BMDDisplayModeSupport result;
			const char *displayModeName;
			
			foundDisplayMode = true;
			displayMode->GetName(&displayModeName);
			printf(" displayModeName = %s \n" ,displayModeName);

			selectedDisplayMode = displayMode->GetDisplayMode();
			
			printf("selectedDisplayMode = %x \n\n" , selectedDisplayMode);
			deckLinkInput->DoesSupportVideoMode(selectedDisplayMode, pixelFormat, bmdVideoInputFlagDefault, &result, NULL);


			delegate->width_capture = (int)displayMode->GetWidth();
			delegate->height_caputre = (int)displayMode->GetHeight();

			printf("result = %u \n\n" ,result);
			if (result == bmdDisplayModeNotSupported)
			{
				fprintf(stderr, "The display mode %s is not supported with the selected pixel format\n", displayModeName);
				exit(NOT_SUPPORT_MODE);
			}

			break;
		}
		displayModeCount++;
		displayMode->Release();
	} //end while

	if (!foundDisplayMode)
	{
		fprintf(stderr, "Invalid mode %d specified\n", VIDEO_MODE_INDEX);
		exit(INVALID_MODE);
	}

	//*****************************************************************
	//in here ,find capture card mode
	//==============================================
	/*	parse the option */
	delegate->prog_num = parse_option_argument(delegate->seg_union ,argc ,argv);
	//after parse_option_argument function ,input argument have been assignment
	printf("prog_num = %d ,width = %d ,height = %d  \n" ,delegate->prog_num ,delegate->width_capture  ,delegate->height_caputre);

	av_register_all();
	avformat_network_init();

	int i ;
	for(i = 0; i < delegate->prog_num ; i ++){

		/*Segment union */
		init_seg_union(delegate->seg_union[i] ,i );
		seg_write_header(delegate->seg_union[i]);

		/*	init yuv_video_buffer*/
		delegate->yuv_video_buf[i] = (yuv_video_buf_union * )malloc(sizeof(yuv_video_buf_union));
		if(delegate->yuv_video_buf[i] == NULL){
			printf("yuv video buf malloc failed .\n");
			exit(1);
		}

		delegate->yuv_video_buf[i]->yuv_data = (unsigned char *)malloc(delegate->width_capture * delegate->height_caputre * 2);
		if(delegate->yuv_video_buf[i]->yuv_data == NULL){
			printf("yuv_data buffer malloc failed .\n");
			exit(1);
		}

		//take img_conver_ctx from here
		delegate->seg_union[i]->output_ctx->img_convert_ctx = sws_getContext(
				delegate->width_capture ,delegate->height_caputre ,PIX_FMT_UYVY422,
				delegate->seg_union[i]->output_ctx->video_stream->codec->width ,delegate->seg_union[i]->output_ctx->video_stream->codec->height ,PIX_FMT_YUV420P ,
				SWS_BILINEAR /*SWS_BICUBIC*/ ,NULL ,NULL ,NULL);

		//the img_convert_ctx used for the jpeg
		delegate->seg_union[i]->output_ctx->RGB_img_convert_ctx = sws_getContext(
				delegate->seg_union[i]->output_ctx->video_stream->codec->width ,delegate->seg_union[i]->output_ctx->video_stream->codec->height ,PIX_FMT_YUV420P,
				JPEG_WIDTH ,JPEG_HEIGHT ,AV_PIX_FMT_RGB24 ,
				SWS_BILINEAR /*SWS_BICUBIC*/ ,NULL ,NULL ,NULL);

		pthread_mutex_init(&delegate->yuv_video_buf[i]->yuv_buf_mutex, NULL);
		pthread_cond_init(&delegate->yuv_video_buf[i]->yuv_buf_cond, NULL);
		delegate->yuv_video_buf[i]->have_data_mark = 0;


	}

	deckLinkInput->SetCallback(delegate);	// set deckLinkInput callback function

	//===========================


	//new a thread to listen the user input information
	pthread_t pid_key_listion;
	pthread_create(&pid_key_listion , NULL ,key_listen ,delegate);
	//new a thread to encode video data
	pthread_t pid_video_encode;
	pthread_t pid_video_encode1;
	pthread_t pid_video_encode2;
	pthread_create(&pid_video_encode , NULL ,encode_yuv_data ,delegate);

	if(delegate->prog_num == 2){  //two different bitrate
		pthread_t pid_video_encode1;
		pthread_create(&pid_video_encode1 , NULL ,encode_yuv_data1 ,delegate);
	}else if(delegate->prog_num == 3){ //three different bitrate

		pthread_create(&pid_video_encode1 , NULL ,encode_yuv_data1 ,delegate);

		pthread_create(&pid_video_encode2 , NULL ,encode_yuv_data2 ,delegate);
	}




	//*****************************************************************
	/*
	 *The  EnableVideoInput method configures video input and
	 *puts the hardware into video capture mode.
	 *Video input (and optionally audio input) is started by calling  StartStreams .
	 * */

	printf("inputFlags = %u \n" ,inputFlags);
    result = deckLinkInput->EnableVideoInput(selectedDisplayMode, pixelFormat, inputFlags);
    if(result != S_OK)
    {
		fprintf(stderr, "Failed to enable video input. Is another application using the card?\n");
        goto bail;
    }

    /*
     * The  EnableAudioInput method configures audio input
     * and puts the hardware into audio capture mode.
     *  Synchronized audio and video input is started by calling StartStreams .
     * */
    result = deckLinkInput->EnableAudioInput(bmdAudioSampleRate48kHz, CAPTURE_AUDIO_SAMPLE_DEPTH, CAPTURE_AUDIO_CHANNEL_NUM);
    if(result != S_OK)
    {
    	printf("audion enable failed ...\n");
        goto bail;
    }

    /*
     * The  StartStreams  method starts synchronized video and
     * audio capture as configured with EnableVideoInput and optionally  EnableAudioInput.
     * */
	result = deckLinkInput->StartStreams();
    if(result != S_OK)
    {
        goto bail;
    }

    pthread_join(pid_video_encode ,NULL);
    pthread_join(pid_key_listion ,NULL);
	if(delegate->prog_num == 2){  //two different bitrate
	    pthread_join(pid_video_encode1 ,NULL);
	}else if(delegate->prog_num == 3){
	    pthread_join(pid_video_encode1 ,NULL);
	    pthread_join(pid_video_encode2 ,NULL);
	}


    printf("program over......\n");
	// Block main thread until signal occurs

bail:
   	
	if (displayModeIterator != NULL)
	{
		displayModeIterator->Release();
		displayModeIterator = NULL;
	}

    if (deckLinkInput != NULL)
    {
        deckLinkInput->Release();
        deckLinkInput = NULL;
    }

    if (deckLink != NULL)
    {
        deckLink->Release();
        deckLink = NULL;
    }

	if (deckLinkIterator != NULL)
		deckLinkIterator->Release();

    return 0;
}

