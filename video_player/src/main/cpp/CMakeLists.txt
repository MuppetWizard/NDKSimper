cmake_minimum_required(VERSION 3.10.2)

#定义 ffmpeg，librtmp, player目录
set(FFMPEG ${CMAKE_SOURCE_DIR}/ffmpeg)
set(RTMP ${CMAKE_SOURCE_DIR}/librtmp)
set(PLAYER ${CMAKE_SOURCE_DIR}/player)
set(PUSHER ${CMAKE_SOURCE_DIR}/pusher)

#指定 ffmpeg 头文件
include_directories(${FFMPEG}/include)
include_directories(${RTMP}/include)

#指定 ffmpeg 静态库文件目录
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -L$(FFMPEG)/libs/${CMAKE_ANDROID_ARCH_ABI}")

#指定 rtmp 静态库文件目录
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -L$(RTMP)/libs/${CMAKE_ANDROID_ARCH_ABI}")

file(GLOB Player_CPP ${PLAYER}/*.cpp)
file(GLOB Pusher_CPP ${PUSHER}/*.cpp)

#添加自己编写 cpp 源文件生成动态库
add_library(PLAYER SHARED ${Player_CPP})
add_library(PUSHER SHARED ${Pusher_CPP})

find_library(
        log-lib
        log)

target_link_libraries(
        PLAYER
        PUSHER
        # 写了此命令不用在乎添加 ffmpeg lib 顺序问题导致应用崩溃
        -Wl,--start-group
        avcodec avfilter avformat avutil swresample swscale
        -Wl,--end-group
        z
        rtmp
        android
        #音频播放
        OpenSLES
        ${log_lib}
)