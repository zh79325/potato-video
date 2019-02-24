#ifndef POTATO_VIDEO_NATIVE_LIBRARY_H
#define POTATO_VIDEO_NATIVE_LIBRARY_H


#include "FmpegLib.h"
#include "StreamInfo.h"

#include <iostream>
#include "fmpeg/libavutil/mathematics.h"
int cut_video(double from_seconds, double end_seconds, const char *in_filename, const char *out_filename);

int concat_video(const char *dest,   char *src1,   char *src2);

void hello();

#endif