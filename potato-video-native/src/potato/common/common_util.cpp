//
// Created by potato on 2019-02-25.
//

#include "common_util.h"

void show_video_info(char *filename){
    AVFormatContext *context=NULL;
    avformat_open_input(&context, filename, 0, 0);
    av_dump_format(context, 0, filename, 0);
    avformat_close_input(&context);
}

void log_packet(const AVFormatContext *fmt_ctx, const AVPacket *pkt, const char *tag) {
    AVRational *time_base = &fmt_ctx->streams[pkt->stream_index]->time_base;

    printf("%s: pts:%s pts_time:%s dts:%s dts_time:%s duration:%s duration_time:%s stream_index:%d\n",
           tag,
           av_ts2str(pkt->pts), av_ts2timestr(pkt->pts, time_base),
           av_ts2str(pkt->dts), av_ts2timestr(pkt->dts, time_base),
           av_ts2str(pkt->duration), av_ts2timestr(pkt->duration, time_base),
           pkt->stream_index);
}

void printError(int ret) {
    char err[1000];
    av_strerror(ret,err,1000);
//    printf(err);
    av_log(NULL, AV_LOG_ERROR, err);
    av_log(NULL, AV_LOG_ERROR, "\n");
}