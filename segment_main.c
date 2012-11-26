/*
 * segment_main.c
 *
 *  Created on: Oct 16, 2012
 *      Author: chris
 */

#include <stdio.h>

#include "chris_global.h"
#include "chris_error.h"
#include "segment_yy.h"

int main(int argc ,char * argv[]){

	//define a struct point variable
	Segment_U * seg_union = NULL;

	/*Segment union */
	init_seg_union(&seg_union ,argc ,argv);

	/*segmentation medium	*/
	seg_transcode_main(seg_union);


	/*free memory*/
	free_seg_union(seg_union);
	return 0;
}


