//
// Created by potato on 2019-02-25.
//

#include "gtest/gtest.h"
#include <iostream>
#include "../../src/potato/library.h"
#include <string>

using namespace std;
string resourceFolder="/Users/zh_zhou/Desktop/video/resources/";
string getMovie(string name){
    return resourceFolder+name;
}
TEST(Movie, cut) {

    string movie = getMovie("mv.mp4");
    char *output = "/Users/zh_zhou/Desktop/video/out.mp4";
    int durition = 5;
    int n=10;
    char *files[n];
    for (int i = 0; i < 5; ++i) {
        double start = i * durition;
        double end = start + durition;
        files[i]=new char[100];
        sprintf(files[i], "/Users/zh_zhou/Desktop/video/out/%d.mp4", i);
        int n = cut_video(start, end, movie.c_str(), files[i]);
    }

}

TEST(Movie, concat){
    char *mix = "/Users/zh_zhou/Desktop/video/out/mix.mp4";
    string *files=new string [2]{
            "/Users/zh_zhou/Desktop/video/out/0.mp4",
            "/Users/zh_zhou/Desktop/video/out/1.mp4"
    };
    concat_video(mix,files[0].c_str(),files[1].c_str());
}

TEST(Movie, scale){
    string movie = getMovie("dy.mp4");
    string movieOut = getMovie("mv.out.mp4");
    string img = getMovie("../my_logo.png");
    watermark_movie(movie.c_str(),movieOut.c_str(),img.c_str());
}

TEST(Movie,watermark){
    string movie = getMovie("mv.mp4");
    add_water_mark(movie.c_str(),NULL);
}