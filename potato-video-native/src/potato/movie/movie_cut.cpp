//
// Created by potato on 2019-02-25.
//
extern "C" {
#include <libavutil/mathematics.h>
}
#include "movie_cut.h"


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
