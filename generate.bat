@echo off
echo Updating submodules...
call git submodule update --init
call git submodule foreach --quiet "test $sm_path = deps/sentry-native || git submodule update --init --recursive"
call tools\premake5 %* vs2026
