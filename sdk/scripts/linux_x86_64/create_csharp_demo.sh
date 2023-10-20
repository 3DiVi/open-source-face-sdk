#!/bin/bash

DEMO_NAME=${1:-"face"}
MAJOR=$(echo "$(dotnet --version)" | awk '{split($0, result, "."); print result[1]}')
MINOR=$(echo "$(dotnet --version)" | awk '{split($0, result, "."); print result[2]}')
UBUNTU_VERSION=$(echo "$(lsb_release -rs)" | awk '{split($0, result, "."); print result[1]}')

if [[ ${UBUNTU_VERSION} == 20 ]]; then
    OPENCV_RUNTIME=OpenCvSharp4_.runtime.ubuntu.20.04-x64
else
    OPENCV_RUNTIME=OpenCvSharp4.runtime.ubuntu.${UBUNTU_VERSION}.04-x64
fi

mkdir -p build
cd build
dotnet new sln -n csharp_${DEMO_NAME}_demo

cp -r ../samples/csharp/csharp_${DEMO_NAME}_demo ./csharp_${DEMO_NAME}_demo

cp ../src/csharp_api/* ./csharp_${DEMO_NAME}_demo/

cd ./csharp_${DEMO_NAME}_demo/
find . -type f -exec sed -i "s@open_source_sdk.dll@libopen_source_sdk.so@g" {} +
if [[ ${UBUNTU_VERSION} == 16 ]];
then
    find . -type f -exec sed -i "s@7.0@6.0@g" {} +
fi
cd ../

dotnet sln csharp_${DEMO_NAME}_demo.sln add csharp_${DEMO_NAME}_demo/csharp_${DEMO_NAME}_demo.csproj

cd csharp_${DEMO_NAME}_demo

dotnet add package OpenCvSharp4
dotnet add package ${OPENCV_RUNTIME} --prerelease
dotnet add package CommandLineParser

dotnet publish --configuration Release --output bin/publish /p:AllowUnsafeBlocks=true

cd ..
cp -r csharp_${DEMO_NAME}_demo/bin/publish/* ./make-install/bin
cp ./make-install/lib/libopen_source_sdk.so ./make-install/bin/

echo "Done!"
