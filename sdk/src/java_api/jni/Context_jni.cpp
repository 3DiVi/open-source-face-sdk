#include <jni.h>
/* Header for class Context */
#include <api/Context.h>
#include "Context_jni.h"
#include "jni_glob.h"

extern "C" {

JNIEXPORT jlong JNICALL Java_com_face_1sdk_Context_size
	(JNIEnv * env, jobject thiz)
{
	api::HContext* handle_ = (api::HContext*) getPtr(env, thiz, "context_ptr");
	api::ContextEH* eh_ = nullptr;
	size_t lenght = TDVContext_getLength(handle_, &eh_);
	checkException(eh_);
	return lenght;
}


JNIEXPORT jboolean JNICALL Java_com_face_1sdk_Context_isNone
	(JNIEnv *env, jobject thiz)
{
	api::HContext* handle_ = (api::HContext*) getPtr(env, thiz, "context_ptr");
	api::ContextEH* eh_ = nullptr;
	bool result = api::TDVContext_isNone(handle_, &eh_);
	checkException(eh_);
	return result;
}

JNIEXPORT jboolean JNICALL Java_com_face_1sdk_Context_isArray
	(JNIEnv *env, jobject thiz)
{
	api::HContext* handle_ = (api::HContext*) getPtr(env, thiz, "context_ptr");
	api::ContextEH* eh_ = nullptr;
	bool result = api::TDVContext_isArray(handle_, &eh_);
	checkException(eh_);
	return result;
}

JNIEXPORT jboolean JNICALL Java_com_face_1sdk_Context_isObject
	(JNIEnv *env, jobject thiz)
{
	api::HContext* handle_ = (api::HContext*) getPtr(env, thiz, "context_ptr");
	api::ContextEH* eh_ = nullptr;
	bool result = api::TDVContext_isObject(handle_, &eh_);
	checkException(eh_);
	return result;
}

JNIEXPORT jboolean JNICALL Java_com_face_1sdk_Context_isBool
	(JNIEnv *env, jobject thiz)
{
	api::HContext* handle_ = (api::HContext*) getPtr(env, thiz, "context_ptr");
	api::ContextEH* eh_ = nullptr;
	bool result = api::TDVContext_isBool(handle_, &eh_);
	checkException(eh_);
	return result;
}

JNIEXPORT jboolean JNICALL Java_com_face_1sdk_Context_isLong
	(JNIEnv *env, jobject thiz)
{
	api::HContext* handle_ = (api::HContext*) getPtr(env, thiz, "context_ptr");
	api::ContextEH* eh_ = nullptr;
	bool result = api::TDVContext_isLong(handle_, &eh_);
	checkException(eh_);
	return result;
}

JNIEXPORT jboolean JNICALL Java_com_face_1sdk_Context_isDouble
	(JNIEnv *env, jobject thiz)
{
	api::HContext* handle_ = (api::HContext*) getPtr(env, thiz, "context_ptr");
	api::ContextEH* eh_ = nullptr;
	bool result = api::TDVContext_isDouble(handle_, &eh_);
	checkException(eh_);
	return result;
}

JNIEXPORT jboolean JNICALL Java_com_face_1sdk_Context_isString
	(JNIEnv *env, jobject thiz)
{
	api::HContext* handle_ = (api::HContext*) getPtr(env, thiz, "context_ptr");
	api::ContextEH* eh_ = nullptr;
	bool result = api::TDVContext_isString(handle_, &eh_);
	checkException(eh_);
	return result;
}

JNIEXPORT jboolean JNICALL Java_com_face_1sdk_Context_isDataPtr
	(JNIEnv *env, jobject thiz)
{
	api::HContext* handle_ = (api::HContext*) getPtr(env, thiz, "context_ptr");
	api::ContextEH* eh_ = nullptr;
	bool result = api::TDVContext_isDataPtr(handle_, &eh_);
	checkException(eh_);
	return result;
}


JNIEXPORT void JNICALL Java_com_face_1sdk_Context_setLong
	(JNIEnv *env, jobject thiz, jlong value)
{
	api::HContext* handle_ = (api::HContext*) getPtr(env, thiz, "context_ptr");

	api::ContextEH* eh_ = nullptr;
	api::TDVContext_putLong(handle_, (int64_t)value, &eh_);

	api::checkException(eh_);
}


JNIEXPORT void JNICALL Java_com_face_1sdk_Context_setDouble
	(JNIEnv *env, jobject thiz, jdouble value)
{
	api::HContext* handle_ = (api::HContext*) getPtr(env, thiz, "context_ptr");

	api::ContextEH* eh_ = nullptr;
	api::TDVContext_putDouble(handle_, (double)value, &eh_);

	api::checkException(eh_);
}

JNIEXPORT void JNICALL Java_com_face_1sdk_Context_setBool
	(JNIEnv *env, jobject thiz, jboolean value)
{
	api::HContext* handle_ = (api::HContext*) getPtr(env, thiz, "context_ptr");

	api::ContextEH* eh_ = nullptr;
	api::TDVContext_putBool(handle_, (bool)value, &eh_);

	api::checkException(eh_);
}

JNIEXPORT void JNICALL Java_com_face_1sdk_Context_setString
	(JNIEnv *env, jobject thiz, jstring value)
{

	const char *nativeString = env->GetStringUTFChars(value, 0);
	api::HContext* handle_ = (api::HContext*) getPtr(env, thiz, "context_ptr");

	api::ContextEH* eh_ = nullptr;
	api::TDVContext_putStr(handle_, nativeString, &eh_);

	api::checkException(eh_);
}

JNIEXPORT void JNICALL Java_com_face_1sdk_Context_setDataPtr
	(JNIEnv *env, jobject thiz, jbyteArray array)
{
	api::HContext* handle_ = (api::HContext*) getPtr(env, thiz, "context_ptr");

	int len = env->GetArrayLength(array);
	unsigned char *ptr = (unsigned char*)env->GetByteArrayElements(array, NULL);

	api::ContextEH* eh_ = nullptr;
	TDVContext_putDataPtr(handle_, ptr, len, &eh_);
	checkException(eh_);
}

JNIEXPORT jlong JNICALL Java_com_face_1sdk_Context_getLong
	(JNIEnv *env, jobject thiz)
{
	api::HContext* handle_ = (api::HContext*) getPtr(env, thiz, "context_ptr");
	api::ContextEH* eh_ = nullptr;
	jlong result = api::TDVContext_getLong(handle_, &eh_);
	api::checkException(eh_);

	return result;
}

JNIEXPORT jdouble JNICALL Java_com_face_1sdk_Context_getDouble
	(JNIEnv *env, jobject thiz)
{
	api::HContext* handle_ = (api::HContext*) getPtr(env, thiz, "context_ptr");
	api::ContextEH* eh_ = nullptr;
	jdouble result = api::TDVContext_getDouble(handle_, &eh_);
	api::checkException(eh_);

	return result;
}


JNIEXPORT jboolean JNICALL Java_com_face_1sdk_Context_getBool
	(JNIEnv *env, jobject thiz)
{
	api::HContext* handle_ = (api::HContext*) getPtr(env, thiz, "context_ptr");
	api::ContextEH* eh_ = nullptr;
	jboolean result = api::TDVContext_getBool(handle_, &eh_);
	api::checkException(eh_);

	return result;
}

JNIEXPORT jstring JNICALL Java_com_face_1sdk_Context_getString
	(JNIEnv *env, jobject thiz)
{
	api::HContext* handle_ = (api::HContext*) getPtr(env, thiz, "context_ptr");
	api::ContextEH* eh_ = nullptr;

	unsigned long str_size = api::TDVContext_getStrSize(handle_, &eh_);
	api::checkException(eh_);

	std::string result_string;
	result_string.resize(str_size);

	api::TDVContext_getStr(handle_, &result_string[0], &eh_);
	api::checkException(eh_);

	return env->NewStringUTF(result_string.data());

}


//JNIEXPORT jbyteArray JNICALL Java_com_face_sdk_Context_getDataPtr_1jni
//	(JNIEnv *env, jobject thiz)
//{
//
//}


JNIEXPORT jlong JNICALL Java_com_face_1sdk_Context_getByIndex_1jni
	(JNIEnv *env, jobject thiz, jint index)
{
	api::HContext* handle_ = (api::HContext*) getPtr(env, thiz, "context_ptr");
	api::ContextEH* eh_ = nullptr;

	api::HContext* handle = TDVContext_getByIndex(handle_, (int)index, &eh_);

	api::checkException(eh_);

	return (jlong)handle;
}

JNIEXPORT jlong JNICALL Java_com_face_1sdk_Context_getByKey_1jni
	(JNIEnv *env, jobject thiz, jstring key)
{
	api::HContext* handle_ = (api::HContext*) getPtr(env, thiz, "context_ptr");
	api::ContextEH* eh_ = nullptr;

	const char *nativeKey = env->GetStringUTFChars(key, 0);

	api::HContext* handle = TDVContext_getByKey(handle_, nativeKey, &eh_);

	api::checkException(eh_);

	return (jlong)handle;
}


JNIEXPORT jlong JNICALL Java_com_face_1sdk_Context_getOrInsertByKey_1jni
	(JNIEnv *env, jobject thiz, jstring key)
{
	api::HContext* handle_ = (api::HContext*) getPtr(env, thiz, "context_ptr");
	api::ContextEH* eh_ = nullptr;

	const char *nativeKey = env->GetStringUTFChars(key, 0);

	api::HContext* handle = TDVContext_getOrInsertByKey(handle_, nativeKey, &eh_);

	api::checkException(eh_);

	return (jlong)handle;
}

JNIEXPORT void JNICALL Java_com_face_1sdk_Context_pushBack_1jni
	(JNIEnv *env, jobject thiz, jobject data, jboolean copy)
{
	api::HContext* handle_ = (api::HContext*) getPtr(env, thiz, "context_ptr");
	api::HContext* handle_data = (api::HContext*) getPtr(env, data, "context_ptr");

	api::ContextEH* eh_ = nullptr;

	api::TDVContext_pushBack(handle_, handle_data, copy, &eh_);

	api::checkException(eh_);
}

JNIEXPORT void JNICALL Java_com_face_1sdk_Context_destroyContext_1jni
	(JNIEnv *env, jobject thiz)
{
	api::HContext* handle_ = (api::HContext*) getPtr(env, thiz, "context_ptr");

	api::ContextEH* eh_ = nullptr;
	api::TDVContext_destroy(handle_, &eh_);
	api::checkException(eh_);
}

JNIEXPORT void JNICALL Java_com_face_1sdk_Context_clear
	(JNIEnv *env, jobject thiz)
{
	api::HContext* handle_ = (api::HContext*) getPtr(env, thiz, "context_ptr");

	api::ContextEH* eh_ = nullptr;
	api::TDVContext_clear(handle_, &eh_);
	api::checkException(eh_);
}



}



