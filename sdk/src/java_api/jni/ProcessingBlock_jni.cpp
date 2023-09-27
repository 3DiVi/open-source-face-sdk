#include <jni.h>
/* Header for class Context */
#include <api/ProcessingBlock.h>
#include "ProcessingBlock_jni.h"
#include "jni_glob.h"

extern "C" {

JNIEXPORT void JNICALL Java_com_face_1sdk_ProcessingBlock_process
	(JNIEnv *env, jobject thiz, jobject data)
{
	api::HPBlock* handle_ = (api::HPBlock*) getPtr(env, thiz, "processing_block_ptr");
	api::HContext* handle_ctx = (api::HContext*) getPtr(env, data, "context_ptr");

	api::ContextEH* eh_ = nullptr;
	api::TDVProcessingBlock_processContext(handle_, handle_ctx, &eh_);
	checkException(eh_);
}

JNIEXPORT void JNICALL Java_com_face_1sdk_ProcessingBlock_destroyProcessingBloc_1jni
	(JNIEnv *env, jobject thiz)
{
	api::HPBlock* handle_ = (api::HPBlock*) getPtr(env, thiz, "processing_block_ptr");
	api::ContextEH* eh_ = nullptr;
	api::TDVProcessingBlock_destroyBlock(handle_, &eh_);
	checkException(eh_);
}

}



