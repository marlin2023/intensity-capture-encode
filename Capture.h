#ifndef __CAPTURE_H__
#define __CAPTURE_H__

#include "DeckLinkAPI.h"

extern "C"{
//#include "chris_global.h"
//#include "chris_error.h"
#ifndef   UINT64_C

#define   UINT64_C(value)__CONCAT(value,ULL)

#endif
#include "segment_yy.h"

typedef struct yuv_video_buf_u{

	unsigned char * yuv_data;
	pthread_mutex_t		yuv_buf_mutex;
	pthread_cond_t		yuv_buf_cond;

	int have_data_mark;

}yuv_video_buf_union;

}

class DeckLinkCaptureDelegate : public IDeckLinkInputCallback
{
public:
	//
	Segment_U * seg_union;
	yuv_video_buf_union *yuv_video_buf;

	int quit_mark;
	DeckLinkCaptureDelegate();
	~DeckLinkCaptureDelegate();

	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, LPVOID *ppv) { return E_NOINTERFACE; }
	virtual ULONG STDMETHODCALLTYPE AddRef(void);
	virtual ULONG STDMETHODCALLTYPE  Release(void);
	virtual HRESULT STDMETHODCALLTYPE VideoInputFormatChanged(BMDVideoInputFormatChangedEvents, IDeckLinkDisplayMode*, BMDDetectedVideoInputFormatFlags);
	virtual HRESULT STDMETHODCALLTYPE VideoInputFrameArrived(IDeckLinkVideoInputFrame*, IDeckLinkAudioInputPacket*);

private:
	ULONG				m_refCount;
	pthread_mutex_t		m_mutex;


};


#define 	VIDEO_STREAM_FLAG 	0
#define 	AUDIO_STREAM_FLAG	1





#endif
