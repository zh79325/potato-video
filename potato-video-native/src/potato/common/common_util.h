//
// Created by potato on 2019-02-25.
//

#ifndef POTATO_VIDEO_NATIVE_COMMON_UTIL_H
#define POTATO_VIDEO_NATIVE_COMMON_UTIL_H

#include "FmpegLib.h"

#include <iostream>

void show_video_info(char *filename);

void log_packet(const AVFormatContext *fmt_ctx, const AVPacket *pkt, const char *tag);

void printError(int ret);

#endif //POTATO_VIDEO_NATIVE_COMMON_UTIL_H
