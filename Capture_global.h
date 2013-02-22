/*
 * Capture_global.h
 *
 *  Created on: Nov 22, 2012
 *      Author: chris
 */

#ifndef CAPTURE_GLOBAL_H_
#define CAPTURE_GLOBAL_H_



#define 	CAPTURE_AUDIO_CHANNEL_NUM				2
#define 	CAPTURE_AUDIO_SAMPLE_DEPTH				16
#define 	CAPTURE_AUDIO_SAMPLE_RATE				48000


// decklink mode ,in here  I use 11
//#define VIDEO_MODE_INDEX				15
//#define 	CAPTURE_FRAME_RATE			60   //mode 15:HD 720p 60

#define VIDEO_MODE_INDEX				11
#define 	CAPTURE_FRAME_RATE			29.97//60   // HD 1080i 59.94 or  HD 1080p 29.97

//#define VIDEO_MODE_INDEX				8
//#define 	CAPTURE_FRAME_RATE			59.94//60   // HD 1080i 59.94 or  HD 1080p 29.97


#define NOT_SUPPORT_MODE				0x1001
#define INVALID_MODE					0x1002

#endif /* CAPTURE_GLOBAL_H_ */
