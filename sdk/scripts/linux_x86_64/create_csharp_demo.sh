#!/bin/bash

DEMO_NAME=${1:-"face"}

mkdir -p build
cd build
dotnet new sln -n csharp_${DEMO_NAME}_demo

cp -r ../samples/csharp/csharp_${DEMO_NAME}_demo ./csharp_${DEMO_NAME}_demo

cp ../src/csharp_api/* ./csharp_${DEMO_NAME}_demo/

cd ./csharp_${DEMO_NAME}_demo/
find . -type f -exec sed -i "s@open_source_sdk.dll@libopen_source_sdk.so@g" {} +
cd ../

dotnet sln csharp_${DEMO_NAME}_demo.sln add csharp_${DEMO_NAME}_demo/csharp_${DEMO_NAME}_demo.csproj

cd csharp_${DEMO_NAME}_demo

dotnet add package OpenCvSharp4
dotnet add package OpenCvSharp4_.runtime.ubuntu.20.04-x64 #Replace for correct Ubuntu version and CPU architecture. Find at https://www.nuget.org/packages
dotnet add package CommandLineParser

dotnet publish --configuration Release --output bin/publish /p:AllowUnsafeBlocks=true

cd ..
cp -r csharp_${DEMO_NAME}_demo/bin/publish/* ./make-install/bin
cp ./make-install/lib/libopen_source_sdk.so ./make-install/bin/

echo "Done!"

