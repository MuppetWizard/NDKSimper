cmake_minimum_required(VERSION 3.10.2)

# 打印日志
message(“AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA>>>”)
message(”当前CMake的路径是：${CMAKE_SOURCE_DIR}“)
message(“当前 CMAKE_ANDROID_ARCH_ABI 的路径是：${CMAKE_ANDROID_ARCH_ABI}”)

# 批量引入源文件
file(GLOB allCpp *.cpp)



# 导入静态库
#add_library(test_a STATIC IMPORTED)
# 开始真正的导入
#set_target_properties(test_a PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/libtest.a)

#导入动态库
add_library(test_so SHARED IMPORTED)
set_target_properties(test_so PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/../jin/${CMAKE_ANDROID_ARCH_ABI}/libTest.so)

# 加入cpp源文件
add_library(
        native-lib
        SHARED
        #        native-lib.cpp #替换  ${allCpp} 批量导入文件
        ${allCpp}
)

# 只能找系统的
find_library(
        log-lib
        log)

message(“当前的log路径在哪里啊 >>>>>>>>>>>>>>>>> ${log-lib}”)

#开始链接指定的库
target_link_libraries(
        native-lib
        test_so
        ${log-lib}

        #        test_a
)

