-- vkd3d-shader, which translates IW4's Direct3D 9 shader bytecode to SPIR-V
-- for the Vulkan renderer (see src/Vulkan/).
--
-- Upstream only builds with autotools, which also generates some sources:
-- IDL headers (widl), HLSL and preprocessor parsers (bison and flex), and
-- SPIR-V grammar tables (perl). Those are in deps/extra/vkd3d/, along with
-- config.h, and scripts/generate-vkd3d.sh regenerates them for another
-- release. So when moving the submodule, rerun the script and compare the
-- source lists below with Makefile.am's libvkd3d_shader_la_SOURCES and
-- libvkd3d_common_la_SOURCES.
--
-- Note that we only need bytecode translation, but the library's entry
-- points reference the HLSL compiler and the rest. So we build all of
-- vkd3d-shader and don't patch it.
--
local targets = ...

local includedirs =
{
  "../extra/vkd3d/include",
  "../extra/vkd3d/libs/vkd3d-shader",
  "include",
  "include/private",
  "libs/vkd3d-shader"
}

-- MSVC has no unistd.h, which debug.c includes (see generate-vkd3d.sh).
--
if not _OPTIONS["mingw"] then
  table.insert (includedirs, "../extra/vkd3d/msvc")
end

targets:static_library ("vkd3d",
{
  directory = "vkd3d",
  language  = "C",
  cdialect  = "C17",

  files =
  {
    "libs/vkd3d-common/blob.c",
    "libs/vkd3d-common/debug.c",
    "libs/vkd3d-common/error.c",
    "libs/vkd3d-common/memory.c",
    "libs/vkd3d-common/string.c",

    "libs/vkd3d-shader/checksum.c",
    "libs/vkd3d-shader/d3d_asm.c",
    "libs/vkd3d-shader/d3dbc.c",
    "libs/vkd3d-shader/dxbc.c",
    "libs/vkd3d-shader/dxil.c",
    "libs/vkd3d-shader/fx.c",
    "libs/vkd3d-shader/glsl.c",
    "libs/vkd3d-shader/hlsl.c",
    "libs/vkd3d-shader/hlsl_codegen.c",
    "libs/vkd3d-shader/hlsl_constant_ops.c",
    "libs/vkd3d-shader/ir.c",
    "libs/vkd3d-shader/msl.c",
    "libs/vkd3d-shader/spirv.c",
    "libs/vkd3d-shader/tpf.c",
    "libs/vkd3d-shader/vkd3d_shader_main.c",

    "../extra/vkd3d/libs/vkd3d-shader/hlsl.tab.c",
    "../extra/vkd3d/libs/vkd3d-shader/hlsl.yy.c",
    "../extra/vkd3d/libs/vkd3d-shader/preproc.tab.c",
    "../extra/vkd3d/libs/vkd3d-shader/preproc.yy.c"
  },

  includedirs = includedirs,

  -- Note that blob.c includes vkd3d.h (Direct3D 12 API), which includes
  -- Vulkan headers.
  --
  dependencies = {"spirv-headers", "vulkan-headers"},

  -- Note that VKD3D_DEBUG_ENV names the debug environment variable as
  -- autotools does.
  --
  defines =
  {
    "VKD3D_DEBUG_ENV=VKD3D_SHADER_DEBUG",
    "_USE_MATH_DEFINES",
    "_CRT_SECURE_NO_WARNINGS"
  },

  export =
  {
    includedirs = {"include"},

    -- Note that without LIBVKD3D_SHADER_SOURCE vkd3d_shader.h declares
    -- functions dllimport, which a static library can't satisfy. With it
    -- they are dllexport on both sides, which links, but also means that
    -- iw4x.dll exports vkd3d-shader's API.
    --
    -- @@ Nothing outside the client calls them, but it would be cleaner
    --    not to export them. Upstream has no static build to follow.
    --
    defines = {"LIBVKD3D_SHADER_SOURCE"}
  }
})
