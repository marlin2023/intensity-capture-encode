#ffmpeg-1.2
lib_path=/home/chris/work/ffmpeg/refs/ffmpeg-1.2.1/lib
include_path=/home/chris/work/ffmpeg/refs/ffmpeg-1.2.1/include

all:  
	g++ -D_CHRIS -g -pg -c Capture.cpp  DeckLinkAPIDispatch.cpp	-I${include_path}
	gcc -D_CHRIS -g -pg -c segment_utils.c segment_yy.c	input_handle.c 	output_handle.c	 -I${include_path}  
	gcc *.o -g -pg -D_CHRIS  -o capture -lstdc++  -L${lib_path} -lavformat -lavcodec  -lswscale -lswresample -lavutil -lm -ldl -lz -lpthread -lx264 -lfaac -ljpeg -lrtmp -lssl

clean:
	rm -f capture
	rm *.o
#yyt@27.189.31.59:~/
