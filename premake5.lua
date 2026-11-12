-- Visual Studio solution of the client and its third-party libraries.
--
-- Generate it in build/ with, for example:
--
--   tools\premake5 vs2022
--
-- Note that the third-party libraries come from the submodules in deps/,
-- which generate.bat and generate-linux.sh fetch first.
--
local module_directory = path.join (_MAIN_SCRIPT_DIR, "premake")

local target_set = dofile (path.join (module_directory, "target.lua"))
local revision   = dofile (path.join (module_directory, "revision.lua"))
local c_literal  = dofile (path.join (module_directory, "c-literal.lua"))

-- The build directory holds the solution along with everything the build
-- produces. Files generated from the sources go into src/ under it, which
-- mirrors their place in the source tree.
--
local mingw = _OPTIONS["mingw"]
local build_directory = path.join (_MAIN_SCRIPT_DIR, "build", mingw and "mingw" or "")
local generated_directory = path.join (build_directory, "src")

newoption
{
  trigger     = "mingw",
  description = "Generate a 64-bit MinGW-w64 GNU Make build"
}

newoption
{
  trigger     = "copy-to",
  value       = "PATH",
  description = "Copy the binaries into PATH after each build"
}

newoption
{
  trigger     = "copy-pdb",
  description = "Copy the debug information as well (requires --copy-to)"
}

newoption
{
  trigger     = "disable-binary-check",
  description = "Don't verify that the game executable is the supported one"
}

newoption
{
  trigger     = "sentry-dsn",
  value       = "URL",
  description = "Submit crash reports to URL (compiled out by default)"
}

newoption
{
  trigger     = "no-buildinfo",
  description = "Don't regenerate the build information before each build"
}

newoption
{
  trigger     = "profile",
  description = "Build with Tracy (see README.md)"
}

-- Note that the version action exits as soon as it is done, so that the
-- version is the last line of its output (premake reports the completion of
-- an action otherwise).
--
newaction
{
  trigger     = "version",
  description = "Print the version of the source tree",

  execute = function ()
    print (revision.load (_MAIN_SCRIPT_DIR):version ())
    os.exit (0)
  end
}

newaction
{
  trigger     = "generate-buildinfo",
  description = "Generate the build information headers (version.h, etc)",

  execute = function ()
    local r = revision.load (_MAIN_SCRIPT_DIR)

    print ("info: generating build information for " .. r:version ())

    r:write (generated_directory)
  end
}

-- Verify the options before we act on them.
--
if _OPTIONS["copy-pdb"] and not _OPTIONS["copy-to"] then
  premake.error ("--copy-pdb specified without --copy-to")
end

if mingw and _ACTION and _ACTION ~= "gmake" and _ACTION ~= "generate-buildinfo" then
  premake.error ("--mingw requires the gmake action")
end

if mingw and _OPTIONS["copy-to"] then
  premake.error ("--copy-to is only supported by the Visual Studio build")
end

if mingw then
  -- Premake adds /usr/lib64 for x86_64 GCC, which is a host directory and
  -- should not be searched when cross-compiling with MinGW.
  --
  premake.override (premake.tools.gcc.libraryDirectories.architecture,
                    "x86_64",
                    function () return {} end)
end

-- Load the third-party targets.
--
-- Note that we skip iw4-open-formats: it describes 32-bit asset structures
-- and only zone tools use it (see below).
--
local targets = target_set.new (path.join (_MAIN_SCRIPT_DIR, "deps"))
targets:load (path.join (_MAIN_SCRIPT_DIR, "deps/premake"),
              {"iw4-open-formats"})

