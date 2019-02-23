cmake_minimum_required(VERSION 3.13)
project(potato_video_native)

set(CMAKE_CXX_STANDARD 14)


include_directories(/usr/local/Cellar/ffmpeg/4.1.1/include/)
link_directories(/usr/local/Cellar/ffmpeg/4.1.1/lib/)

file(GLOB_RECURSE fmpeg_extend
        "src/fmpeg/*.h"
        "src/fmpeg/*.cpp"
        "src/fmpeg/*.c"
        )

set(LIB_SOURCE_FILES src/library.cpp
        src/library.h
        ${fmpeg_extend})
add_library(potato_video_native ${LIB_SOURCE_FILES})


set(DEMO_SOURCE_FILES src/main.cpp
        ${LIB_SOURCE_FILES})

add_executable(FfmpegDemo ${DEMO_SOURCE_FILES})

target_link_libraries(
        potato_video_native
        avcodec
        avdevice
        avfilter
        avformat
        avresample
        avutil
        postproc
        swresample
        swscale
)
target_link_libraries(FfmpegDemo potato_video_native)