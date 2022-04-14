cmake_minimum_required(VERSION 3.10.2)

#视频渲染
set(OpenGL ${CMAKE_SOURCE_DIR}/gles)

#批量添加自己编写的 cpp 文件,不要把 *.h 加入进来了
file(GLOB ALL_CPP ${OpenGL}/*.cpp)

add_library(video SHARED ${ALL_CPP})

find_library(log-lib
        log)

target_link_libraries( # Specifies the target library.
        video
        # OpenGL 与 NativeWindow 连接本地窗口的中间者
        EGL
        #视频渲染
        GLESv2
        #添加本地库
        android

        ${log-lib})