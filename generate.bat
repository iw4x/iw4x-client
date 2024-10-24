@echo off
echo Updating submodules...
call git submodule update --init --recursive

:: Check if the repository has an upstream remote (indicating a fork)
git remote get-url upstream >nul 2>&1
IF ERRORLEVEL 1 (
    echo No upstream remote found. This might be a fork.
    echo Adding upstream...
    git remote add upstream https://github.com/iw4x/iw4x-client.git
    echo Adding git tags...
    git fetch upstream --tags
    git push origin --tags
)

call tools\premake5 %* vs2022
