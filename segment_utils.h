/*
 * segment_utils.h
 *
 *  Created on: Oct 16, 2012
 *      Author: chris
 */

#ifndef SEGMENT_UTILS_H_
#define SEGMENT_UTILS_H_

#include "segment_yy.h"
#include "output_handle.h"

#define 		bool_interger	0			//configure the ts file accuracy
#ifdef CHRIS
#define  chris_print(format, ...) { \
printf (format, ##__VA_ARGS__); \
fflush (stdout); \
}
#esle
#define chris_print(format ,...)
#endif

/*
 * function :parse_option
 *
 * */
int parse_options_chris(int argc, char *argv[]);
/*
 * function :parse_option
 *
 * */
int parse_option_argument( Segment_U ** segment_union, int argc, char *argv[]);


/*
 *
 * function :create storage if not exist
 * */
void create_directory(char *storage_name);

/*
 *
 * function :splice the first ts name
 * */
void create_first_ts_name(Segment_U * segment_union ,int mode_type);

/*
 * 	concat the full m3u8 name
 *
 * */
void create_m3u8_name(Segment_U * seg_union );


void record_segment_time(Output_Context *ptr_output_ctx);

/*
 * write m3u8 file functions
 * */
void write_m3u8_header(Output_Context *ptr_output_ctx);

void write_m3u8_body(Output_Context *ptr_output_ctx ,double segment_duration);

void write_m3u8_tailer(Output_Context *ptr_output_ctx);

#endif /* SEGMENT_UTILS_H_ */
