cmake_minimum_required(VERSION 2.8.12)

set(PROJECT_NAME face_demo)
project(${PROJECT_NAME})

add_definitions(-std=c++11)
link_directories(${3RDPARTY_OPENCV_LIB_DIR})

set(LIBS
	open-source-sdk
)

if (CMAKE_GENERATOR MATCHES "Visual Studio")
	set(LIBS ${LIBS} opencv_world310)
endif()

if(UNIX)
	set(LIBS ${LIBS}
			opencv_highgui
			opencv_imgcodecs
			opencv_imgproc
			opencv_core
			zlib
			libjpeg
			libwebp
			libpng
			libtiff
			libjasper
			gtk-x11-2.0
			gdk-x11-2.0
			gdk_pixbuf-2.0
			cairo
			gobject-2.0
			glib-2.0
		)
endif()

add_executable(${PROJECT_NAME}
	main.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
	${CMAKE_SOURCE_DIR}/include
	${3RDPARTY_INCLUDE_DIR}
)

target_link_libraries(${PROJECT_NAME} ${LIBS})

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
