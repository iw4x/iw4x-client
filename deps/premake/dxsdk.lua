-- DirectX SDK headers and x86 libraries.
--
-- Note that the client links the SDK libraries it needs with #pragma
-- comment (lib), so the export only adds their directory.
--
local targets = ...

targets:binless_library ("dxsdk",
{
  directory = "dxsdk",

  export =
  {
    includedirs = {"Include"},
    libdirs     = {"Lib/x86"}
  }
})
