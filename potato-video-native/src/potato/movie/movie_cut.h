//
// Created by potato on 2019-02-25.
//

#ifndef POTATO_VIDEO_NATIVE_MOVIE_CUT_H
#define POTATO_VIDEO_NATIVE_MOVIE_CUT_H

#include "../common/FmpegLib.h"
#include "../common/StreamInfo.h"
#include "../common/common_util.h"
#include <iostream>

int cut_video(double from_seconds, double end_seconds, const char *in_filename, const char *out_filename);



#endif //POTATO_VIDEO_NATIVE_MOVIE_CUT_H
