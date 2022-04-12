//
// Created by ASUS on 2022/4/12.
//

#ifndef NDKSIMPLE_AUDIOENGINE_H
#define NDKSIMPLE_AUDIOENGINE_H

#include <SLES/OpenSLES.h>
#include <stdio.h>
#include <SLES/OpenSLES_Android.h>
#include <assert.h>
#include <android/log.h>

class AudioEngine{
public:
    SLObjectItf engineObj;
    SLEngineItf engine;

    SLObjectItf outputMixObj;

private:
    /*
     * 第一步创建音频引擎并获取引擎接口
     */
    void createEngine() {
        // 1.1、创建引擎对象 SLObjectItf engineObj
        SLresult result = slCreateEngine(&engineObj,0,NULL,0,NULL,NULL);
        if (result != SL_RESULT_SUCCESS) {
            return;
        }

        //1.2、初始化引擎
        result = (*engineObj) ->Realize(engineObj,SL_BOOLEAN_FALSE);
        if (result != SL_BOOLEAN_FALSE) {
            return;
        }

        //1.3、获取引擎接口 SLEngineItf engineInterface
        result = (*engineObj) ->GetInterface(engineObj,SL_IID_ENGINE,&engine);
        if (result != SL_RESULT_SUCCESS) {
            return;
        }
    }

    /*
     * 第二步，创建混音器
     */
    void createMixer(){
        //2.1、创建混音器 SLObjectItf outputMixObj
        SLresult result = (*engine) -> CreateOutputMix(engine,&outputMixObj,0,0,0);
        if (result != SL_RESULT_SUCCESS) {
            return;
        }

        //2.2、初始化混音器
        result = (*outputMixObj) ->Realize(outputMixObj,SL_BOOLEAN_FALSE);
        if (result != SL_BOOLEAN_FALSE) {
            return;
        }
    }

    virtual void release(){
        if (outputMixObj){
            (*outputMixObj) ->Destroy(outputMixObj);
            outputMixObj = nullptr;
        }
        if (engineObj){
            (*engineObj) ->Destroy(engineObj);
            engineObj = nullptr;
            engine = nullptr;
        }
    }

public:
    AudioEngine() : engineObj(nullptr),engine(nullptr),outputMixObj(nullptr){
        createEngine();
        createMixer();
    }
    virtual ~AudioEngine(){
        release();
    }
};


#endif //NDKSIMPLE_AUDIOENGINE_H
