
project(${CMAKE_PROJECT_NAME}_lib)



set(FFMPEG_SOURCE_FILES ${fmpeg_src})
file(GLOB_RECURSE fmpeg_extend
        "fmpeg/*.h"
        "fmpeg/*.cpp"
        "fmpeg/*.c"
        )
file(GLOB_RECURSE potato_source
        "potato/*.h"
        "potato/*.cpp"
        "potato/*.c"
        )
set(LIB_SOURCE_FILES ${potato_source}
        ${fmpeg_extend})
add_library(${CMAKE_PROJECT_NAME}_lib ${LIB_SOURCE_FILES})





target_link_libraries(
        ${CMAKE_PROJECT_NAME}_lib
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



