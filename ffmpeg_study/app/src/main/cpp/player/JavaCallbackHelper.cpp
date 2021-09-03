//
// Created by hyk on 21-9-3.
//

#include "JavaCallbackHelper.h"


JavaCallbackHelper::JavaCallbackHelper(JavaVM *_javaVM,
        JNIEnv *_env, jobject &_jobj) : javaVm(_javaVM),env(_env) {
    jobj = env->NewGlobalRef(_jobj);
    jclass  jclazz = env->GetObjectClass(jobj);

    jmId_error = env->GetMethodID(jclazz,"onError","(I)V");
    jmId_prepared = env->GetMethodID(jclazz,"onPrepared","()V");
    jmId_progress = env->GetMethodID(jclazz,"onProgress","(I)V");
}

JavaCallbackHelper::~JavaCallbackHelper() {
    env->DeleteGlobalRef(jobj);
    jobj = nullptr;
}


void JavaCallbackHelper::onError(int errorCode, int threadId) {
    //子thread调用Java中方法
    if (THREAD_CHILD == threadId){
        JNIEnv *jniEnv;
        //attach 当前 thread
        if (javaVm ->AttachCurrentThread(&jniEnv, nullptr)!=JNI_OK){
            return;
        }
        jniEnv->CallVoidMethod(jobj,jmId_error,errorCode);
        javaVm->DetachCurrentThread();
    } else{
        env->CallVoidMethod(jobj,jmId_error,errorCode);
    }
}

void JavaCallbackHelper::onPrepared(int threadId) {
    if (THREAD_CHILD == threadId){
        JNIEnv *jniEnv;
        if (javaVm->AttachCurrentThread(&jniEnv, nullptr)!=JNI_OK){
            return;
        }
        jniEnv->CallVoidMethod(jobj,jmId_prepared);

        javaVm->DestroyJavaVM();
    } else{
        env->CallVoidMethod(jobj,jmId_prepared);
    }
}

void JavaCallbackHelper::onProgress(int progress, int threadId) {
    if (THREAD_CHILD == threadId){
        JNIEnv *jniEnv;
        if (javaVm->AttachCurrentThread(&jniEnv, nullptr)!=JNI_OK){
            return;
        }
        jniEnv->CallVoidMethod(jobj,jmId_progress,progress);
        javaVm->DestroyJavaVM();
    } else{
        env ->CallVoidMethod(jobj,jmId_progress,progress);
    }
}

