# IW4x 

## How to compile

- Run `premake5 vs2015` or use the delivered `generate.bat`.
- Build via solution file in `build\iw4x.sln`. (You can use the `build.bat` script to do it quick and easy.)

## Premake arguments

- `--copy-to=PATH` - Optional, copy the DLL to a custom folder after build, define the path here if wanted.
- `--no-new-structure` - Do not use new virtual path structure (separating headers and source files).
