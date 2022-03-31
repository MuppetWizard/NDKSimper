#include <jni.h>
#include <string>
#include <android/log.h>
#include <iostream>


#define TAG "native-lib"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,TAG,__VA_ARGS__);
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG,__VA_ARGS__);
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,TAG,__VA_ARGS__);

extern "C" JNIEXPORT jstring JNICALL
Java_com_youyi_ndksimple_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */){
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}

/*
 * 动态注册
 */
//对应的Java类全路径名
const char *classPathName = "com/youyi/ndksimple/MainActivity";
extern "C" //支持C语言
JNIEXPORT void JNICALL //告诉虚拟机。这是jni函数
native_dynamicRegister(JNIEnv* env,jobject instance,jstring name){
    const char *j_name = env->GetStringUTFChars(name,NULL);
    LOGD("Dynamic Register %s",j_name);

    //JNI 异常处理
    jclass  clazz = env->GetObjectClass(instance);
    jmethodID mid= env->GetMethodID(clazz,"testException","()V");//从java代码抛出异常
    env->CallVoidMethod(instance,mid);//执行抛出异常
    jthrowable exc = env->ExceptionOccurred(); //检测是否发生异常
    if (exc) {
        env->ExceptionDescribe(); //打印异常
        env->ExceptionClear(); //清除掉发生的异常
//        jclass newExcCls = env->FindClass("java/lang/IllegalArgumentException");
//        env->ThrowNew(newExcCls,"An Exception occurs from JNI");//返回一个新的异常信息
    }

    env->ReleaseStringUTFChars(name,j_name);
}

/*
 * 创建全局引用
 */
jclass personClass;
extern "C" JNIEXPORT void JNICALL
native_globalRef(JNIEnv *env,jobject instance){
    LOGD("Test Local Ref");
    if (personClass == nullptr) {
        //提升全局解决不能重复使用问题
        const char *person_class = "com/youyi/ndksimple/Person";
        jclass jclass1 = env->FindClass(person_class);
//        personClass = static_cast<jclass>(env->NewGlobalRef(jclass1));//创建全局引用
        personClass = static_cast<jclass>(env->NewWeakGlobalRef(jclass1));//创建弱全局引用
        LOGD("personClass == nullptr is Running");
    }

    //Person实例化
    const char *sig = "()V";
    const char *method = "<init>";
    jmethodID init_method = env->GetMethodID(personClass,method,sig);
    //创建
    env->NewObject(personClass,init_method);

//    if (env->IsSameObject(personClass,personClass))
//      可以用 IsSameObject 函数来检验一个弱全局引用是否仍然指向活动的类实例.

    //显式释放全局引用
//    env->DeleteGlobalRef(personClass);//回收全局引用
    env->DeleteWeakGlobalRef(personClass);//回收弱全局引用
    personClass = nullptr;
};

JavaVM *jvm;
jobject instance;
void *customThread(void *pVoid){
    //调用一定需要 JNIEnv *env
    //env 无法跨越线程，只有JavaVM可以跨越线程

    JNIEnv *env;
    int result = jvm->AttachCurrentThread(&env, NULL); //将native线程，附加到jvm上
    if (result != 0) {
        return 0;
    }

    jclass mainAct = env->GetObjectClass(instance);

    //拿到createDialog方法
    const char *sig = "()V";
    jmethodID method_updateUI = env->GetMethodID(mainAct,"createDialog",sig);
    env->CallVoidMethod(instance,method_updateUI);

    //解除附加到JVM的native线程
    jvm->DetachCurrentThread();
    return 0;
}

extern "C" JNIEXPORT void JNICALL
native_testThread(JNIEnv *env, jobject mainAct){
    instance = env->NewGlobalRef(mainAct);//获取全局变量，保证不被释放回收
    env->GetJavaVM(&jvm);
    pthread_t pthreadID;
    pthread_create(&pthreadID,0,customThread,instance);
    pthread_join(pthreadID,0);
}

extern "C" JNIEXPORT void JNICALL
native_uiThread(JNIEnv *env, jobject mainAct){
    if (instance != NULL) {
        env->DeleteGlobalRef(instance);
        instance = NULL;
    }
}



/*
 * 线程同步
 */
