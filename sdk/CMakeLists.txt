cmake_minimum_required(VERSION 2.8.12)

include(ExternalProject)

set(PROJECT_NAME open_source_sdk)
project(${PROJECT_NAME})

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(WITH_SSE false CACHE BOOL "with or without SSE intrinsics")

option(ONNXRT_OBSOLETE_API "use api for onnxruntime v1.4" ON)
option(WITH_SAMPLES "build samples" OFF)
option(WITH_JAVA "build java_api" OFF)

add_definitions(-DNO_OPENCV)	# binary utils only

if(ONNXRT_OBSOLETE_API)
	add_definitions(-DONNXRT_OBSOLETE_API)
endif()

add_definitions(-fPIC)
add_definitions(-D_REENTRANT)
add_definitions(-D_POSIX_C_SOURCE)
add_definitions(-D_7ZIP_ST)
add_definitions(-D_Boost_USE_STATIC_LIBS=ON)
add_definitions(-D_Boost_USE_STATIC_RUNTIME=ON)

if(NOT UNIX)
	set(CMAKE_SHARED_LIBRARY_PREFIX "")
endif()

if(NOT DEFINED TDV_OPENCV_DIR)
	set(TDV_OPENCV_DIR ${CMAKE_SOURCE_DIR}/3rdparty/build/opencv)
endif()

if(NOT DEFINED TDV_ONNXRUNTIME_DIR)
	set(TDV_ONNXRUNTIME_DIR ${CMAKE_SOURCE_DIR}/3rdparty/build/onnxruntime)
endif()

if(NOT DEFINED TDV_INSTALL_DIR)
	set(TDV_INSTALL_DIR ${CMAKE_BINARY_DIR}/install)
endif()

set(TDV_DIRECT_INSTALL_DIR ${TDV_INSTALL_DIR})
set(TDV_3RDPARTY_DIR ${CMAKE_SOURCE_DIR}/3rdparty)

set(3RDPARTY_NLOHMANN external_nlohmann)
set(3RDPARTY_ONNXRUNTIME external_onnxruntime)
set(3RDPARTY_OPENCV external_OpenCV)
set(3RDPARTY_PREFIX ${CMAKE_BINARY_DIR}/3rdparty)

# Need for compliance with test/bridge_test, test/json_serializer.
set(3RDPARTY_LIB_DIR ${TDV_INSTALL_DIR}/3rdparty/lib/)
set(3RDPARTY_OPENCV_LIB_DIR ${TDV_INSTALL_DIR}/3rdparty/share/OpenCV/3rdparty/lib)
set(3RDPARTY_INCLUDE_DIR ${TDV_INSTALL_DIR}/3rdparty/include)

message(STATUS "Install dir " ${CMAKE_INSTALL_PREFIX})

# List of project dependencies.
set(3RDPARTY_TARGETS	${3RDPARTY_NLOHMANN}
						${3RDPARTY_OPENCV}
						${3RDPARTY_ONNXRUNTIME})
link_directories(
	${3RDPARTY_LIB_DIR}
	${TDV_DIRECT_INSTALL_DIR}/lib
)

set(SOURCES ${SOURCES}
	src/api/c_api.cpp
	src/tdv/modules/ProcessingBlock.cpp
	src/tdv/modules/DetectionModules/FaceDetectionModule.cpp
	src/tdv/modules/BaseEstimationModule.cpp
	src/tdv/modules/ONNXRuntimeAdapter.cpp
	src/tdv/modules/ONNXRuntimeEnvironment.cpp
	src/tdv/modules/FitterModule.cpp
	src/tdv/modules/FaceIdentificationModule.cpp
	src/tdv/modules/MatcherModule.cpp
	src/tdv/modules/AgeEstimationModule.cpp
	src/tdv/modules/EmotionsEstimationModule.cpp
	src/tdv/modules/GenderEstimationModule.cpp
	src/tdv/modules/GlassesEstimationModule.cpp
	src/tdv/modules/MaskEstimationModule.cpp
	src/tdv/modules/EyeOpenessEstimationModule.cpp
	src/tdv/modules/LivenessDetectionModule/LivenessBaseModule.cpp
	src/tdv/modules/LivenessDetectionModule/LivenessDetectionModule.cpp
	src/tdv/utils/recognizer_utils/RecognizerUtils.cpp
	src/tdv/modules/DetectionModules/BodyDetectionModule.cpp
	src/tdv/modules/BodyReidentificationModule.cpp
	src/tdv/modules/HpeResnetV1DModule.cpp
	src/tdv/utils/har_utils/har_utils.cpp
	src/tdv/data/JSONSerializer.cpp
	src/tdv/data/ContextUtils.cpp
)

