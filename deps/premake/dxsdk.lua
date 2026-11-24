-- DirectX SDK headers and x64 libraries.
--
-- Note that the client names the SDK libraries it links (see premake5.lua),
-- so the export only adds their directory.
--
local targets = ...

targets:binless_library ("dxsdk",
{
  directory = "dxsdk",

  export =
  {
    includedirs = {"Include"},
    libdirs     = {"Lib/x64"}
  }
})
