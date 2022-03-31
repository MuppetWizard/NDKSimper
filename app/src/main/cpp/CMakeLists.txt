# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.
#1. 指定 cmake 的最小版本
cmake_minimum_required(VERSION 3.10.2)

# Declares and names the project.
#2. 设置项目名称
project("ndksimple")

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

# 打印日志
#message("当前CMake的路径是：${CMAKE_SOURCE_DIR}")
#message("当前 CMAKE_ANDROID_ARCH_ABI 的路径是：${CMAKE_ANDROID_ARCH_ABI}")

# 批量引入源文件
#file(GLOB allCpp *.cpp)

#生成可执行文件
#add_executable(demo native-lib.cpp)

## 生成动态库或共享库
add_library( # Sets the name of the library.
        ndksimple

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        native-lib.cpp)

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

#查找指定库文件
find_library( # Sets the name of the path variable.
        log-lib

        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log)

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

# 设置 target 需要链接的库
target_link_libraries( # Specifies the target library.
        ndksimple

        # Links the target library to the log library
        # included in the NDK.
        ${log-lib})