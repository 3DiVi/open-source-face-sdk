cmake_minimum_required(VERSION 3.5)

set(name open_source_sdk_jni)

project(${name})


add_library(${name} SHARED
	jni/Service_jni.cpp
	jni/Context_jni.cpp
	jni/ProcessingBlock_jni.cpp
)

target_include_directories(${name} PRIVATE "${JAVA_HOME}/include/" "${JAVA_HOME}/include/linux/" "${JAVA_HOME}/include/win32" )
target_include_directories(${name} PUBLIC ${CMAKE_SOURCE_DIR}/include)

target_link_libraries(${name}
	open_source_sdk
)


install(TARGETS ${name} DESTINATION lib)

