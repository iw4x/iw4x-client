@echo off & setlocal

cd %~dp0

if exist "%PROGRAMFILES(x86)%\Microsoft Visual Studio\2019\Enterprise\Common7\Tools\VsMSBuildCmd.bat" call "%PROGRAMFILES(x86)%\Microsoft Visual Studio\2019\Enterprise\Common7\Tools\VsMSBuildCmd.bat"
call msbuild /version >NUL 2>NUL
if errorlevel 0 goto:build

if exist "%PROGRAMFILES(x86)%\Microsoft Visual Studio\2019\Enterprise\MSBuild\15.0\Bin\msbuild.exe" path %PROGRAMFILES(x86)%\Microsoft Visual Studio\2019\Enterprise\MSBuild\15.0\Bin;%PATH%
call msbuild /version >NUL 2>NUL
if errorlevel 0 goto:build

echo Couldn't find any MSBuild to build this project.
echo Make sure you have Visual C++ Build Tools 2019 or Visual Studio 2019 installed.
endlocal
exit /B 1

:build
call generate.bat
set PLATFORM=Win32
set CONFIGURATION=Release
call msbuild /nologo /m /v:m %* build\iw4x.sln
endlocal
exit /B %ERRORLEVEL%
