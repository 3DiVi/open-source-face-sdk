# Build C# API for Linux
* At first, you should install .NET. For Ubuntu you could do that with these comands:
```bash
wget https://packages.microsoft.com/config/ubuntu/$(lsb_release -rs)/packages-microsoft-prod.deb -O packages-microsoft-prod.deb
sudo dpkg -i packages-microsoft-prod.deb
sudo apt update
sudo apt install apt-transport-https
sudo apt install dotnet-sdk-7.0
```
* Also, there are some dependencies which should be installed for sample execution:
```bash
sudo apt install tesseract-ocr
sudo apt install libdc1394-22
sudo apt install libopenexr24
```
* There should be successful SDK build, so build it following the instructions.
* Finally, to build sample you should do that (example for csharp_face_demo, but can be used for csharp_estimator_demo and csharp_body_demo):
```bash
cd build
dotnet new sln -n csharp_face_demo
cp -r ../samples/csharp/csharp_face_demo ./csharp_face_demo
cp ../src/csharp_api/* ./csharp_face_demo/

cd ./csharp_face_demo
find . -type f -exec sed -i "s@open_source_sdk.dll@libopen_source_sdk.so@g" {} +
cd ../

dotnet sln csharp_face_demo.sln add csharp_face_demo/csharp_face_demo.csproj
cd csharp_face_demo

dotnet add package OpenCvSharp4
dotnet add package OpenCvSharp4_.runtime.ubuntu.20.04-x64 #Replace for correct Ubuntu version and CPU architecture. Find at https://www.nuget.org/packages
dotnet add package CommandLineParser

dotnet publish --configuration Release --output bin/publish /p:AllowUnsafeBlocks=true
cd ..
cp -r csharp_face_demo/bin/publish/* ./make-install/bin
```