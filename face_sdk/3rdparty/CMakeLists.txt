cmake_minimum_required(VERSION 2.8.12)

include(ExternalProject)

add_subdirectory(nlohmann)
add_subdirectory(OpenCV)

add_subdirectory(onnxruntime)