zlib = {
	settings = nil
}

function zlib.setup(settings)
	if not settings.source then error("Missing source.") end

	zlib.settings = settings

	if not zlib.settings.defines then zlib.settings.defines = {} end
end

function zlib.import()
	if not zlib.settings then error("You need to call zlib.setup first") end

	links { "zlib" }
	zlib.includes()
end

function zlib.includes()
	if not zlib.settings then error("You need to call zlib.setup first") end

	includedirs { zlib.settings.source }
	defines(zlib.settings.defines)
end

function zlib.project()
	if not zlib.settings then error("You need to call zlib.setup first") end

	project "zlib"
		language "C"

		zlib.includes()
		files
		{
			path.join(zlib.settings.source, "*.h"),
			path.join(zlib.settings.source, "*.c"),
		}
		defines
		{
			"_CRT_SECURE_NO_DEPRECATE",
		}

		-- not our code, ignore POSIX usage warnings for now
		warnings "Off"

		kind "StaticLib"
end