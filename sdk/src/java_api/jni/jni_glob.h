#ifndef __PBIO_API__JAVA_API__JNI_GLOB_H_
#define __PBIO_API__JAVA_API__JNI_GLOB_H_

#include <string>
#include <jni.h>


template<typename T>
class JLocalRefHandler
{
public:
    JLocalRefHandler(
        JNIEnv *env,
        T obj) :
    env(env),
    obj(obj)
    {
    }

    ~JLocalRefHandler()
    {
        if (obj)
        {
            env->DeleteLocalRef(obj);
        }
    }

    T get()
    {
        return obj;
    }

private:
    JNIEnv *env;
    T obj;
};

class JByteArrayHandler
{
public:
    JByteArrayHandler(
        JNIEnv * env,
        jbyteArray & jbyte_array,
        jint mode);
    

    jbyte* get_jbyteArray_ptr() const;

    ~JByteArrayHandler();

private:
    JNIEnv * env;
    jbyteArray &jbyte_array;
    jbyte * jbyte_array_ptr;
    jint mode;
};

inline jlong getPtr(JNIEnv * env, jobject thiz, const char * ptr_name)
{
	JLocalRefHandler<jclass> clzz(env, env->GetObjectClass(thiz));
	jfieldID field = env->GetFieldID(clzz.get(), ptr_name, "J");
	jlong ptr = env->GetLongField(thiz, field);

	if (ptr == 0){
		printf("Error transform class2ptr");
	}
	return ptr;
}

inline jbyteArray unsigned_char_ptr2jbyteArray(JNIEnv * env, const unsigned char * array, int size)
{
	jbyteArray jarray = env->NewByteArray(size);
	env->SetByteArrayRegion(jarray, 0, size, (const jbyte *)array);

	return jarray;
}


inline std::string jstring2string(JNIEnv * env, jstring jstr)
{
	const char * buf = env->GetStringUTFChars(jstr, NULL);
	std::string str = buf;
    env->ReleaseStringUTFChars(jstr, buf);

    return str;
}

#endif
