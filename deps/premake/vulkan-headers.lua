-- Vulkan headers.
--
-- Note that we don't link Vulkan loader: volk loads it at run time
-- (see volk.lua).
--
local targets = ...

targets:binless_library ("vulkan-headers",
{
  directory = "Vulkan-Headers",

  export =
  {
    includedirs = {"include"},
    defines     = {"VK_USE_PLATFORM_WIN32_KHR"}
  }
})
