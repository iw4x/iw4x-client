pushd "%~dp0"
mkdir "..\build\src\proto" 2>NUL
protoc.exe -I=proto --cpp_out=../build/src/proto/ proto/node.proto
popd