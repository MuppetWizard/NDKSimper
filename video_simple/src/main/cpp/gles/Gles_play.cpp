//
// Created by ASUS on 2022/4/14.
//

#include <pthread.h>
#include "Gles_play.h"

EGLDisplay display;
ANativeWindow *nativeWindow = 0;
EGLSurface winSurface;
EGLContext context;
int width = 640;
int height = 272;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // 静态初始化 互斥锁

//顶点着色器，每个定点执行一次，可并行执行
#define GET_STR(x) #x
static const char *vertexShader = GET_STR(
        attribute
        vec4 aPosition; //输入的顶点坐标，会在程序指定将数据输入到该字段
        attribute
        vec2 aTextCoord; //输入的纹理坐标，会在程序指定将数据输入到该字段
        varying
        vec2 vTextCoord; //输出的纹理坐标
        void main(){
            //这里其实是将上下翻转过来（因为安卓图片会自动上下翻转，所以转回来）
            vTextCoord = vec2(aTextCoord.x, 1.0 - aTextCoord.y);
            //直接把传入的坐标值作为传入渲染管线。gl_Position是OpenGL内置的
            gl_Position = aPosition;
        }
);

//图元被光栅化为多少片段，就被调用多少次
static const char *fragYUV20P = GET_STR(
        precision
        mediump float;
        varying
        vac2 vTextCoord;
        //输入的三个 yuv 纹理
        uniform
        sampler2D yTexture;//采样器
        uniform
        sampler2D uTexture;//采样器
        uniform
        sampler2D vTexture;//采样器
        void main() {
            vec3 yuv;
            vec3 rgb;
            //分别取yuv各个分量的采样纹理（r表示？）
            yuv.x = texture2D(yTexture, vTextCoord).g;
            yuv.y = texture2D(uTexture, vTextCoord).g - 0.5;
            yuv.z = texture2D(vTexture, vTextCoord).g - 0.5;
            rgb = mat3(
                    1.0, 1.0, 1.0,
                    0.0, -0.39465, 2.03211,
                    1.13983, -0.5806, 0.0
            ) * yuv;
            //gl_FragColor是OpenGL内置的
            gl_FragColor = vec4(rgb, 1.0);
        }
);

GLint initShader(const char *source, int type);

GLint initShader(const char *source, int type){
    //创建 shader
    GLint sh = glCreateShader(type);
    if (sh == 0){
        LOGD("gl CreateShader %d failed",type);
        return 0;
    }
    //加载shader
    glShaderSource(sh,
                   1, //shader数量
                   &source,
                   0 //代码长度，传0则读到字符串结尾
    );

    //编译 shader
    glCompileShader(sh);

    GLint status;
    glGetShaderiv(sh,GL_COMPILE_STATUS,&status);
    if (status == 0) {
        LOGD("gl CompileShader %d failed",type);
        LOGD("source %d",source);
        return 0;
    }

    LOGD("gl CompileShader %d success",type);
    return sh;

}

void Gles_play::playYUV(jobject surface) {
    //加锁
    pthread_mutex_lock(&mutex);
    //判断资源是否被释放
    release();

    showMessage(env,"start", true);

    //开始播放标志
    isPlay = true;

    //1、获取原始窗口
    nativeWindow = ANativeWindow_fromSurface(env,surface);

    //获取 Display
    display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

    if (display == EGL_NO_DISPLAY) {
        LOGD("egl display failed");
        showMessage(env,"egl display failed", false);
        return;
    }

    //初始化egl, 后两个参数为主次版本号
    if (EGL_TRUE != eglInitialize(display,0,0)) {
        LOGD("egl Initialize failed");
        showMessage(env, "egl Initialize failed", false);
        return;
    }

    //3.1 surface 配置
    EGLConfig eglConfig;
    EGLint configNum;
    EGLint configSpec[] = {
            EGL_RED_SIZE,8,
            EGL_GREEN_SIZE,8,
            EGL_BLUE_SIZE,8,
            EGL_SURFACE_TYPE,EGL_WINDOW_BIT,
            EGL_NONE
    };

    if (EGL_TRUE != eglChooseConfig(display, configSpec, &eglConfig, 1, &configNum)) {
        LOGD("egl ChooseConfig failed");
        showMessage(env,"egl ChooseConfig failed", false);
        return;
    }
    //3.2、创建surface 和nativeWindow进行关联, 最后一个参数为属性信息，0表示使用默认版本
    winSurface = eglCreateWindowSurface(display,eglConfig,nativeWindow,0);
    if (winSurface == EGL_NO_SURFACE){
        LOGD("egl CreateWindowSurface failed");
        showMessage(env,"egl createWindowSurface failed", false);
        return;
    }


    //4 创建关联上下文
    const EGLint ctxAttr[] = {
            EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE
    };

    //EGL_NO_CONTEXT 表示不需要多个设备共享上下文
    context = eglCreateContext(display,eglConfig,EGL_NO_CONTEXT,ctxAttr);
    if (context == EGL_NO_CONTEXT){
        LOGD("egl CreateContext failed");
        showMessage(env,"egl CreateContext failed",false);
        return;
    }

    //将egl和opengl关联
    //两个surface一个读一个写。第二个一般用来离线渲染？
    if (EGL_TRUE != eglMakeCurrent(display,winSurface,winSurface,context)) {
        LOGD("egl  MakeCurrent failed");
        showMessage(env,"egl MakeCurrent failed", false);
        return;
    }

    GLint vsh = initShader(vertexShader,GL_VERTEX_SHADER);
    GLint fsh = initShader(fragYUV20P,GL_FRAGMENT_SHADER);

    //创建渲染程序
    GLint program = glCreateProgram();
    if (program == 0) {
        LOGD("gl CreateProgram failed");
        showMessage(env,"gl CreateProgram failed", false);
        return;
    }

    //向渲染程序中加入着色器
    glAttachShader(program,vsh);
    glAttachShader(program,fsh);

    //链接程序
    glLinkProgram(program);

}

void Gles_play::start() {

}

void Gles_play::progress() {

}

void Gles_play::release() {

}



Gles_play::Gles_play(JNIEnv *env, jobject thiz, PlayCallback *playCallback, const char *data_source,
                     jobject surface) {
    this->playCallback = playCallback;
    this->env = env;
    this->thiz = thiz;
    this->surface = surface;
    this->data_source = new char[strlen(data_source) + 1];
    strcpy(this->data_source,data_source);
}

Gles_play::~Gles_play() {
    if (playCallback) {
        delete playCallback;
        playCallback = 0;
    }
}