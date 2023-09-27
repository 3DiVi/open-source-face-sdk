/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class com_face_sdk_Service */

#ifndef _Included_com_face_sdk_Service
#define _Included_com_face_sdk_Service
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     com_face_sdk_Service
 * Method:    createService_jni
 * Signature: (Ljava/lang/String;)J
 */
JNIEXPORT jlong JNICALL Java_com_face_1sdk_Service_createService_1jni
  (JNIEnv *, jclass, jstring);

/*
 * Class:     com_face_sdk_Service
 * Method:    createContext_jni
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_com_face_1sdk_Service_createContext_1jni
  (JNIEnv *, jobject);

/*
 * Class:     com_face_sdk_Service
 * Method:    createProcessingBlock_jni
 * Signature: (Lcom/face_sdk/Context;)J
 */
JNIEXPORT jlong JNICALL Java_com_face_1sdk_Service_createProcessingBlock_1jni
  (JNIEnv *, jobject, jobject);

#ifdef __cplusplus
}
#endif
#endif