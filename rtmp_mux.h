/*
 * rtmp_mux.h
 *
 *  Created on: May 18, 2013
 *      Author: chris
 */

#ifndef RTMP_MUX_H_
#define RTMP_MUX_H_

#include "output_handle.h"

/*
 * init_rtmp_output function
 *
 * */
int init_rtmp_output(Output_Context *ptr_output_ctx ,char *rtmp_stream_name ,Output_Context *ptr_output_ctx_copy_from);


#endif /* RTMP_MUX_H_ */
