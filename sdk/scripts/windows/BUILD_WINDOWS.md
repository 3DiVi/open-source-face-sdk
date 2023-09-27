# Build FaceSDK for Windows (MSVC)

## Prerequisites

* Microsoft Visual Studio 17 or newer
* CMake

### Visual Studio install

1. Go to the Microsoft [site](https://visualstudio.microsoft.com/downloads/) and download the MSVC installer.  
1.1. Also download Build Tools for Visual Studio from this [link](https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2022)(This link to install tools for MSVC 22).

2. Run the installer and select the following toolkit:
    * Under Workloads, select "Desktop C++ Development".
    * Under "Individual Components"->"Compilers, Build Tools, and Runtimes"->"CMake Tools for Windows".

3. Click install button

### CMake

1. Download and install Cmake on your computer from [this link](https://cmake.org/download/).  
2. To check if CMake is installed correctly, open a command prompt (Win+R, then write `cmd`), and enter `cmake` command.

## Configure and build SDK

### Set PowerShell policy to Unrestricted
PowerShell policy should be set to Unrestricted to download SDK dependencies and configure SDK build.  
1. Open PowerShell as Administrator
2. Enter the command `Set-ExecutionPolicy Unrestricted`
3. Enter Y

### Build SDK
To configure SDK project, use the make_project.ps1 script. This script will automatically find and install all the necessary dependencies and build the project. To run, use the following command in Windows Powershell:
```powershell
.\scripts\windows\make_project.ps1
```
Then, open the `build\open_sourse_sdk.sln` solution in Visual Studio. Select `Release` configuration to build SDK and build `INSTALL` project.
