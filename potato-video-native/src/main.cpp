//
// Created by potato on 2019-02-23.
//
#include <iostream>
#include "library.h"
#include <string>

using namespace std;

int main() {
    string resourceFolder="/Users/zh_zhou/Desktop/video/resources/";
    string movie = (resourceFolder+"dy2.mp4");
    char *output = "/Users/zh_zhou/Desktop/video/out.mp4";
    int durition = 5;
    int n=10;
    char *files[n];
    for (int i = 1; i < 2; ++i) {
        double start = i * durition;
        double end = start + durition;
        files[i]=new char[100];
        sprintf(files[i], "/Users/zh_zhou/Desktop/video/out/%d.mp4", i);
        int n = cut_video(start, end, movie.c_str(), files[i]);
    }
    char *mix = "/Users/zh_zhou/Desktop/video/out/mix.mp4";
    concat_video(mix,files[0],files[1]);
    return 0;
}

