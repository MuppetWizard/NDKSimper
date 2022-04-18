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

/*
 * 播放 YUV 线程
 */
void *readYUVThread(void *pVoid){
    Gles_play *gles_play = static_cast<Gles_play *>(pVoid);
    gles_play->start();
    return 0;
}

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

    GLint status = 0;
    glGetProgramiv(program,GL_LINK_STATUS,&status);
    if (status == 0) {
        LOGD("gl glLinkProgram failed");
        showMessage(env,"gl glLinkProgram failed", false);
        return;
    }

    LOGD("glLinkProgram success");
    //激活渲染程序
    glUseProgram(program);

    //加入三维顶点数据
    static float ver[] = {
            1.0f, -1.0f, 0.0f,
            -1.0f, -1.0f, 0.0f,
            1.0f, 1.0f, 0.0f,
            -1.0f, 1.0f, 0.0f
    };

    GLuint apos = static_cast<GLuint>(glGetAttribLocation(program,"mPosition"));
    glEnableVertexAttribArray(apos);
    glVertexAttribPointer(apos,3,GL_FLOAT,GL_FALSE,0,ver);

    //加入纹理坐标数据
    static float fragment[] = {
            1.0f, 0.0f,
            0.0f, 0.0f,
            1.0f, 1.0f,
            0.0f, 1.0f
    };
    GLuint aTex = static_cast<GLuint>(glGetAttribLocation(program,"aTextCoord"));
    glEnableVertexAttribArray(aTex);
    glVertexAttribPointer(aTex,2,GL_FLOAT,GL_FALSE,0,fragment);

    /**
   *  //获取一致变量的存储位置
   * GLint textureUniformY = glGetUniformLocation(program, "SamplerY");
   * GLint textureUniformU = glGetUniformLocation(program, "SamplerU");
   * GLint textureUniformV = glGetUniformLocation(program, "SamplerV");
   * //对几个纹理采样器变量进行设置
   * glUniform1i(textureUniformY, 0);
   * glUniform1i(textureUniformU, 1);
   * glUniform1i(textureUniformV, 2);
   */

    //对sampler 变量，使用函数glUniform1i和glUniform1iv进行设置
    glUniform1i(glGetUniformLocation(program,"yTexture"),0);
    glUniform1i(glGetUniformLocation(program,"uTexture"),0);
    glUniform1i(glGetUniformLocation(program,"vTexture"),0);
    //纹理id
    GLuint texts[3] = {0};
    //创建若干个纹理对象，并且得到纹理id
    glGenTextures(3,texts);

    //绑定纹理，后面的设置和加载全部用于当前绑定的对象
    //GL_TEXTURE0、GL_TEXTURE1、GL_TEXTURE2 的就是纹理单元，GL_TEXTURE_1D、GL_TEXTURE_2D、CUBE_MAP为纹理目标
    //通过 glBindTexture 函数将纹理目标和纹理绑定后，对纹理目标所进行的操作都反映到纹理上
    glBindTexture(GL_TEXTURE_2D,texts[0]);
    //缩小的过滤器
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    //设置纹理的格式和大小
    // 加载纹理到 OpenGL，读入 buffer 定义的位图数据，并把它复制到当前绑定的纹理对象
    // 当前绑定的纹理对象就会被附加上纹理图像。
    //width, height 表示每几个像素公用一个yuv元素？比如width / 2表示横向每两个像素使用一个元素？
    glTexImage2D(GL_TEXTURE_2D,
                 0, //细节基本 默认0
                 GL_LUMINANCE,//gpu内部格式，亮度 灰度图（这里指只取一个亮度的颜色通道的意思）
                 width, //加载的纹理宽度。最好为2的次幂(这里对y分量数据当做指定尺寸算，但显示尺寸会拉伸到全屏？)
                 height, //加载的纹理高度。最好为2的次幂
                 0,//纹理边框
                 GL_LUMINANCE,//数据的像素格式 亮度，灰度图
                 GL_UNSIGNED_BYTE,//像素点存储的数据类型
                 NULL //纹理数据 先不传
                 );

    unsigned char *buf[3] = {0};
    buf[0] = new unsigned char [width * height]; //y
    buf[1] = new unsigned char [width * height / 4]; //u
    buf[2] = new unsigned char [width * height / 4]; //v

    showMessage(env, "onSucceed", true);

    FILE *fp = fopen(data_source, "r6");

    if (!fp) {
        LOGD("open file %s failed",data_source);
        return;
    }

    //读取视频yuv数据
    while (!feof(fp)) {
        //解决异常退出，终止读取数据
        if (!isPlay)
            return;
        fread(buf[0],1,width * height, fp);
        fread(buf[1],1,width * height / 4,fp);
        fread(buf[2],1,width * height / 4,fp);

        //激活第一层纹理，绑定到创建的纹理
        //下面的 width 和 height 主要是显示尺寸？
        glActiveTexture(GL_TEXTURE0);
        //绑定y对应的纹理
        glBindTexture(GL_TEXTURE_2D,texts[0]);
        //替换纹理 比重新使用glTexImage20性能高很多
        glTexSubImage2D(GL_TEXTURE_2D,0,
                        0,0, //相对原来的纹理的offset
                        width, height,//加载的纹理宽度和高度，最好为2的次幂
                        GL_LUMINANCE, GL_UNSIGNED_BYTE,
                        buf[0]
                        );

        //激活第二层纹理，绑定到创建的纹理
        glActiveTexture(GL_TEXTURE1);
        //绑定u对应的纹理
        glBindTexture(GL_TEXTURE_2D, texts[1]);
        //替换纹理，比重新使用glTexImage2D性能高
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width / 2, height / 2, GL_LUMINANCE,
                        GL_UNSIGNED_BYTE,
                        buf[1]);

        //激活第三层纹理，绑定到创建的纹理
        glActiveTexture(GL_TEXTURE2);
        //绑定v对应的纹理
        glBindTexture(GL_TEXTURE_2D, texts[2]);
        //替换纹理，比重新使用glTexImage2D性能高
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width / 2, height / 2, GL_LUMINANCE,
                        GL_UNSIGNED_BYTE,
                        buf[2]);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        //窗口显示，交换双缓冲区
        eglSwapBuffers(display, winSurface);
    }
    showMessage(env, "onComplete", true);
    release();
    isPlay = false;
    //释放锁
    pthread_mutex_unlock(&mutex);
    
}

