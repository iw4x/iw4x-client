base128 = {
	settings = nil
}

function base128.setup(settings)
	if not settings.source then error("Missing source.") end

	base128.settings = settings
end

function base128.import()
	if not base128.settings then error("Run base128.setup first") end

	base128.links()
	base128.includes()
end

function base128.links()
	if not base128.settings then error("Run base128.setup first") end

	links { "base128" }
end

function base128.includes()
	if not base128.settings then error("Run base128.setup first") end

	includedirs { path.join(base128.settings.source, "cpp") }
end

function base128.project()
	if not base128.settings then error("Run base128.setup first") end

	project "base128"
		language "C++"

		base128.includes()

		files
		{
			path.join(base128.settings.source, "cpp/*.cpp"),
			path.join(base128.settings.source, "cpp/*.h"),
		}
		removefiles
		{
			"**/demo.*",
		}

		-- not our code, ignore POSIX usage warnings for now
		warnings "Off"

		defines { "_LIB" }
		removedefines { "_USRDLL", "_DLL" }
		kind "StaticLib"
end
