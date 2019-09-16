@echo off
echo Updating submodules...
call git submodule update --init --recursive
call tools\premake5 %* vs2019
