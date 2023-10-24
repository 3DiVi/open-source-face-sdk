@echo off

set DEMO_NAME=%1%

if %DEMO_NAME%.==. (
    set DEMO_NAME="face"
)

cd build
dotnet new sln -n csharp_%DEMO_NAME%_demo
xcopy /isvy "..\samples\csharp\csharp_%DEMO_NAME%_demo\" ".\csharp_%DEMO_NAME%_demo"
xcopy /isvy "..\src\csharp_api" ".\csharp_%DEMO_NAME%_demo\csharp_api"
dotnet sln csharp_%DEMO_NAME%_demo.sln add csharp_%DEMO_NAME%_demo/csharp_%DEMO_NAME%_demo.csproj
cd csharp_%DEMO_NAME%_demo
dotnet add package OpenCvSharp4
dotnet add package OpenCvSharp4.runtime.win
dotnet add package CommandLineParser
dotnet publish --configuration Release --output bin\publish /p:AllowUnsafeBlocks=true
cd ..
xcopy /isvy "csharp_%DEMO_NAME%_demo\bin\publish" ".\make_install\bin\"

@echo Done!
