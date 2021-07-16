@echo off
echo Updating submodules...
call git submodule update --init --recursive
call tools\premake5 %* vs2019 --generate_iw4x_specific_zones