void Gles_play::start() {
    this->playYUV(this->surface);
}

void Gles_play::prepare() {
    pthread_create(&pid_prepare, 0, readYUVThread,this);
}

void Gles_play::release() {
    if (display || winSurface || context) {
        //销毁显示设备
        eglDestroySurface(display,winSurface);
        //销魂上下文
        eglDestroyContext(display,context);
        //释放窗口
        ANativeWindow_release(nativeWindow);
        //释放线程
        eglReleaseThread();
        //停止
        eglTerminate(display);
        eglMakeCurrent(display,winSurface,EGL_NO_SURFACE,context);
        context = EGL_NO_CONTEXT;
        display = EGL_NO_DISPLAY;
        winSurface = nullptr;
        winSurface = 0;
        nativeWindow = 0;
        isPlay = false;
    }
}



Gles_play::Gles_play(JNIEnv *env, jobject thiz, PlayCallback *playCallback, const char *data_source,
                     jobject surface) {
    this->playCallback = playCallback;
    this->env = env;
    this->thiz = thiz;
    this->surface = surface;
    // 这里有坑，这里赋值之后，不能给其他地方用，因为被释放了，变成了悬空指针
    // this->data_source = source;

    // 解决上面的坑，自己Copy才行
    // [strlen(data_source)] 这段代码有坑：因为（hello  而在C++中是 hello\n），所以需要加1
    this->data_source = new char[strlen(data_source) + 1];
    strcpy(this->data_source,data_source);
}

Gles_play::~Gles_play() {
    if (playCallback) {
        delete playCallback;
        playCallback = 0;
    }
}

void Gles_play::showMessage(JNIEnv *env, const char *message, bool b) {
    if (this->playCallback) {
        if (b) {
            this->playCallback->onSucceed(message);
        } else {
            this->playCallback->onError(message);
        }
    }
}
