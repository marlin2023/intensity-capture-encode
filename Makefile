#ffmpeg-1.2
lib_path=/home/chris/work/ffmpeg/refs/ffmpeg-1.2.1/lib
include_path=/home/chris/work/ffmpeg/refs/ffmpeg-1.2.1/include

all:  
	g++  -g -pg -c Capture.cpp  DeckLinkAPIDispatch.cpp	-I${include_path}
	gcc  -g -pg -c segment_utils.c segment_yy.c	input_handle.c 	output_handle.c	 -I${include_path}  
	gcc *.o -g -pg  -o capture -lstdc++  -L${lib_path} -lavformat -lavcodec  -lswscale -lswresample -lavutil -lm -ldl -lz -lpthread -lx264 -lfaac -ljpeg -lrtmp -lssl

clean:
	rm -f capture
	rm *.o
#-D_CHRIS