workspace "iw4x"
  location (build_directory)
  objdir "%{wks.location}/obj"
  targetdir "%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}"
  startproject "iw4x"

  -- The game is a 64-bit Windows executable, so this is the only platform.
  --
  configurations {"Debug", "Release"}
  platforms {"x64"}
  architecture "x86_64"
  systemversion "latest"

  if mingw then
    system "windows"
    toolset "gcc"
    gccprefix "x86_64-w64-mingw32-"
    makesettings {".DELETE_ON_ERROR:"}
    omitframepointer "Off"
    linkoptions {"-static"}
  end

  language "C++"
  cppdialect "C++20"
  characterset "ASCII"
  staticruntime "On"
  defines {"_WINDOWS", "WIN32"}

  -- Treat warnings as errors in both configurations. CI builds both and
  -- we want a local debug build to fail on the same warnings as CI
  -- does.
  --
  warnings "Extra"
  fatalwarnings {"All"}

  -- Don't warn about the headers in the external include directories,
  -- which are those of the third-party libraries (see premake/target.lua).
  -- GCC doesn't warn about them either way.
  --
  externalwarnings "Off"

  -- Produce debug information in both configurations since we symbolicate
  -- the crash reports with it.
  --
  symbols "On"
  editandcontinue "Off"

  incrementallink "Off"
  minimalrebuild "Off"
  multiprocessorcompile "On"

  filter "configurations:Debug"
    optimize "Debug"
    defines {"DEBUG", "_DEBUG"}

  -- Note that a release build has no RTTI. The debug build keeps it for the
  -- component names that it prints (see Components::Component).
  --
  -- With link-time code generation the code is generated by the linker,
  -- which then issues the unreachable code warnings (C4702). We ignore them
  -- for now.
  --
  -- @@ Find out which code they come from and whether it is ours.
  --
  -- Keep the frame pointers since the crash reports walk the stacks with
  -- them.
  --
  filter "configurations:Release"
    -- Size optimization emitted unresolved constructor calls with MinGW GCC.
    optimize (mingw and "Speed" or "Size")
    omitframepointer "Off"
    rtti "Off"
    defines {"NDEBUG"}

    -- GCC failed during LTO when linking the 32-bit MinGW DLL.
    --
    -- @@ See if it links the 64-bit one.
    --
    if not mingw then
      linktimeoptimization "On"
      linkoptions {"/IGNORE:4702"}
    end

  filter {}

