#ifndef POTATO_VIDEO_NATIVE_LIBRARY_H
#define POTATO_VIDEO_NATIVE_LIBRARY_H

extern "C"{
#include <libswscale/swscale.h>
#include <libavcodec/avcodec.h>
#include <libavutil/mathematics.h>
#include <libavformat/avformat.h>
#include <libavutil/timestamp.h>

}
#include "fmpeg/libavutil/mathematics.h"
#include <iostream>
int cut_video(double from_seconds, double end_seconds, const char* in_filename, const char* out_filename);
void hello();

#endif