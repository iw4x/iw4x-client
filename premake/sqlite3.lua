sqlite3 = {
	settings = nil,
}

function sqlite3.setup(settings)
	if not settings.source then error("Missing source.") end

	sqlite3.settings = settings
end

function sqlite3.import()
	if not sqlite3.settings then error("Run sqlite3.setup first") end

	sqlite3.includes()
	sqlite3.links()
end

function sqlite3.links()
	links { "sqlite3" }
end

function sqlite3.includes()
	if not sqlite3.settings then error("Run sqlite3.setup first") end

	includedirs { sqlite3.settings.source }
end

function sqlite3.project()
	if not sqlite3.settings then error("Run sqlite3.setup first") end

	project "sqlite3"
		language "C++"

		includedirs
		{
			sqlite3.settings.source,
		}

		files
		{
			path.join(sqlite3.settings.source, "sqlite3*.c"),
			path.join(sqlite3.settings.source, "sqlite3*.h"),
		}

		-- not our code, ignore POSIX usage warnings for now
		warnings "Off"

		--kind "SharedLib"
		--filter "*Static"
			kind "StaticLib"
		--filter "kind:StaticLib"
		--	defines { "_LIB" }
		--	removedefines { "_USRDLL", "_DLL" }
end
