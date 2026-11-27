-- SPIR-V headers, which vkd3d-shader generates SPIR-V with.
--
local targets = ...

targets:binless_library ("spirv-headers",
{
  directory = "SPIRV-Headers",

  export =
  {
    includedirs = {"include"}
  }
})