if(CMAKE_GENERATOR MATCHES "Visual Studio")
	set(LIB_LIBS ${LIB_LIBS} libpthreadVC3)

	add_definitions(-DNOMINMAX)

	if(BUILD_SHARED)
	link_directories(${3RDPARTY_OPENCV_LIB_DIR})
	set(LIB_LIBS
		${LIB_LIBS}
		-Wl,--whole-archive
		opencv_world310
		-Wl,--no-whole-archive)
	endif()
elseif(PIP)
set(LIB_LIBS
	${LIB_LIBS}
	-Wl,--whole-archive
	-Wl,--no-whole-archive
	opencv_imgcodecs
	opencv_imgproc
	opencv_core)
else()
	set(LIB_LIBS ${LIB_LIBS} pthread dl)

	if(BUILD_SHARED)
	link_directories(${3RDPARTY_OPENCV_LIB_DIR})
	set(LIB_LIBS
		${LIB_LIBS}
		-Wl,--whole-archive
		opencv_imgcodecs
		opencv_imgproc
		opencv_core
		zlib
		libjpeg
		libwebp
		libpng
		libtiff
		libjasper
		cairo
		gobject-2.0
		glib-2.0
		-Wl,--no-whole-archive)
	endif()
endif()

add_subdirectory(3rdparty)
if(WITH_JAVA)
	add_subdirectory(src)
endif()

add_subdirectory(samples)

if(BUILD_SHARED)
	add_library(${PROJECT_NAME} SHARED ${SOURCES})
else()
	add_library(${PROJECT_NAME} STATIC ${SOURCES})
endif()

if(WITH_SSE)
	target_compile_options(${PROJECT_NAME} PRIVATE -msse4.1 -mssse3 -msse3 -msse2 -msse)
endif()

target_include_directories(${PROJECT_NAME} PRIVATE
	${CMAKE_SOURCE_DIR}/include
	${CMAKE_SOURCE_DIR}/src
	${3RDPARTY_INCLUDE_DIR})

add_dependencies(${PROJECT_NAME} ${3RDPARTY_TARGETS})

target_link_libraries(${PROJECT_NAME} ${LIB_LIBS})

install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/install/3rdparty/lib/
		DESTINATION lib
		FILES_MATCHING
		PATTERN "libonnxruntime.*"
		PATTERN "engines-1.1" EXCLUDE
		PATTERN "pkgconfig" EXCLUDE
)

if(WIN32 OR (CMAKE_GENERATOR MATCHES "Visual Studio"))
	install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/install/3rdparty/bin/ DESTINATION bin)
	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/install/3rdparty/lib/onnxruntime.dll DESTINATION bin)
else()
	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/install/3rdparty/lib/libonnxruntime.so DESTINATION lib)
endif()

if(BUILD_SHARED AND (WIN32 OR (CMAKE_GENERATOR MATCHES "Visual Studio")))
	install(TARGETS ${PROJECT_NAME} DESTINATION bin)
else()
	install(TARGETS ${PROJECT_NAME} DESTINATION lib)
endif()

install(DIRECTORY include/ DESTINATION include)
install(DIRECTORY data/ DESTINATION data)
install(DIRECTORY src/python_api DESTINATION .)
install(FILES samples/python/face_demo.py DESTINATION bin)
install(FILES samples/python/estimator_demo.py DESTINATION bin)
install(FILES samples/python/body_demo.py DESTINATION bin)

if (CMAKE_GENERATOR MATCHES "Visual Studio")
	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/install/3rdparty/lib/onnxruntime.dll DESTINATION python_api/face_sdk/for_windows/onnxruntime-windows-x86-64-shared-install-dir)
	install(FILES ${CMAKE_INSTALL_PREFIX}/bin/open_source_sdk.dll DESTINATION python_api/face_sdk/for_windows/open_source_sdk)
	install(FILES ${CMAKE_INSTALL_PREFIX}/bin/opencv_world310.dll DESTINATION python_api/face_sdk/for_windows/open_source_sdk)
endif()