project "iw4x"
  kind "SharedLib"

  if mingw then
    targetprefix ""
  end

  -- Link the Windows libraries that the client uses. Note that d3dx9 comes
  -- from the DirectX SDK with Visual Studio (see deps/premake/dxsdk.lua)
  -- and from MinGW-w64 itself otherwise.
  --
  links
  {
    "advapi32",
    "avrt",
    "cfgmgr32",
    "comctl32",
    "crypt32",
    "d3d9",
    "d3dx9",
    "dbghelp",
    "dwmapi",
    "gdi32",
    "hid",
    "iphlpapi",
    "ksuser",
    "ntdll",
    "ole32",
    "rpcrt4",
    "setupapi",
    "shlwapi",
    "urlmon",
    "uuid",
    "wininet",
    "winmm",
    "ws2_32"
  }

  files
  {
    "src/**.rc",
    "src/**.hpp",
    "src/**.cpp",
    "src/**.asm"
  }

  -- Zone tools.
  --
  -- These read and write 32-bit IW4x zones. On x64 we load stock zones
  -- converted with OpenAssetTools instead. Note that Zones and ModelSurfs
  -- only load what zone tools produce.
  --
  removefiles
  {
    "src/Components/Modules/AssetInterfaces/**",
    "src/Components/Modules/ModelSurfs.*",
    "src/Components/Modules/ZoneBuilder.*",
    "src/Components/Modules/ZoneConvert.*",
    "src/Components/Modules/Zones.*"
  }

  -- Sources not yet ported to x64.
  --
  -- NOTE: remember to register each component in Loader::Initialize() when
  --       taking its sources off this list.
  --
  removefiles
  {
    "src/Components/Modules/Changelog.*",
    "src/Components/Modules/Chat.*",
    "src/Components/Modules/ClanTags.*",
    "src/Components/Modules/ClientCommand.*",
    "src/Components/Modules/ConnectProtocol.*",
    "src/Components/Modules/Console.*",
    "src/Components/Modules/Controller.*",
    "src/Components/Modules/D3D9Ex.*",
    "src/Components/Modules/Debug.*",
    "src/Components/Modules/Dedicated.*",
    "src/Components/Modules/Discord.*",
    "src/Components/Modules/Discovery.*",
    "src/Components/Modules/Download.*",
    "src/Components/Modules/Elevators.*",
    "src/Components/Modules/Events.*",
    "src/Components/Modules/FastFiles.*",
    "src/Components/Modules/FileSystem.*",
    "src/Components/Modules/Friends.*",
    "src/Components/Modules/Huffman.*",
    "src/Components/Modules/Lean.*",
    "src/Components/Modules/Localization.*",
    "src/Components/Modules/MapDump.*",
    "src/Components/Modules/MapRotation.*",
    "src/Components/Modules/Maps.*",
    "src/Components/Modules/Materials.*",
    "src/Components/Modules/Menus.*",
    "src/Components/Modules/Missile.*",
    "src/Components/Modules/ModelCache.*",
    "src/Components/Modules/ModList.*",
    "src/Components/Modules/NetworkDebug.*",
    "src/Components/Modules/News.*",
    "src/Components/Modules/Node.*",
    "src/Components/Modules/Party.*",
    "src/Components/Modules/PlayerMovement.*",
    "src/Components/Modules/PlayerName.*",
    "src/Components/Modules/Playlist.*",
    "src/Components/Modules/Profiler.*",
    "src/Components/Modules/QuickPatch.*",
    "src/Components/Modules/RawFiles.*",
    "src/Components/Modules/RawMouse.*",
    "src/Components/Modules/RCon.*",
    "src/Components/Modules/RemoteControl.*",
    "src/Components/Modules/Renderer.*",
    "src/Components/Modules/Rumble.*",
    "src/Components/Modules/Security.*",
    "src/Components/Modules/ServerCommands.*",
    "src/Components/Modules/ServerDemo.*",
    "src/Components/Modules/ServerInfo.*",
    "src/Components/Modules/ServerList.*",
    "src/Components/Modules/Session.*",
    "src/Components/Modules/SlowMotion.*",
    "src/Components/Modules/Sound.*",
    "src/Components/Modules/StartupMessages.*",
    "src/Components/Modules/Stats.*",
    "src/Components/Modules/StringTable.*",
    "src/Components/Modules/StructuredData.*",
    "src/Components/Modules/TextRenderer.*",
    "src/Components/Modules/Theatre.*",
    "src/Components/Modules/Threading.*",
    "src/Components/Modules/Toast.*",
    "src/Components/Modules/UIFeeder.*",
    "src/Components/Modules/Updater.*",
    "src/Components/Modules/ViewModelFx.*",
    "src/Components/Modules/ViewModelFxSetup.*",
    "src/Components/Modules/VisionFile.*",
    "src/Components/Modules/Voice.*",
    "src/Components/Modules/Vote.*",
    "src/Components/Modules/Weapon.*",
    "src/Components/Modules/Window.*",
    "src/Components/Modules/WineD3D.*",
    "src/Components/Modules/BotLib/**",
    "src/Components/Modules/GSC/**",
    "src/Controller/**",
  }

  -- Assemble the assembly sources with ml directly instead of through the
  -- MASM build customization, for two reasons. Firstly, the customization
  -- names the object after the file name without the extension, so Foo.asm
  -- would overwrite the object of Foo.cpp. Secondly, it relies on
  -- XamlTaskFactory, which MSBuild under Wine doesn't have.
  --
  -- We also prefix the object name with the name of the source directory
  -- since the objects all go into the same directory and two assembly
  -- sources with the same name would otherwise produce the same object.
  --
  -- Note that the custom build step passes its outputs to the linker.
  --
  local asm_object = "%{cfg.objdir}/%{file.directory:match ('[^/]+$')}." ..
                     "%{file.basename}.asm." .. (mingw and "o" or "obj")

  filter "files:**.asm"
    buildmessage "%{file.name}"

    if mingw then
      buildcommands
      {
        '"$(UASM)" -q -win64 -Fo"' .. asm_object .. '" "%{file.relpath}"'
      }
    else
      buildcommands
      {
        "ml64.exe /nologo /c /Zi /W3 /WX " ..
        "/Fo\"" .. asm_object .. "\" \"%{file.relpath}\""
      }
    end

    buildoutputs {asm_object}

  filter {}

  -- We put the generated directory first so that its version.h takes
  -- precedence over the one generate-linux.sh writes into the source tree.
  --
  includedirs
  {
    generated_directory,
    "src"
  }

  -- Spell the generated directory with $(ProjectDir) as well, which the
  -- Visual Studio resource editor needs to open Resource.rc.
  --
  if mingw then
    makesettings {"UASM ?= uasm"}

    -- Note that windres doesn't recognize -isystem, so we only pass it the
    -- -I directories, which are the ones the resource script includes from.
    --
    filter "files:**/Resource.rc"
      local resource_object = "%{cfg.objdir}/%{file.basename}.rc.o"
      buildmessage "%{file.name}"
      buildcommands
      {
        '$(RESCOMP) "$<" -O coff -o "$@" $(RESFLAGS) $(DEFINES) ' ..
        '$(filter -I%,$(INCLUDES))'
      }
      buildinputs
      {
        path.join (generated_directory, "version.h"),
        path.join (generated_directory, "version.hpp"),
        "src/iw4x.manifest"
      }
      buildoutputs {resource_object}

    filter {}
  else
    resincludedirs {"$(ProjectDir)src"}

    -- Don't generate a manifest since Resource.rc embeds iw4x.manifest,
    -- which would collide with the one the linker generates.
    --
    manifest "Off"
  end

  -- Note that the precompiled header is named the way the sources include
  -- it. The forced include makes it the first include of every source.
  --
  pchheader "STDInclude.hpp"
  pchsource "src/STDInclude.cpp"
  forceincludes {"STDInclude.hpp"}

  defines {"RAPIDJSON_HAS_STDSTRING"}

  if _OPTIONS["disable-binary-check"] then
    defines {"DISABLE_BINARY_CHECK"}
  end

  -- Treat an empty DSN as none, which is what CI passes when the secret
  -- isn't set.
  --
  if _OPTIONS["sentry-dsn"] and _OPTIONS["sentry-dsn"] ~= "" then
    defines {"IW4X_SENTRY_DSN=" .. c_literal (_OPTIONS["sentry-dsn"])}
  end

  -- Keep identical functions apart (/OPT:NOICF). With folding, a frame of
  -- a crash report can name another function that has the same code.
  --
  if not mingw then
    filter "configurations:Release"
      linkoptions {"/OPT:NOICF"}

    filter {}
  end

  -- Import the third-party libraries that the client uses directly. We get
  -- the libraries they depend on through their exports (see
  -- premake/target.lua).
  --
  local libraries =
  {
    "companion",
    "discord-rpc",
    "fonts",
    "gsl",
    "json",
    "libtomcrypt",
    "libtommath",
    "mongoose",
    "pdcurses",
    "protobuf",
    "rapidjson",
    "sentry",
    "sqlite",
    "stb",
    "tracy",
    "zlib"
  }

  if not mingw then
    table.insert (libraries, "dxsdk")
  end

  targets:import (libraries, "iw4x")

  -- Regenerate the build information before each build to reflect the
  -- current commit. Use the same Premake script so that the generated
  -- files are written to the correct directory.
  --
  -- Note that this requires a Windows Git on PATH, which is normally
  -- unavailable when building under Wine (see --no-buildinfo).
  --
  if not _OPTIONS["no-buildinfo"] then
    local command = string.format ("\"%s\" --file=\"%s\" %sgenerate-buildinfo",
                                   path.translate (_PREMAKE_COMMAND),
                                   path.translate (_MAIN_SCRIPT),
                                   mingw and "--mingw " or "")

    if mingw then
      makesettings
      {
        ".DEFAULT_GOAL := all",
        "prebuild: src/version.hpp",
        ".PHONY: force-buildinfo",
        "src/version.h src/version.hpp &: force-buildinfo",
        "\t$(SILENT) " .. command
      }
    else
      prebuildcommands {command}
    end
  end

  -- Copy the binaries of the build, which include those of the Sentry
  -- helpers, into the directory specified with --copy-to.
  --
  -- Note that cmd.exe carries on after a failed command and the build only
  -- sees the status of the last one. So we make each command exit on
  -- failure.
  --
  if _OPTIONS["copy-to"] then
    local d = path.translate (path.getabsolute (_OPTIONS["copy-to"]))

    local copy = function (pattern)
      return string.format ("copy /y \"$(TargetDir)%s\" \"%s\" || exit /b 1",
                            pattern, d)
    end

    postbuildcommands
    {
      string.format ("if not exist \"%s\" mkdir \"%s\" || exit /b 1", d, d)
    }

    if _OPTIONS["copy-pdb"] then
      postbuildcommands {copy ("*.pdb")}
    end

    postbuildcommands
    {
      copy ("*.exe"),
      copy ("*.dll")
    }
  end

