cmake_minimum_required(VERSION 3.10.2)

# 打印日志
message(“AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA>>>”)

#音频渲染
set(OpenSL ${CMAKE_SOURCE_DIR}/opensl)

# 批量引入源文件
file(GLOB ALL_Cpp ${OpenSL}/*.cpp)

#打印当前 cmakeLists 的路径
message("当前cmakel路径: ${CMAKE_SOURCE_DIR} \n cpu架构：${CMAKE_ANDROID_ARCH_ABI}")

add_library(audio-simple SHARED ${ALL_Cpp})

find_library(log-lib log)

target_link_libraries(
        audio-simple
        #音频渲染
#        OpenSLES
        #添加本地库
        android

        ${log-lib}
)