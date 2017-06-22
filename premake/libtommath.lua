libtommath = {
	settings = nil
}

function libtommath.setup(settings)
	if not settings.source then error("Missing source") end

	libtommath.settings = settings

	if not libtommath.settings.defines then libtommath.settings.defines = {} end
end

function libtommath.import()
	if not libtommath.settings then error("Run libtommath.setup first") end

	links { "libtommath" }
	libtommath.includes()
end

function libtommath.includes()
	if not libtommath.settings then error("Run libtommath.setup first") end

	defines(libtommath.settings.defines)
	includedirs { libtommath.settings.source }
end

function libtommath.project()
	if not libtommath.settings then error("Run libtommath.setup first") end

	project "libtommath"
		language "C"

		libtommath.includes()
		files
		{
			path.join(libtommath.settings.source, "*.c"),
		}

		-- not our code, ignore POSIX usage warnings for now
		warnings "Off"

		defines { "_LIB" }
		removedefines { "_DLL", "_USRDLL" }
		kind "StaticLib"
end
