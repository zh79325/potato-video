//
// Created by potato on 2019-02-25.
//

#include "movie_concat.h"
#include <map>
using  namespace std;

int concat_video(const char *dest,const char *src1,const char *src2) {
    AVFormatContext *i_fmt_ctx=NULL;


    if (avformat_open_input(&i_fmt_ctx, src1, NULL, NULL) != 0) {
        fprintf(stderr, "could not open input file\n");
        return -1;
    }

    if (avformat_find_stream_info(i_fmt_ctx, 0) < 0) {
        fprintf(stderr, "could not find stream info\n");
        return -1;
    }


    AVFormatContext *o_fmt_ctx;

    avformat_alloc_output_context2(&o_fmt_ctx, NULL, NULL, dest);

    StreamInfo **output_streams=new StreamInfo*[i_fmt_ctx->nb_streams];
    map<AVMediaType,StreamInfo*> streamMap;
    for (int i = 0; i < i_fmt_ctx->nb_streams; i++) {
        AVStream *in_stream = i_fmt_ctx->streams[i];


        AVCodecParameters *in_codecpar = in_stream->codecpar;

        if (in_codecpar->codec_type != AVMEDIA_TYPE_AUDIO &&
            in_codecpar->codec_type != AVMEDIA_TYPE_VIDEO &&
            in_codecpar->codec_type != AVMEDIA_TYPE_SUBTITLE) {
            output_streams[i]=NULL;
            continue;
        }
        StreamInfo* info=new StreamInfo();
        output_streams[i]=info;
        streamMap[in_codecpar->codec_type]=info;
        AVStream *out_stream = avformat_new_stream(o_fmt_ctx, NULL);
        info->output=out_stream;
        if (!out_stream) {
            fprintf(stderr, "Failed allocating output stream\n");
            return AVERROR_UNKNOWN;
        }
        int ret = avcodec_parameters_copy(out_stream->codecpar, in_codecpar);
        if (ret < 0) {
            fprintf(stderr, "Failed to copy context from input to output stream codec context\n");
            return ret;
        }
        out_stream->codecpar->codec_tag = 0;
    }
    avio_open(&o_fmt_ctx->pb, dest, AVIO_FLAG_WRITE);
    /* yes! this is redundant */
    avformat_close_input(&i_fmt_ctx);
    avformat_write_header(o_fmt_ctx, NULL);

    char *files[2];
    files[0] = const_cast<char *>(src1);
    files[1] = const_cast<char *>(src2);
    for (int i = 0; i < 2; i++) {
        i_fmt_ctx = NULL;
        if (avformat_open_input(&i_fmt_ctx, files[i], NULL, NULL) != 0) {
            fprintf(stderr, "could not open input file\n");
            return -1;
        }

        if (avformat_find_stream_info(i_fmt_ctx, 0) < 0) {
            fprintf(stderr, "could not find stream info\n");
            return -1;
        }
        av_dump_format(i_fmt_ctx, 0, files[i], 0);


        AVPacket i_pkt;
        while (1) {

            av_init_packet(&i_pkt);
            i_pkt.size = 0;
            i_pkt.data = NULL;
            if (av_read_frame(i_fmt_ctx, &i_pkt) < 0)
                break;

            AVStream * input= i_fmt_ctx->streams[i_pkt.stream_index];
            AVMediaType type=input->codecpar->codec_type;
            if(!streamMap.count(type)){
                continue;
            }
            StreamInfo *info=streamMap[type];

            /*
             * pts and dts should increase monotonically
             * pts should be >= dts
             */
            i_pkt.flags |= AV_PKT_FLAG_KEY;
            info->pts = i_pkt.pts;
            i_pkt.pts =av_rescale_q_rnd(i_pkt.pts + info->start_pts, input->time_base,
                                        info->output->time_base, AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX);
            info->dts = i_pkt.dts;
            i_pkt.dts = av_rescale_q_rnd(i_pkt.dts +info->start_dts, input->time_base,
                                         info->output->time_base, AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX);;
            i_pkt.duration = av_rescale_q(i_pkt.duration,  input->time_base,  info->output->time_base);
            i_pkt.pos = -1;
            av_interleaved_write_frame(o_fmt_ctx, &i_pkt);
        }
        av_packet_unref(&i_pkt);

        for (std::pair<AVMediaType, StreamInfo*> element : streamMap) {
            StreamInfo* info = element.second;
            info->start_pts+=info->pts;
            info->pts=0;
            info->start_dts+=info->dts;
            info->dts=0;
        }

        avformat_close_input(&i_fmt_ctx);
    }

    av_write_trailer(o_fmt_ctx);

//    avcodec_close(o_fmt_ctx->streams[0]->codec);
//    av_freep(&o_fmt_ctx->streams[0]->codec);
//    av_freep(&o_fmt_ctx->streams[0]);

    avio_close(o_fmt_ctx->pb);
    av_free(o_fmt_ctx);
    return 0;
}
