# System Requirements
Minimal system requirements are:
* CPU - 4 cores, 3.7GHz
* RAM - 8Gb
* Operating System -  Windows 7, 10-11/Ubuntu 16-22/ASTRA linux COMMON edition

# Building and Installation
You can use two parts of library - SDK and OpenMIDAS - separately, so there are  different prerequisites and installation steps for them.
## SDK

### Prerequisites
For Windows:
* [Visual Studio 2017 or newer](https://visualstudio.microsoft.com/downloads/)  
    1. Go to the Microsoft [site](https://visualstudio.microsoft.com/downloads/) and download the MSVC installer.  
    1.1. Also download Build Tools for Visual Studio from this [link](https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2022)(This link to install tools for MSVC 22).

    2. Run the installer and select the following toolkit:
        * Under Workloads, select "Desktop C++ Development".
         * Under "Individual Components"->"Compilers, Build Tools, and Runtimes"->"CMake Tools for Windows".

    3. Click install button
* [Python 3.7 or higher](https://www.python.org/downloads/)  
* [Visual C++ Redistributable for Visual Studio **2015 or higher**](https://aka.ms/vs/17/release/vc_redist.x64.exe)  
* [CMake 2.8.12 or higher](https://cmake.org/download/)
    1. Download and install Cmake on your computer 
    2. To check if CMake is installed correctly, open a command prompt (Win+R, then write `cmd`), and enter `cmake` command.
* Also you should install gdown pip's package:
  ```cmd
  pip.exe install gdown==4.7.1
  ```
* (To build and use Java API) [JDK 8](https://www.oracle.com/java/technologies/javase/javase8-archive-downloads.html)
* (To buiild and use C# API) [.NET 7.0 or higher](https://dotnet.microsoft.com/en-us/download)

For Linux:
* gcc compiler (4.8.5 or higher)
* [Python 3.7 or higher](https://www.python.org/downloads/)
* libgtk2.0
* standard wget and unzip bash utilities
* gdown pip's package  
* (To build and use Java API) openjdk-8-jdk
* (To buiild and use C# API) dotnet-sdk-7.0, tesseract-ocr, libdc1394-22, libopenexr24


Below the sample of instructions to install dependencies on Ubuntu
<details>
  <summary>Ubuntu 22.04</summary>

```bash
sudo apt update
sudo apt install gcc cmake
sudo apt install libgtk2.0-dev
sudo apt install wget unzip
sudo apt install python3 python3-pip libcanberra-gtk-module
sudo apt install lsb-core
pip3 install --upgrade pip setuptools wheel
pip3 install gdown==4.7.1

# Java API dependencies
sudo apt install openjdk-8-jdk
# C# API dependencies
sudo apt install apt-transport-https
sudo apt install dotnet-sdk-7.0
sudo apt install tesseract-ocr
sudo apt install libdc1394-dev libavcodec-dev libavformat-dev libswscale-dev libv4l-dev
sudo apt install libxvidcore-dev libx264-dev libjpeg-dev libpng-dev libtiff-dev
sudo apt install libopenexr-dev
```

</details>

<details>
  <summary>Ubuntu 20.04</summary>

```bash
sudo apt update
sudo apt install gcc cmake
sudo apt install libgtk2.0-dev
sudo apt install wget unzip
sudo apt install python3 python3-pip libcanberra-gtk-module
sudo apt install lsb-core
pip3 install --upgrade pip setuptools wheel
pip3 install gdown==4.7.1

# Java API dependencies
sudo apt install openjdk-8-jdk
# C# API dependencies
wget https://packages.microsoft.com/config/ubuntu/$(lsb_release -rs)/packages-microsoft-prod.deb -O packages-microsoft-prod.deb
sudo dpkg -i packages-microsoft-prod.deb
sudo apt install apt-transport-https
sudo apt update
sudo apt install dotnet-sdk-7.0
sudo apt install tesseract-ocr
sudo apt install libdc1394-22 libavcodec-dev libavformat-dev libswscale-dev libv4l-dev
sudo apt install libxvidcore-dev libx264-dev libjpeg-dev libpng-dev libtiff-dev
sudo apt install openexr
rm -f packages-microsoft-prod.deb
```

</details>

<details>
  <summary>Ubuntu 18.04</summary>

```bash
sudo apt update
sudo apt install gcc cmake
sudo apt install libgtk2.0-dev
sudo apt install wget unzip
sudo apt install python3 python3-pip libcanberra-gtk-module
sudo apt install lsb-core
pip3 install --upgrade pip setuptools wheel
pip3 install gdown==4.7.1

# Java API dependencies
sudo apt install openjdk-8-jdk
# C# API dependencies
wget https://packages.microsoft.com/config/ubuntu/$(lsb_release -rs)/packages-microsoft-prod.deb -O packages-microsoft-prod.deb
sudo dpkg -i packages-microsoft-prod.deb
sudo apt install apt-transport-https
sudo apt update
sudo apt install dotnet-sdk-7.0 tesseract-ocr
sudo apt install libdc1394-22 libavcodec-dev libavformat-dev libswscale-dev libv4l-dev
sudo apt install libxvidcore-dev libx264-dev libjpeg-dev libpng-dev libtiff-dev
sudo apt install openexr
rm -f packages-microsoft-prod.deb
```
</details>

<details>
  <summary>Ubuntu 16.04</summary>

```bash
sudo apt update
sudo apt install gcc cmake
sudo apt install libgtk2.0-dev
sudo apt install wget unzip
sudo apt install python3 python3-pip libcanberra-gtk-module
sudo apt install lsb-core

# There might be problems with system python in ubuntu 16, thus we recomend to install Python 3.8. Follow instructions to make python 3/8 your system python3 interpreter.
sudo apt install build-essential zlib1g-dev libncurses5-dev libgdbm-dev libnss3-dev libssl-dev libreadline-dev libffi-dev libsqlite3-dev wget libbz2-dev -y
wget https://www.python.org/ftp/python/3.8.0/Python-3.8.0.tgz
tar -xf Python-3.8.0.tgz
cd Python-3.8.0 && ./configure --enable-optimizations && make && sudo make install && cd ..

pip install --upgrade pip setuptools wheel
pip install urllib3==1.26.6

# Java API dependencies
sudo apt install openjdk-8-jdk
# C# API dependencies
wget https://packages.microsoft.com/config/ubuntu/$(lsb_release -rs)/packages-microsoft-prod.deb -O packages-microsoft-prod.deb
sudo dpkg -i packages-microsoft-prod.deb
sudo apt install apt-transport-https
sudo apt update
sudo apt install dotnet-sdk-6.0
sudo apt install tesseract-ocr
sudo apt install libdc1394-22 libavcodec-dev libavformat-dev libswscale-dev libv4l-dev
sudo apt install libxvidcore-dev libx264-dev libjpeg-dev libpng-dev libtiff-dev
sudo apt install openexr
rm -f packages-microsoft-prod.deb
```
</details>

### Build C++
Clone this repo 
```bash
git clone https://github.com/3DiVi/open-source-face-sdk
```
and follow instructions for your platform.  
For Windows:
  * Set PowerShell policy to Unrestricted. PowerShell policy should be set to Unrestricted to download SDK dependencies and configure SDK build.  
    1. Open PowerShell as Administrator
    2. Enter the command `Set-ExecutionPolicy Unrestricted`
    3. Enter Y
* Build SDK. To configure SDK project, use the make_project.ps1 script. This script will automatically find and install all the necessary dependencies and build the project. To run, use the following command in Windows Powershell:
    ```powershell
    cd open-source-face-sdk\sdk
    .\scripts\windows\make_project.ps1
    ```
  Then, open the `build\open_sourse_sdk.sln` solution in Visual Studio. Select `Release` configuration to build SDK and build `INSTALL` project.

For Linux:
  * Use the build_sdk.sh script. This script will automatically find and install all the necessary dependencies and build the project. To run, use the following command:

    ```bash
    cd open-source-face-sdk/sdk
    source scripts/linux_x86_64/build_sdk.sh    
    ```
### Build Python API 
When you successfully built C++ API on previous step, you can install python_api (located in _build/make_install_ after library building) via the comand 
```bash
pip3 install <path_to_python_api_folder>
```
Alternatively, you can use prebuilt pip packages to install Python API.  
For Windows
```bash
pip3 install  http://download.3divi.com/facesdk/archives/artifacts/wl/windows/face_sdk-1.0.0-py3-none-any.whl
```
For Linux:
```bash
pip3 install  http://download.3divi.com/facesdk/archives/artifacts/wl/linux/face_sdk-1.0.0-py3-none-any.whl
```

### Build Java API
If you want to use Java API, you should call build script from **Build C++** step with path to your JDK installation. For example, on Linux there should be command  
```bash
source scripts/linux_x86_64/build_sdk.sh  /usr/lib/jvm/java-8-openjdk-amd64   
```
After that, go to _sdk/samples/java_ and make there _bin_ directory. Then, you can build Java API sample with the next commands
```bash
javac -sourcepath ../../src/java_api/src/ -d bin com/face_detector_demo/face_detector_demo.java 
```
```bash
LD_LIBRARY_PATH=../../build/make-install/lib java -classpath ./bin com.face_detector_demo.face_detector_demo <path_to_image> ../../build/make-install
```

### Build C#
If you successfully built C++ API on previous steps, you can use builded library to work with C# API.  
For Windows
* In _sdk_ folder run following cmd command to build csharp_face_demo for Windows
    ```bash
    .\scripts\windows\create_sharp_demo.bat
    ```
* This script builds `face` demo by default. To build `body` or `estimator` demo, just call the script with corresponding argument.

For Linux
* In _sdk_ folder run following cmd command to build csharp_face_demo for Windows
    ```bash
    ./scripts/linux/create_sharp_demo.sh
    ```
* This script builds `face` demo by default. To build `body` or `estimator` demo, just call the script with corresponding argument.

## OpenMIDAS

### Prerequisites
Python 3.10.8 or higher

### Installation 
Clone this repo and run following command from _open_midas_ folder to install required dependencies
```bash
pip3 install -r requirements.txt
```
