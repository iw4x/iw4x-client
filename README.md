# IW4x

## How to compile

- Run `premake5 vs2015` (build with VS2015) or `premake5 vs2013` (with VS2013).
- Build via solution file in `build\iw4x.sln`.

## Premake arguments

- `--copy-to=PATH` - Optional, copy the DLL to a custom folder after build, define the path here if wanted.
- `--no-new-structure` - Do not use new virtual path structure (separating headers and source files).
