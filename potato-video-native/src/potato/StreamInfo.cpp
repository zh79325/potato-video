//
// Created by potato on 2019-02-24.
//

#include "StreamInfo.h"

MovieInfo::MovieInfo(AVStream *stream) {

    this->width=stream->codecpar->width;
    this->height=stream->codecpar->height;
    this->format=stream->codec->pix_fmt;


}

bool MovieInfo::same_size(MovieInfo *movieInfo) {
    return width==movieInfo->width&&height==movieInfo->height;
}

void MovieInfo::init_img_buf(int align) {
    av_image_alloc(data, linesize,
                   width, height, format, align);
}
