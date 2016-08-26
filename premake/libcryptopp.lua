libcryptopp = {
	settings = nil,
}

function libcryptopp.setup(settings)
	if not settings.source then error("Missing source.") end

	libcryptopp.settings = settings
end

function libcryptopp.import()
	if not libcryptopp.settings then error("Run libcryptopp.setup first") end

	libcryptopp.includes()
end

function libcryptopp.includes()
	if not libcryptopp.settings then error("Run libcryptopp.setup first") end

	filter "Debug*"
		defines { "_DEBUG" }

	filter "Release*"
		defines { "NDEBUG" }

	filter "system:windows"
		defines { "_WINDOWS", "WIN32" }
	filter {}

	includedirs { libcryptopp.settings.source }
end

function libcryptopp.project()
	if not libcryptopp.settings then error("Run libcryptopp.setup first") end

	externalrule "MASM"
		buildmessage "Building and assembling %(Identity)..."
		propertydefinition {
			name = "PreprocessorDefinitions",
			kind = "string",
			value = "",
			switch = "/D",
		}
		propertydefinition {
			name = "UseSafeExceptionHandlers",
			kind = "boolean",
			value = false,
			switch = "/safeseh",
		}

	--[[
	rule "CustomProtoBuildTool"
		display "C++ prototype copy"
		location "./build"
		fileExtension ".proto"
		buildmessage "Preparing %(Identity)..."
		buildcommands {
			'if not exist "$(ProjectDir)\\src\\%(Filename)" copy "%(Identity)" "$(ProjectDir)\\src\\%(Filename)"',
			'echo: >> "src\\%(Filename).copied"',
		}
		buildoutputs {
			'$(ProjectDir)\\src\\%(Filename)',
		}
	]]

	project "libcryptopp"
		language "C++"
		characterset "MBCS"

		defines {
			"USE_PRECOMPILED_HEADERS"
		}
		includedirs
		{
			libcryptopp.settings.source,
		}
		files
		{
			path.join(libcryptopp.settings.source, "*.cpp"),
			--path.join(libcryptopp.settings.source, "*.cpp.proto"),
			path.join(libcryptopp.settings.source, "*.h"),
			path.join(libcryptopp.settings.source, "*.txt"),
		}

		removefiles {
			path.join(libcryptopp.settings.source, "eccrypto.cpp"),
			path.join(libcryptopp.settings.source, "eprecomp.cpp"),
			path.join(libcryptopp.settings.source, "bench*"),
			path.join(libcryptopp.settings.source, "*test.*"),
			path.join(libcryptopp.settings.source, "fipsalgt.*"),
			path.join(libcryptopp.settings.source, "cryptlib_bds.*"),
			path.join(libcryptopp.settings.source, "validat*.*"),
		}

		-- Pre-compiled header
		pchheader "pch.h" -- must be exactly same as used in #include directives
		pchsource(path.join(libcryptopp.settings.source, "pch.cpp")) -- real path

		defines { "_SCL_SECURE_NO_WARNINGS" }
		warnings "Off"

		vectorextensions "SSE"

		rules {
			"MASM",
			--"CustomProtoBuildTool",
		}

		kind "SharedLib"
		filter "*Static"
			kind "StaticLib"

		filter "kind:SharedLib"
			defines { "CRYPTOPP_IMPORTS" }

		filter "architecture:x86"
			exceptionhandling "SEH"
			masmVars {
				UseSafeExceptionHandlers = true,
				PreprocessorDefinitions = "_M_X86",
			}
		filter "architecture:x64"
			files {
				path.join(libcryptopp.settings.source, "x64masm.asm"),
			}
			masmVars {
				PreprocessorDefinitions = "_M_X64",
			}
		filter { "architecture:x64", "kind:SharedLib" }
			files {
				path.join(libcryptopp.settings.source, "x64dll.asm"),
			}

		filter("files:" .. path.join(libcryptopp.settings.source, "dll.cpp")
			.. " or files:" .. path.join(libcryptopp.settings.source, "iterhash.cpp"))
			flags { "NoPCH" }
end
