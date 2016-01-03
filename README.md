# IW4x

## How to compile

- Run `premake5 vs2013` (build with VS2013) or `premake5 vs2015` (with VS2015). We use VS2013 for compatibility reasons.
- Build via solution file in `build\iw4x.sln`. (You can use the `build.bat` script to do it quick and easy.)

## Premake arguments

- `--copy-to=PATH` - Optional, copy the DLL to a custom folder after build, define the path here if wanted.
- `--no-new-structure` - Do not use new virtual path structure (separating headers and source files).
