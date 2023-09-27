package com.face_sdk;

import com.face_sdk.Context;


public class ProcessingBlock {
	private long processing_block_ptr;
	private String dll_path;
	static {
		System.loadLibrary("open_source_sdk_jni");
	}

	ProcessingBlock(long processing_block_ptr){
		this.processing_block_ptr = processing_block_ptr;
	}

	public void dispose() {
		destroyProcessingBloc_jni();
	}

	public void finalize() {
		dispose();
	}

	/**
	 * @brief Infer and put results in data
	 * 
	 * @param data Results of infer
	 */
	public native void process(Context data);
	public native void destroyProcessingBloc_jni();
}


