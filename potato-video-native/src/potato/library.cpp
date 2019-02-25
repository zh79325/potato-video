#include "library.h"
#include <map>

using namespace std;

int64_t all(AVRounding a, AVRounding b) {
    return static_cast<int64_t>(static_cast<int>(a) | static_cast<int>(b));
}


int internalProcess(double from_seconds, double end_seconds, AVOutputFormat *pFormat, AVFormatContext *pContext,
                    AVFormatContext *pFormatContext, const char *in_filename, const char *out_filename);

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

int cut_video(double from_seconds, double end_seconds, const char *in_filename, const char *out_filename) {
    AVOutputFormat *ofmt = NULL;
    AVFormatContext *ifmt_ctx = NULL, *ofmt_ctx = NULL;
    int ret = internalProcess(from_seconds, end_seconds, ofmt, ifmt_ctx, ofmt_ctx, in_filename, out_filename);

    avformat_close_input(&ifmt_ctx);

    /* close output */
    if (ofmt_ctx && !(ofmt->flags & AVFMT_NOFILE))
        avio_closep(&ofmt_ctx->pb);
    avformat_free_context(ofmt_ctx);

    if (ret < 0 && ret != AVERROR_EOF) {
        fprintf(stderr, "Error occurred: %s\n", av_err2str(ret));
        return 1;
    }

    return 0;
}

int concat_video(const char *dest, char *src1, char *src2) {
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
    files[0] = src1;
    files[1] = src2;
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

int internalProcess(double from_seconds, double end_seconds, AVOutputFormat *ofmt, AVFormatContext *ifmt_ctx,
                    AVFormatContext *ofmt_ctx, const char *in_filename, const char *out_filename) {
    AVPacket pkt;
    int ret, i;
    int stream_index = 0;
    int *stream_mapping = NULL;
    int stream_mapping_size = 0;
    if ((ret = avformat_open_input(&ifmt_ctx, in_filename, 0, 0)) < 0) {
        fprintf(stderr, "Could not open input file '%s'", in_filename);
        return ret;
    }

    if ((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0) {
        fprintf(stderr, "Failed to retrieve input stream information");
        return ret;
    }

    av_dump_format(ifmt_ctx, 0, in_filename, 0);

    avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, out_filename);
    if (!ofmt_ctx) {
        fprintf(stderr, "Could not create output context\n");
        ret = AVERROR_UNKNOWN;
        return ret;
    }
    stream_mapping_size = ifmt_ctx->nb_streams;
    stream_mapping = static_cast<int *>(av_mallocz_array(stream_mapping_size, sizeof(*stream_mapping)));
    if (!stream_mapping) {
        ret = AVERROR(ENOMEM);
        return ret;
    }

    ofmt = ofmt_ctx->oformat;

    for (i = 0; i < ifmt_ctx->nb_streams; i++) {
        AVStream *in_stream = ifmt_ctx->streams[i];
        AVCodecParameters *in_codecpar = in_stream->codecpar;

        if (in_codecpar->codec_type != AVMEDIA_TYPE_AUDIO &&
            in_codecpar->codec_type != AVMEDIA_TYPE_VIDEO &&
            in_codecpar->codec_type != AVMEDIA_TYPE_SUBTITLE) {
            stream_mapping[i] = -1;
            continue;
        }

        stream_mapping[i] = stream_index++;

        AVStream *out_stream = avformat_new_stream(ofmt_ctx, NULL);
        if (!out_stream) {
            fprintf(stderr, "Failed allocating output stream\n");
            ret = AVERROR_UNKNOWN;
            return ret;
        }

        ret = avcodec_parameters_copy(out_stream->codecpar, in_codecpar);
        if (ret < 0) {
            fprintf(stderr, "Failed to copy context from input to output stream codec context\n");
            return ret;
        }
        out_stream->codecpar->codec_tag = 0;
    }
    av_dump_format(ofmt_ctx, 0, out_filename, 1);

    if (!(ofmt->flags & AVFMT_NOFILE)) {
        ret = avio_open(&ofmt_ctx->pb, out_filename, AVIO_FLAG_WRITE);
        if (ret < 0) {
            fprintf(stderr, "Could not open output file '%s'", out_filename);
            return ret;
        }
    }

    ret = avformat_write_header(ofmt_ctx, NULL);
    if (ret < 0) {
        fprintf(stderr, "Error occurred when opening output file\n");
        return ret;
    }

    //    int indexs[8] = {0};


    //    int64_t start_from = 8*AV_TIME_BASE;
    ret = av_seek_frame(ifmt_ctx, -1, from_seconds * AV_TIME_BASE+ifmt_ctx->start_time, AVSEEK_FLAG_FRAME|AVSEEK_FLAG_BACKWARD);
    if (ret < 0) {
        fprintf(stderr, "Error seek\n");
        return ret;
    }

    int n=ofmt_ctx->nb_streams;
    int64_t max=std::numeric_limits<int64_t>::max();
    int64_t *dts_start_from = new int64_t[n];
    for (int j = 0; j < n; ++j) {
        dts_start_from[j]=max;
    }
    int64_t *pts_start_from = new int64_t[n];
    for (int j = 0; j < n; ++j) {
        pts_start_from[j]=max;
    }

    while (1) {
        AVStream *in_stream, *out_stream;

        ret = av_read_frame(ifmt_ctx, &pkt);
        if (ret < 0)
            break;

        int o = stream_mapping[pkt.stream_index];
        if (o < 0) {
            continue;
        }
        in_stream = ifmt_ctx->streams[pkt.stream_index];
        pkt.stream_index=o;
        out_stream = ofmt_ctx->streams[o];

        double current_time=av_q2d(in_stream->time_base) * pkt.pts;
//        if(pkt.pts<0||pkt.dts<0){
//            continue;
//        }
//        if(current_time<0){
//            continue;
//        }
        if (current_time > end_seconds) {
            av_packet_unref(&pkt);
            break;
        }

        if (dts_start_from[pkt.stream_index] == max) {
            dts_start_from[pkt.stream_index] = pkt.dts;
            printf("dts_start_from: %s\n", av_ts2str(dts_start_from[pkt.stream_index]));
        }
        if (pts_start_from[pkt.stream_index] == max) {
            pts_start_from[pkt.stream_index] = pkt.pts;
            printf("pts_start_from: %s\n", av_ts2str(pts_start_from[pkt.stream_index]));
        }

        log_packet(ifmt_ctx, &pkt, "in");

        /* copy packet */
        pkt.pts = av_rescale_q_rnd(pkt.pts - pts_start_from[pkt.stream_index], in_stream->time_base,
                                   out_stream->time_base, AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX);
        pkt.dts = av_rescale_q_rnd(pkt.dts - dts_start_from[pkt.stream_index], in_stream->time_base,
                                   out_stream->time_base, AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX);
        pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
        pkt.pos = -1;
        if(pkt.pts < pkt.dts) {
            continue;
        }
        log_packet(ofmt_ctx, &pkt, "out");
        printf("\n");

        ret = av_interleaved_write_frame(ofmt_ctx, &pkt);
        if (ret < 0) {
            fprintf(stderr, "Error muxing packet\n");
            break;
        }
        av_packet_unref(&pkt);
    }
    free(dts_start_from);
    free(pts_start_from);

    av_write_trailer(ofmt_ctx);


    return ret;
}