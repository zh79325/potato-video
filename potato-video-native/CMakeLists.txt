cmake_minimum_required(VERSION 3.13)
project(potato_video_native)


include_directories(/usr/local/Cellar/ffmpeg/4.1.1/include/)
link_directories(/usr/local/Cellar/ffmpeg/4.1.1/lib/)


set(CMAKE_CXX_STANDARD 14)

set(SOURCE_FILES main.cpp)

add_executable(demo ${SOURCE_FILES})



add_subdirectory(src)
add_subdirectory(tests)

target_link_libraries(demo potato_video_native_lib)