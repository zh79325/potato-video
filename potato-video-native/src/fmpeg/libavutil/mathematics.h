

#ifndef AVUTIL_MATHEMATICS_H_EXTEND
#define AVUTIL_MATHEMATICS_H_EXTEND



extern "C" {
#include <stdint.h>
#include <limits.h>
#include <libavutil/avassert.h>
}

int64_t av_rescale_q_rnd(int64_t a, AVRational bq, AVRational cq,
                          int64_t rnd) av_const;

#endif /* AVUTIL_MATHEMATICS_H */
