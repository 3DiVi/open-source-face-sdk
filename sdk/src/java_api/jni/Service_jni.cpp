#include <jni.h>

#include <api/c_api.h>
#include <api/Service.h>
#include <api/Exception.h>
#include <iostream>
#include "Service_jni.h"
#include "jni_glob.h"


extern "C"
JNIEXPORT jlong JNICALL Java_com_face_1sdk_Service_createService_1jni
	(JNIEnv *env, jclass clazz, jstring jdll_path)
{
//1)
	std::string dll_path = jstring2string(env, jdll_path);

//2)
	api::Service service_ptr = api::Service::createService(dll_path);

//3)
	return (jlong) new api::Service(service_ptr);

}

extern "C"
JNIEXPORT jlong JNICALL Java_com_face_1sdk_Service_createContext_1jni
	(JNIEnv *env, jobject thiz)
{
	api::ContextEH* eh_ = nullptr;
	api::HContext* handle_ = api::TDVContext_create(&eh_);
	api::checkException(eh_);

	return (jlong)handle_;

}

extern "C"
JNIEXPORT jlong JNICALL Java_com_face_1sdk_Service_createProcessingBlock_1jni
	(JNIEnv *env, jobject thiz, jobject data)
{
	api::HContext* handle_ctx = (api::HContext*) getPtr(env, data, "context_ptr");

	api::ContextEH* eh_ = nullptr;
	api::HPBlock* handle_ = api::TDVProcessingBlock_createProcessingBlock(handle_ctx, &eh_);
	api::checkException(eh_);

	return (jlong)handle_;
}