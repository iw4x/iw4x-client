-- volk, Vulkan meta-loader.
--
-- The client loads vulkan-1.dll with volk at run time and doesn't link its
-- import library. This way iw4x.dll doesn't depend on Vulkan, which is only
-- needed when the Vulkan renderer is selected (see src/Vulkan/). Note
-- that VK_NO_PROTOTYPES makes Vulkan headers declare no functions, so every
-- Vulkan call goes through volk.
--
local targets = ...

targets:static_library ("volk",
{
  directory = "volk",
  language  = "C",

  files = {"volk.h", "volk.c"},

  export =
  {
    includedirs  = {"."},
    defines      = {"VK_NO_PROTOTYPES"},
    dependencies = {"vulkan-headers"}
  }
})
