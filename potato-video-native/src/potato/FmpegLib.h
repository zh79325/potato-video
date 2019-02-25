//
// Created by potato on 2019-02-24.
//

#ifndef POTATO_VIDEO_NATIVE_FMPEGLIB_H
#define POTATO_VIDEO_NATIVE_FMPEGLIB_H

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/mathematics.h>
#include <libavformat/avformat.h>
#include <libavutil/timestamp.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <libavutil/parseutils.h>
#include <libswscale/swscale.h>
}
#endif //POTATO_VIDEO_NATIVE_FMPEGLIB_H
