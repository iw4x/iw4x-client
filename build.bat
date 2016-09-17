@echo off & setlocal

cd %~dp0

if exist "%VS140COMNTOOLS%\vsvars32.bat" call "%VS140COMNTOOLS%\vsvars32.bat"
msbuild /version >NUL 2>NUL
if errorlevel 0 goto:build

if exist "%PROGRAMFILES(x86)%\msbuild\14.0\Bin\msbuild.exe" path %PROGRAMFILES(x86)%\msbuild\14.0\Bin;%PATH%
msbuild /version >NUL 2>NUL
if errorlevel 0 goto:build

echo Couldn't find any MSBuild to build this project.
echo Make sure you have Microsoft Build Tools 2015 or Visual Studio 2015 installed.
endlocal
exit /B 1

:build
call generate.bat
set PLATFORM=Win32
set CONFIGURATION=Release
msbuild /nologo /m /v:m %* build\iw4x.sln
endlocal
exit /B %ERRORLEVEL%