extern "C" JNIEXPORT void JNICALL
native_count(JNIEnv *env, jobject instance){
    jclass cls = env->GetObjectClass(instance);
    jfieldID fieldId = env->GetFieldID(cls,"count","I");

    if (env->MonitorEnter(instance) != JNI_OK) {
        LOGE("%s: MonitorEnter() failed", __FUNCTION__);
    }

     /* synchronized block */
    int val = env->GetIntField(instance,fieldId);
    val++;
    LOGD("count: %d",val);
    env->SetIntField(instance,fieldId,val);

    if (env->ExceptionOccurred()) {
        LOGE("ExceptionOccurred()...");
        if (env->MonitorExit(instance) != JNI_OK) {
            LOGE("%s: MonitorExit() failed", __FUNCTION__);
        }
    }

    if (env->MonitorExit(instance) != JNI_OK) {
        LOGE("%s: MonitorExit() failed", __FUNCTION__);
    }

}

/* 源码结构体
 * typedef struct {
    const char* name;
    const char* signature;
    void*       fnPtr;
    } JNINativeMethod;
    原生方法集合
 */
static const JNINativeMethod jniNativeMethod[] = {
        {"dynamicRegister","(Ljava/lang/String;)V",(void *) (native_dynamicRegister)},
        {"globalRef","()V",(void *) (native_globalRef)},
        {"nativeCount","()V",(void *) (native_count)},
        {"uiThread","()V",(void *)(native_uiThread)},
        {"testThread","()V",(void *)(native_testThread)}
};
/*
 * 该函数定义在 jni.h 头文件中，System.local.library() 时会调用 JNI_OnLoad()函数
 */
JNIEXPORT jint JNICALL
JNI_OnLoad(JavaVM *javaVm,void *pVoid){
    //通过虚拟机，创建新的 env
    JNIEnv *env = nullptr;
    jint result = javaVm->GetEnv(reinterpret_cast<void **>(&env),JNI_VERSION_1_6);
    if (result != JNI_OK) {
        return JNI_ERR;//主动报错
    }
    jclass mainActivityClass = env->FindClass(classPathName);
    env->RegisterNatives(mainActivityClass,jniNativeMethod,
                         sizeof(jniNativeMethod) / sizeof(JNINativeMethod) // 动态注册的数量
                         );
    return JNI_VERSION_1_6;
}


/*
 * JNI 处理对象
 */
extern "C" JNIEXPORT jobject JNICALL
Java_com_youyi_ndksimple_MainActivity_getPerson(JNIEnv *env,jobject instance){
    //获取 Java类 全路径
    const char *person_class = "com/youyi/ndksimple/Person";
    const char *method = "<init>";//java 构造方法标识

    //找到需要处理的 Java 对象 class
    jclass j_person_class = env->FindClass(person_class);

    //拿到构造方法
    jmethodID person_constructor = env->GetMethodID(j_person_class,method,"()V");

    //创建对象
    jobject person_obj = env->NewObject(j_person_class,person_constructor);

    //拿到 setName 方法签名，并拿到对应的 setName 方法
    const char *nameSig = "(Ljava/lang/String;)V";
    jmethodID person_setName_id = env->GetMethodID(j_person_class,"setName",nameSig);

    //拿到 setAge 方法签名，并拿到对应的 setAge 方法
    const char *ageSig = "(I)V";
    jmethodID person_setAge_id = env->GetMethodID(j_person_class,"setAge", ageSig);

    //拿到 setGen 方法签名，并拿到对应的 setGen 方法
    const char *genSig = "(Ljava/lang/String;)V";
    jmethodID person_setGen_id = env->GetMethodID(j_person_class,"setGen",genSig);

    //正在嗲用 java对象
    const char *name = "Peter Parker";
    jstring newStringName = env->NewStringUTF(name);
    const char *gen = "male";
    jstring newStringGen = env->NewStringUTF(gen);
    //call method
    env->CallVoidMethod(person_obj,person_setName_id,newStringName);
    env->CallVoidMethod(person_obj,person_setAge_id,16);
    env->CallVoidMethod(person_obj,person_setGen_id,newStringGen);

    const char *sig = "()Ljava/lang/String;";
    jmethodID person_toString_id = env->GetMethodID(j_person_class,"toString",sig);
    // call method
    jobject obj_string = env->CallObjectMethod(person_obj,person_toString_id);
    //墙砖 jni string
    jstring perStr = static_cast<jstring> (obj_string);
    //转 c string
    const char *str_person = env->GetStringUTFChars(perStr,NULL);
    //打印
    LOGD("Person: %s",str_person);
    return person_obj;
}

