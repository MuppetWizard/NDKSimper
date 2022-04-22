//
// Created by ASUS on 2022/4/22.
//

#ifndef NDKSIMPLE_MACRO_H
#define NDKSIMPLE_MACRO_H

#include <android/log.h>
#define TAG "ykav_common"
// __VA_ARGS__ 代表 ...的可变参数
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG,  __VA_ARGS__);
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG,  __VA_ARGS__);
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG,  __VA_ARGS__);

//宏函数
#define DELETE(obj) if(obj){ delete obj; obj = 0; }

#endif //NDKSIMPLE_MACRO_H
