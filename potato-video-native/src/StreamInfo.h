//
// Created by potato on 2019-02-24.
//

#ifndef POTATO_VIDEO_NATIVE_STREAMINFO_H
#define POTATO_VIDEO_NATIVE_STREAMINFO_H

#include "FmpegLib.h"

struct StreamInfo {
    AVStream *output;
    int64_t start_dts;
    int64_t start_pts;
    bool active;


    int64_t dts;
    int64_t pts;
    StreamInfo(){
        dts=0;
        pts=0;
        start_dts=0;
        start_pts=0;
        active=false;
        output=NULL;
    }
};


#endif //POTATO_VIDEO_NATIVE_STREAMINFO_H
