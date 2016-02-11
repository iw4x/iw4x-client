pushd "%~dp0"
mkdir "..\build\src\proto" 2>NUL
protoc.exe -I=../src/Proto --cpp_out=../build/src/proto/ ../src/Proto/node.proto
popd