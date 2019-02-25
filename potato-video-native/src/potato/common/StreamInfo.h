//
// Created by potato on 2019-02-24.
//

#ifndef POTATO_VIDEO_NATIVE_STREAMINFO_H
#define POTATO_VIDEO_NATIVE_STREAMINFO_H

#include "FmpegLib.h"

struct MovieInfo {
    MovieInfo(AVStream *pStream);

    int width;
    int height;
    AVPixelFormat format;
    uint8_t *data[4];
    int linesize[4];
    bool same_size(MovieInfo *pInfo);
    void init_img_buf(int align);
    ~MovieInfo(){
        av_freep(&data[0]);
    }
};

struct StreamInfo {


    AVStream *output;
    MovieInfo *movieInfo;
    int64_t start_dts;
    int64_t start_pts;
    bool active;


    int64_t dts;
    int64_t pts;

    StreamInfo() {
        dts = 0;
        pts = 0;
        start_dts = 0;
        start_pts = 0;
        active = false;
        output = NULL;
        movieInfo=NULL;
    }

    ~StreamInfo() {
        if (movieInfo != NULL) {
            delete movieInfo;
        }
    }
};


#endif //POTATO_VIDEO_NATIVE_STREAMINFO_H
