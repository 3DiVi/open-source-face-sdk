package com.face_sdk;

import com.face_sdk.Context;
import com.face_sdk.ProcessingBlock;


/**
 * @brief Provides ProcessingBlock and Context creation
 * 
 */
public class Service {
	private long service_ptr;
	private String path_to_dir;
	static {
		System.loadLibrary("open_source_sdk_jni");
	}
	private Service(long service_ptr, String service_path_to_dir){
		this.service_ptr = service_ptr;
		this.path_to_dir = service_path_to_dir;
	}

	/**
	 * @brief Create a Service object
	 * 
	 * @param path_to_dir Path to directory with data/models
	 * @return Service 
	 */
	static public Service createService(String path_to_dir){
		long ptr = createService_jni(path_to_dir);
		return new Service(ptr, path_to_dir);
	}

	/**
	 * @brief Create a Context object
	 * 
	 * @return Context 
	 */
	public Context createContext(){
		long ptr = createContext_jni();
		return new Context(ptr, false);
	}

	/**
	 * @brief Create a ProcessingBlock object
	 * 
	 * @param data Config context with unit_type
	 * @return ProcessingBlock 
	 */
	public ProcessingBlock createProcessingBlock(Context data){
		data.getOrInsertByKey("@sdk_path").setString(this.path_to_dir);
		long ptr = createProcessingBlock_jni(data);
		return new ProcessingBlock(ptr);
	}

	static native long createService_jni(String str);
	native long createContext_jni();
	native long createProcessingBlock_jni(Context data);
}


