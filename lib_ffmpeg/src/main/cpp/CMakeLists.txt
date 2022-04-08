cmake_minimum_required(VERSION 3.10.2)

# 打印日志
message(“AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA>>>”)

# 批量引入源文件
file(GLOB allCpp *.cpp)

#打印当前 cmakeLists 的路径
message("当前cmakel路径: ${CMAKE_SOURCE_DIR} \n cpu架构：${CMAKE_ANDROID_ARCH_ABI}")

# 引入FFmpeg的头文件
include_directories(${CMAKE_SOURCE_DIR}/../jni/include)
#添加共享库搜索路径
LINK_DIRECTORIES(${CMAKE_SOURCE_DIR}/../jni/arm64-v8a)


add_library(ffmpeg_lib SHARED ${allCpp})

find_library(log-lib log)

# 引入FFmpeg的库文件，设置内部的方式引入，指定库的目录是 -L  指定具体的库-l
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -L src/main/jni/${CMAKE_ANDROID_ARCH_ABI}")

target_link_libraries(
        ffmpeg_lib
        avformat avcodec avfilter avutil postproc swresample swscale
        ${log-lib}
)


