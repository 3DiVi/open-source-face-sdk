@echo off

set JAVA_HOME=%1%

if %JAVA_HOME%.==. (
    set WITH_JAVA="OFF"
) else (
    set WITH_JAVA="ON"
)

@REM copy nlohmann
if exist 3rdparty\nlohmann ( 
    mkdir build\install\3rdparty\include\nlohmann
    xcopy 3rdparty\nlohmann\*.hpp build\install\3rdparty\include\nlohmann /s /e /y
)

@REM make dir
if not exist 3rdparty\build (
    mkdir 3rdparty\build
)

if exist 3rdparty\build (
    @REM download and copy onnxruntime
    if not exist .\3rdparty\build\onnxruntime.zip (
        powershell -command "wget https://github.com/microsoft/onnxruntime/releases/download/v1.4.0/onnxruntime-win-x64-1.4.0.zip -O .\3rdparty\build\onnxruntime.zip"    
    )
    if not exist .\3rdparty\build\onnxruntime (
        powershell -command "Expand-Archive .\3rdparty\build\onnxruntime.zip .\3rdparty\build; Rename-Item .\3rdparty\build\onnxruntime-win-x64-1.4.0 onnxruntime"
        xcopy 3rdparty\build\onnxruntime\* build\install\3rdparty /s /e /y
    )

    @REM download opencv
    if not exist .\3rdparty\build\opencv.zip (
        powershell -command "wget https://download.3divi.com/facesdk/archives/artifacts/opencv/POS_SDK/3-1-0/opencv-windows-msvc-vc14-x86-64-install-dir.zip -O .\3rdparty\build\opencv.zip"
    )
    if not exist .\3rdparty\build\opencv (
        powershell -command "Expand-Archive .\3rdparty\build\opencv.zip .\3rdparty\build\opencv"
        xcopy 3rdparty\build\opencv\* build\install\3rdparty /s /e /y
    )

    @REM download pthread
    if not exist .\3rdparty\build\pthread.zip (
        powershell -command "wget https://download.3divi.com/facesdk/archives/artifacts/pthread/pthreads_windows_msvc_vc14_install_dir.zip -O .\3rdparty\build\pthread.zip"
    )
    if not exist .\3rdparty\build\pthread (
        powershell -command "Expand-Archive .\3rdparty\build\pthread.zip .\3rdparty\build\pthread"
        xcopy 3rdparty\build\pthread\* build\install\3rdparty /s /e /y
    )

    @REM download models
    gdown https://drive.google.com/u/0/uc?id=162OXlEh_18TLI0denqNysnBAGE3l8F-N -O models.zip

    tar -xf models.zip -C data\models

    del models.zip

    cd build
    set "CMAKE_INSTALL_PREFIX=%cd%\build\make-install"
    echo %CMAKE_INSTALL_PREFIX%
    cmake -DBUILD_SHARED=ON -DWITH_SSE=ON -DCMAKE_BUILD_TYPE=Release -DWITH_SAMPLES=ON -DWITH_JAVA=%WITH_JAVA% -DJAVA_HOME=%JAVA_HOME% -DCMAKE_INSTALL_PREFIX=%CMAKE_INSTALL_PREFIX% ..
)
