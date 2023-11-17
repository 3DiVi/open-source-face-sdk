$JAVA_HOME=$args[0]

if ([string]::IsNullOrEmpty($JAVA_HOME))
{
    $WITH_JAVA="OFF"
}
else
{
    $WITH_JAVA="ON"
}

Write-Output 'Begin making Open source SDK...'

$dest_3rdparty = ".\build\install\3rdparty"
$source_3rdparty = ".\3rdparty\"

$url_onnxruntime = "https://github.com/microsoft/onnxruntime/releases/download/v1.4.0/onnxruntime-win-x64-1.4.0.zip"
$url_opencv = "https://download.3divi.com/facesdk/archives/artifacts/opencv/POS_SDK/3-1-0/opencv-windows-msvc-vc14-x86-64-install-dir.zip"
$url_pthread = "https://download.3divi.com/facesdk/archives/artifacts/pthread/pthreads_windows_msvc_vc14_install_dir.zip"
function get_3rd_by_url {

    [CmdletBinding()]
    param (
        [string]$url,
        [string]$source,
        [string]$dest,
        [string]$name
    )
    
    if (-not(Test-Path -Path "$source\$name.zip" -PathType Leaf)) {
        Write-Output "Load $name..."
        wget $url -O "$source\$name.zip"
    }
    if (-not(Test-Path -Path "$source\$name")) {
        Write-Output "Copy $name..."
        Expand-Archive "$source\$name.zip" $source;
        Get-ChildItem $source -Recurse |
            Where {$_.Name -Match '-win-x64-1.4.0'} |
            Rename-Item -NewName {$_.name -replace '-win-x64-1.4.0'}
        Copy-Item "$source\$name\*" -Destination $dest -Recurse -Force
    }
    Write-Output "Done $name."
}

if (Test-Path -Path ".\build\") {
    rm ".\build\" -r -Force
}

if (Test-Path -Path "$source_3rdparty\build") {
    rm "$source_3rdparty\build" -r -Force
}

rm .\data\models\* -r

if (Test-Path -Path "$source_3rdparty\nlohmann") {
    Write-Output 'Load nlohmann...'
    try {
        $nlohmann_pref = "include\nlohmann"
        if (-not(Test-Path -Path "$dest_3rdparty\$nlohmann_pref")) {
            New-Item -ItemType "directory" -Path "$dest_3rdparty\$nlohmann_pref" -Force
            Copy-Item "$source_3rdparty\nlohmann\*.hpp" -Destination "$dest_3rdparty\$nlohmann_pref" -Recurse -Force
        }
    }
    catch {
        throw $_.Exception.Message
    }
    Write-Output 'Done!'
}
else {
    throw Write-Output "Please check nlohmann folder" 
}

if (-not(Test-Path -Path "$source_3rdparty\build")) {
    New-Item -ItemType "directory" -Path "$source_3rdparty\build" -Force
}

if (Test-Path -Path "$source_3rdparty\build") {
    get_3rd_by_url -url $url_onnxruntime -source "$source_3rdparty\build" -dest $dest_3rdparty -name "onnxruntime"
    get_3rd_by_url -url $url_opencv -source "$source_3rdparty\build" -dest $dest_3rdparty -name "opencv"
    get_3rd_by_url -url $url_pthread -source "$source_3rdparty\build" -dest $dest_3rdparty -name "pthread"
    
    gdown https://drive.google.com/u/0/uc?id=162OXlEh_18TLI0denqNysnBAGE3l8F-N -O models.zip

    tar -xf models.zip -C data\models

    del models.zip

    $CMAKE_INSTALL_PREFIX = pwd
    $CMAKE_INSTALL_PREFIX = "$CMAKE_INSTALL_PREFIX\build\make-install"
    cd build
    cmake -DBUILD_SHARED=ON -DWITH_SSE=ON -DCMAKE_BUILD_TYPE=Release -DWITH_SAMPLES=ON -DWITH_JAVA="$WITH_JAVA" -DJAVA_HOME="$JAVA_HOME" -DCMAKE_INSTALL_PREFIX="$CMAKE_INSTALL_PREFIX" ..
}