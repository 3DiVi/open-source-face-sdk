#!/bin/bash
set -e

echo "Download ONNX"

export PROJ_PATH=`pwd`
export CURRENT_ARCH=`uname -m`

mkdir -p 3rdparty/build && cd 3rdparty/build
wget https://github.com/microsoft/onnxruntime/releases/download/v1.4.0/onnxruntime-linux-x64-1.4.0.tgz
tar -xvf onnxruntime-linux-x64-1.4.0.tgz && mv onnxruntime-linux-x64-1.4.0 onnxruntime

rm onnxruntime-linux-x64-1.4.0.tgz

#download opencv artifact

wget https://download.3divi.com/facesdk/archives/artifacts/opencv/POS_SDK/3-1-0/opencv-ubuntu14.04-x86-64-install-dir.zip -O opencv.zip --no-check-certificate

unzip -d opencv opencv.zip

rm opencv.zip

cd ${PROJ_PATH}

mkdir build
cd build

export CMAKE_INSTALL_PREFIX="$(pwd)/make-install"

cmake \
    -DBUILD_SHARED=ON \
    -DWITH_SSE=ON \
    -DCMAKE_BUILD_TYPE=Release \
    -DWITH_SAMPLES=ON \
    -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX} \
    -DTDV_OPENCV_DIR=${PROJ_PATH}/3rdparty/build/opencv \
    -DTDV_ONNXRUNTIME_DIR=${PROJ_PATH}/3rdparty/build/onnxruntime/ \
    ..

make install