/*
 * 基本数据类型接收获取操作
 */
extern "C" JNIEXPORT void JNICALL
Java_com_youyi_ndksimple_MainActivity_test1(
        JNIEnv* env,jobject instance,jboolean jboolean1,jchar jchar1,jbyte jbyte1,jshort jshort1,
        jlong jlong1,jfloat jfloat1,jdouble jdouble1,jstring name_,jint age,jintArray i_,jobject person,
        jobjectArray strs,jbooleanArray bArray
        ){
    //接收java传过来的 Boolean 值
    unsigned char c_boolean = jboolean1;
    LOGD("boolean -> %d",c_boolean);

    //接收java传过来的 char 值
    unsigned short c_char = jchar1;
    LOGD("char -> %d",c_char);

    //接收java传过来的 byte 值
    char c_byte = jbyte1;
    LOGD("byte -> %d",c_byte);

    //接收java传过来的 short 值
    short c_short = jshort1;
    LOGD("short -> %d",c_short);

    //接收java传过来的 long 值
    long c_long = jlong1;
    LOGD("long -> %ld",c_long);

    //接收java传过来的 float 值
    float c_float = jfloat1;
    LOGD("float -> %f",c_float);

    //接收java传过来的 double 值
    double c_double = jdouble1;
    LOGD("double -> %f",c_double);

    //接收java传过来的 String 值
    const char *name_str = env->GetStringUTFChars(name_,0);
    LOGD("string -> %s",name_str);

    //接收java传过来的 int 值
    int age_java = age;
    LOGD("int -> %d",age_java);

    //接收java传过来的 int[] 值
    jint *intArray = env->GetIntArrayElements(i_,NULL);
    //获取数组长度
    jsize intArraySize = env->GetArrayLength(i_);
    for (int i = 0; i < intArraySize; ++i) {
        LOGD("IntArray -> %d",intArray[i]);
    }
    //释放数组
    env->ReleaseIntArrayElements(i_,intArray,0);

    //接收java传过来的 String[] 值
    jsize stringArray = env->GetArrayLength(strs);
    for (int i = 0; i < stringArray; ++i) {
        jobject jobject1 = env->GetObjectArrayElement(strs,i);
        // 强转 JNI String
        jstring stringArrayData = static_cast<jstring> (jobject1);
        // 转 c string
        const char *itemStr = env->GetStringUTFChars(stringArrayData,NULL);
        LOGD("String[%d] -> %s",i,itemStr);
        // 回收 String[]
        env->ReleaseStringUTFChars(stringArrayData,itemStr);
    }

    //接收传递的 object 对象
    //获取字节码
    const char *person_class_str = "com/youyi/ndksimple/Person";
    //转 jni class
    jclass person_class = env->FindClass(person_class_str);
    //获取方法签名 javap -a
    // JNI 方法描述符各式: (参数)返回值
    const char *sig = "()Ljava/lang/String;";
    jmethodID jmethodId = env->GetMethodID(person_class,"toString",sig);
    // call method
    jobject obj_string = env->CallObjectMethod(person,jmethodId);
    //墙砖 jni string
    jstring perStr = static_cast<jstring> (obj_string);
    //转 c string
    const char *str_name = env->GetStringUTFChars(perStr,NULL);
    //打印
    LOGD("Person: %s",str_name);
    //回收
    env->DeleteLocalRef(person_class);
    env->DeleteLocalRef(person);

    //接收java 传递过来的 booleanArray
    jsize b_array_size = env->GetArrayLength(bArray);
    jboolean *booleanArray = env->GetBooleanArrayElements(bArray,NULL);
    for (int i = 0; i < b_array_size; ++i) {
        bool b = booleanArray[i];
        jboolean jb = booleanArray[i];
        LOGD("bool[%d] -> %d",i,booleanArray[i])
    }
    env->ReleaseBooleanArrayElements(bArray,booleanArray,0);
}