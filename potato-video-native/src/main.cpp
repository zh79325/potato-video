//
// Created by potato on 2019-02-23.
//
#include <iostream>
#include "library.h"


using namespace std;

int main() {
    char *input = "/Users/zh_zhou/Desktop/video/402410374.mp4";
    char *output = "/Users/zh_zhou/Desktop/video/out.mp4";
    int durition = 5;
    int n=10;
    char *files[n];
    for (int i = 0; i < 10; ++i) {
        double start = i * durition;
        double end = start + durition;
        files[i]=new char[100];
        sprintf(files[i], "/Users/zh_zhou/Desktop/video/out/%d.mp4", i);
        int n = cut_video(start, end, input, files[i]);
    }
    char *mix = "/Users/zh_zhou/Desktop/video/out/mix.mp4";
    concat_video(mix,files[1],files[1]);
    return 0;
}

