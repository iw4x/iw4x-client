json11 = {
	settings = nil,
}

function json11.setup(settings)
	if not settings.source then error("Missing source.") end

	json11.settings = settings
end

function json11.import()
	if not json11.settings then error("Run json11.setup first") end

	links { "json11" }
	json11.includes()
end

function json11.includes()
	if not json11.settings then error("Run json11.setup first") end

	includedirs { json11.settings.source }
end

function json11.project()
	if not json11.settings then error("Run json11.setup first") end

	project "json11"
		language "C++"

		includedirs
		{
			json11.settings.source,
		}

		files
		{
			path.join(json11.settings.source, "*.cpp"),
			path.join(json11.settings.source, "*.hpp"),
		}
		removefiles
		{
			path.join(json11.settings.source, "test*"),
		}

		-- not our code, ignore POSIX usage warnings for now
		warnings "Off"

		defines { "_LIB" }
		removedefines { "_USRDLL", "_DLL" }
		kind "StaticLib"
end