group "External Dependencies"
  targets:define ()

group ""

-- Compile the client messages (src/Proto/*.proto) into the generated
-- directory, where the client includes them from as <proto/...>.
--
-- Note that the rule runs in the protobuf project (see deps/premake/
-- protobuf.lua), which lives in the build directory like every project.
--
if mingw then
  project "protobuf"
    makesettings {"PROTOC ?= protoc", ".DEFAULT_GOAL := all"}

    local proto_directory = path.join (_MAIN_SCRIPT_DIR, "src/Proto")
    local output_directory = path.join (generated_directory, "proto")
    local input = path.getrelative (build_directory, proto_directory)
    local output = path.getrelative (build_directory, output_directory)

    for _, proto in ipairs (os.matchfiles (path.join (proto_directory, "*.proto"))) do
      local name = path.getname (proto)
      local base = path.join (output, path.getbasename (proto))
      files {path.join (build_directory, base .. ".pb.cc")}

      makesettings
      {
        "prebuild: " .. base .. ".pb.h",
        base .. ".pb.cc " .. base .. ".pb.h &: " .. input .. "/" .. name,
        "\t@echo protoc " .. name .. "\n" ..
        "\t$(SILENT) mkdir -p \"$(@D)\"\n" ..
        "\t$(SILENT) \"$(PROTOC)\" -I=\"" .. input ..
        "\" --cpp_out=\"" .. output .. "\" \"$<\""
      }
    end
else
  rule "ProtobufCompiler"
    display "Protobuf compiler"
    location (build_directory)
    fileExtension ".proto"
    buildmessage "protoc %(Identity)"

    buildcommands
    {
      "if not exist \"$(ProjectDir)src\\proto\" " ..
      "mkdir \"$(ProjectDir)src\\proto\"",

      string.format ("\"$(ProjectDir)%s\" --error_format=msvs " ..
                     "-I=%%(RelativeDir) --cpp_out=src\\proto %%(Identity)",
                     path.translate (
                       path.getrelative (build_directory,
                                         path.join (_MAIN_SCRIPT_DIR,
                                                    "tools/protoc.exe"))))
    }

    buildoutputs
    {
      "$(ProjectDir)src\\proto\\%(Filename).pb.cc",
      "$(ProjectDir)src\\proto\\%(Filename).pb.h"
    }
end
